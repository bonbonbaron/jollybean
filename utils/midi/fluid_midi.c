/* FluidSynth - A Software Synthesizer
 *
 * Copyright (C) 2003  Peter Hanappe and others.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA
 */

#include "include/fluid_midi.h"
#include "log.h"
#include <stdio.h>

// file-reading sets the player up, so they're both needed to extract our data.

static int fluid_midi_event_length (unsigned char event);
static int fluid_isasciistring (char *s);
static long fluid_getlength (const unsigned char *s);


/* Read the entire contents of a file into memory, allocating enough memory
 * for the file, and returning the length and the buffer.
 * Note: This rewinds the file to the start before reading.
 * Returns NULL if there was an error reading or allocating memory.
 */
typedef FILE *fluid_file;
static char *fluid_file_read_full (fluid_file fp, size_t *length);
static void fluid_midi_event_set_sysex_LOCAL (fluid_midi_event_t * evt,
																							int type, void *data, int size,
																							int dynamic);
static void fluid_midi_event_get_sysex_LOCAL (fluid_midi_event_t * evt,
																							void **data, int *size);
#define READ_FULL_INITIAL_BUFLEN 1024

static fluid_track_t *new_fluid_track (int num);
static void delete_fluid_track (fluid_track_t * track);
static int fluid_track_set_name (fluid_track_t * track, char *name);
static int fluid_track_add_event (fluid_track_t * track,
																	fluid_midi_event_t * evt);
static fluid_midi_event_t *fluid_track_next_event (fluid_track_t * track);
static int fluid_track_get_duration (fluid_track_t * track);
static int fluid_track_reset (fluid_track_t * track);

static int fluid_player_callback (void *data, unsigned int msec);
static int fluid_player_reset (fluid_player_t * player);
static int fluid_player_load (fluid_player_t * player,
															fluid_playlist_item * item);
static void fluid_player_playlist_load (fluid_player_t * player,
																				unsigned int msec);
static void fluid_player_update_tempo (fluid_player_t * player);

static fluid_midi_file *new_fluid_midi_file (const char *buffer,
																						 size_t length);
static void delete_fluid_midi_file (fluid_midi_file * mf);
static int fluid_midi_file_read_mthd (fluid_midi_file * midifile);
static int fluid_midi_file_load_tracks (fluid_midi_file * midifile, fluid_player_t *playerP);
static int fluid_midi_file_read_track (fluid_midi_file * mf, fluid_player_t *playerP, int num);
static int fluid_midi_file_read_event (fluid_midi_file * mf,
																			 fluid_track_t * track);
static int fluid_midi_file_read_varlen (fluid_midi_file * mf);
static int fluid_midi_file_getc (fluid_midi_file * mf);
static int fluid_midi_file_push (fluid_midi_file * mf, int c);
static int fluid_midi_file_read (fluid_midi_file * mf, void *buf, int len);
static int fluid_midi_file_skip (fluid_midi_file * mf, int len);
static int fluid_midi_file_eof (fluid_midi_file * mf);
static int fluid_midi_file_read_tracklen (fluid_midi_file * mf);
static int fluid_midi_file_eot (fluid_midi_file * mf);
static int fluid_midi_file_get_division (fluid_midi_file * midifile);


/***************************************************************
 *
 *                      MIDIFILE
 */

/**
 * Check if a file is a MIDI file.
 * @param filename Path to the file to check
 * @return TRUE if it could be a MIDI file, FALSE otherwise
 *
 * The current implementation only checks for the "MThd" header in the file.
 * It is useful only to distinguish between SoundFont and MIDI files.
 */
int fluid_is_midifile (const char *filename) {
	FILE *fp;
	uint32_t id;
	int retcode = FALSE;

	do {
		if ((fp = fopen(filename, "rb")) == NULL) {
			return retcode;
		}

		if (FLUID_FREAD (&id, sizeof (id), 1, fp) != 1) {
			break;
		}

		retcode = (id == FLUID_FOURCC ('M', 'T', 'h', 'd'));
	}
	while (0);

	FLUID_FCLOSE (fp);

	return retcode;
}

/**
 * Return a new MIDI file handle for parsing an already-loaded MIDI file.
 * @internal
 * @param buffer Pointer to full contents of MIDI file (borrows the pointer).
 *  The caller must not free buffer until after the fluid_midi_file is deleted.
 * @param length Size of the buffer in bytes.
 * @return New MIDI file handle or NULL on error.
 */
fluid_midi_file *new_fluid_midi_file (const char *buffer, size_t length) {
	fluid_midi_file *mf;

	mf = FLUID_NEW (fluid_midi_file);

	if (mf == NULL) {
		printf("Out of memory\n");
		return NULL;
	}

	FLUID_MEMSET (mf, 0, sizeof (fluid_midi_file));

	mf->c = -1;
	mf->running_status = -1;

	mf->buffer = buffer;
	mf->buf_len = length;
	mf->buf_pos = 0;
	mf->eof = FALSE;

	if (fluid_midi_file_read_mthd (mf) != FLUID_OK) {
		FLUID_FREE (mf);
		return NULL;
	}

	return mf;
}

static char *fluid_file_read_full (fluid_file fp, size_t *length) {
	size_t buflen;
	char *buffer;
	size_t n;

	/* Work out the length of the file in advance */
	if (FLUID_FSEEK (fp, 0, SEEK_END) != 0) {
		printf("File load: Could not seek within file\n");
		return NULL;
	}

	buflen = ftell (fp);

	if (FLUID_FSEEK (fp, 0, SEEK_SET) != 0) {
		printf("File load: Could not seek within file\n");
		return NULL;
	}

	printf("File load: Allocating %lu bytes\n", (unsigned long) buflen);
	buffer = FLUID_MALLOC (buflen);

	if (buffer == NULL) {
		printf("Out of memory\n");
		return NULL;
	}

	n = FLUID_FREAD (buffer, 1, buflen, fp);

	if (n != buflen) {
		printf("Only read %lu bytes; expected %lu\n",
							 (unsigned long) n, (unsigned long) buflen);
		FLUID_FREE (buffer);
		return NULL;
	};

	*length = n;

	return buffer;
}

/**
 * Delete a MIDI file handle.
 * @internal
 * @param mf MIDI file handle to close and free.
 */
void delete_fluid_midi_file (fluid_midi_file * mf) {
	fluid_return_if_fail (mf != NULL);

	FLUID_FREE (mf);
}

/*
 * Gets the next byte in a MIDI file, taking into account previous running status.
 *
 * returns -1 if EOF or read error
 */
int fluid_midi_file_getc (fluid_midi_file * mf) {
	unsigned char c;

	if (mf->c >= 0) {
		c = mf->c;
		mf->c = -1;
	} else {
		if (mf->buf_pos >= mf->buf_len) {
			mf->eof = TRUE;
			return -1;
		}

		c = mf->buffer[mf->buf_pos++];
		mf->trackpos++;
	}

	return (int) c;
}

/*
 * Saves a byte to be returned the next time fluid_midi_file_getc() is called,
 * when it is necessary according to running status.
 */
int fluid_midi_file_push (fluid_midi_file * mf, int c) {
	mf->c = c;
	return FLUID_OK;
}

/*
 * fluid_midi_file_read
 */
int fluid_midi_file_read (fluid_midi_file * mf, void *buf, int len) {
	int num = len < mf->buf_len - mf->buf_pos ? len : mf->buf_len - mf->buf_pos;

	if (num != len) {
		mf->eof = TRUE;
	}

	if (num < 0) {
		num = 0;
	}

	/* Note: Read bytes, even if there aren't enough, but only increment
	 * trackpos if successful (emulates old behaviour of fluid_midi_file_read)
	 */
	FLUID_MEMCPY (buf, mf->buffer + mf->buf_pos, num);
	mf->buf_pos += num;

	if (num == len) {
		mf->trackpos += num;
	}
#if DEBUG
	else {
		printf("Could not read the requested number of bytes\n");
	}

#endif
	return (num != len) ? FLUID_FAILED : FLUID_OK;
}

/*
 * fluid_midi_file_skip
 */
int fluid_midi_file_skip (fluid_midi_file * mf, int skip) {
	int new_pos = mf->buf_pos + skip;

	/* Mimic the behaviour of fseek: Error to seek past the start of file, but
	 * OK to seek past end (this just puts it into the EOF state). */
	if (new_pos < 0) {
		printf("Failed to seek position in file\n");
		return FLUID_FAILED;
	}

	/* Clear the EOF flag, even if moved past the end of the file (this is
	 * consistent with the behaviour of fseek). */
	mf->eof = FALSE;
	mf->buf_pos = new_pos;
	return FLUID_OK;
}

/*
 * fluid_midi_file_eof
 */
int fluid_midi_file_eof (fluid_midi_file * mf) {
	/* Note: This does not simply test whether the file read pointer is past
	 * the end of the file. It mimics the behaviour of feof by actually
	 * testing the stateful EOF condition, which is set to TRUE if getc or
	 * fread have attempted to read past the end (but not if they have
	 * precisely reached the end), but reset to FALSE upon a successful seek.
	 */
	return mf->eof;
}

/*
 * fluid_midi_file_read_mthd
 */
int fluid_midi_file_read_mthd (fluid_midi_file * mf) {
	char mthd[14];

	if (fluid_midi_file_read (mf, mthd, sizeof (mthd)) != FLUID_OK) {
		return FLUID_FAILED;
	}

	if ((FLUID_STRNCMP (mthd, "MThd", 4) != 0) || (mthd[7] != 6)
			|| (mthd[9] > 2)) {
		printf("Doesn't look like a MIDI file: invalid MThd header\n");
		return FLUID_FAILED;
	}

	mf->type = mthd[9];
	mf->ntracks = (unsigned) mthd[11];
	mf->ntracks += (unsigned int) (mthd[10]) << 16;

	if ((signed char) mthd[12] < 0) {
		mf->uses_smpte = 1;
		mf->smpte_fps = -(signed char) mthd[12];
		mf->smpte_res = (unsigned) mthd[13];
		printf("File uses SMPTE timing -- Not implemented yet\n");
		return FLUID_FAILED;
	} else {
		mf->uses_smpte = 0;
		mf->division = ((unsigned) mthd[12] << 8) | ((unsigned) mthd[13] & 0xff);
		printf("Division=%d\n", mf->division);
	}

	return FLUID_OK;
}

/*
 * fluid_midi_file_load_tracks
 */
int fluid_midi_file_load_tracks (fluid_midi_file * mf, fluid_player_t *playerP) {
	int i;

	for (i = 0; i < mf->ntracks; i++) {
		if (fluid_midi_file_read_track (mf, playerP, i) != FLUID_OK) {
			return FLUID_FAILED;
		}
	}

	return FLUID_OK;
}

/*
 * fluid_isasciistring
 */
int fluid_isasciistring (char *s) {
	/* From ctype.h */
#define fluid_isascii(c)    (((c) & ~0x7f) == 0)

	size_t i, len = FLUID_STRLEN (s);

	for (i = 0; i < len; i++) {
		if (!fluid_isascii (s[i])) {
			return 0;
		}
	}

	return 1;

#undef fluid_isascii
}

/*
 * fluid_getlength
 */
long fluid_getlength (const unsigned char *s) {
	long i = 0;
	i = s[3] | (s[2] << 8) | (s[1] << 16) | (s[0] << 24);
	return i;
}

/*
 * fluid_midi_file_read_tracklen
 */
int fluid_midi_file_read_tracklen (fluid_midi_file * mf) {
	unsigned char length[5];

	if (fluid_midi_file_read (mf, length, 4) != FLUID_OK) {
		return FLUID_FAILED;
	}

	mf->tracklen = fluid_getlength (length);
	mf->trackpos = 0;
	mf->eot = 0;
	return FLUID_OK;
}

/*
 * fluid_midi_file_eot   (EOT = "end of track)
 */
int fluid_midi_file_eot (fluid_midi_file * mf) {
#if DEBUG

	if (mf->trackpos > mf->tracklen) {
		printf ("track overrun: %d > %d\n", mf->trackpos, mf->tracklen);
	}
#endif
	return mf->eot || (mf->trackpos >= mf->tracklen);
}

/*
 * fluid_player_add_track
 */
int fluid_player_add_track(fluid_player_t *player, fluid_track_t *track) {
    if(player->ntracks < MAX_NUMBER_OF_TRACKS) {
        player->track[player->ntracks++] = track;
        return FLUID_OK;
    }
    else {
        return FLUID_FAILED;
    }
}

/*
 * fluid_midi_file_read_track
 */
int fluid_midi_file_read_track (fluid_midi_file * mf, fluid_player_t *playerP, int num) {
	fluid_track_t *track;
	unsigned char id[5], length[5];
	int found_track = 0;
	int skip;

	if (fluid_midi_file_read (mf, id, 4) != FLUID_OK) {
		return FLUID_FAILED;
	}

	id[4] = '\0';
	mf->dtime = 0;

	while (!found_track) {

		if (fluid_isasciistring ((char *) id) == 0) {
			printf("A non-ascii track header found, corrupt file\n");
			return FLUID_FAILED;

		}
		// if this is a track...
		else if (FLUID_STRCMP ((char *) id, "MTrk") == 0) {

			found_track = 1;

			// get track length
			if (fluid_midi_file_read_tracklen (mf) != FLUID_OK) {
				return FLUID_FAILED;
			}
			// make a new track struct
			track = new_fluid_track (num);

			if (track == NULL) {
				printf("Out of memory");
				return FLUID_FAILED;
			}
			// Till you reach the end of the track, read the next event from file into track object.
			while (!fluid_midi_file_eot (mf)) {
				if (fluid_midi_file_read_event (mf, track) != FLUID_OK) {
					delete_fluid_track (track);
					return FLUID_FAILED;
				}
			}

			/* Skip remaining track data, if any */
			if (mf->trackpos < mf->tracklen) {
				if (fluid_midi_file_skip (mf, mf->tracklen - mf->trackpos) !=
						FLUID_OK) {
					delete_fluid_track (track);
					return FLUID_FAILED;
				}
			}
      if(fluid_player_add_track(playerP, track) != FLUID_OK) {
          delete_fluid_track(track);
          return FLUID_FAILED;
      }
		} else {
			found_track = 0;

			if (fluid_midi_file_read (mf, length, 4) != FLUID_OK) {
				return FLUID_FAILED;
			}

			skip = fluid_getlength (length);

			/* fseek(mf->fp, skip, SEEK_CUR); */
			if (fluid_midi_file_skip (mf, skip) != FLUID_OK) {
				return FLUID_FAILED;
			}
		}
	}

	if (fluid_midi_file_eof (mf)) {
		printf("Unexpected end of file");
		return FLUID_FAILED;
	}

	return FLUID_OK;
}

/*
 * fluid_midi_file_read_varlen
 */
int fluid_midi_file_read_varlen (fluid_midi_file * mf) {
	int i;
	int c;
	mf->varlen = 0;

	for (i = 0;; i++) {
		if (i == 4) {
			printf("Invalid variable length number");
			return FLUID_FAILED;
		}

		c = fluid_midi_file_getc (mf);

		if (c < 0) {
			printf("Unexpected end of file");
			return FLUID_FAILED;
		}

		if (c & 0x80) {
			mf->varlen |= (int) (c & 0x7F);
			mf->varlen <<= 7;
		} else {
			mf->varlen += c;
			break;
		}
	}

	return FLUID_OK;
}

/*
 * fluid_midi_file_read_event
 */

#define BEAN_DBG 0
#if BEAN_DBG
#define printIfDbg(x, y) printf(x, y);
#else
#define printIfDbg(x, y)
#endif
int fluid_midi_file_read_event (fluid_midi_file * mf, fluid_track_t * track) {
	int status;
	int type;
	int tempo;
	unsigned char *metadata = NULL;
	unsigned char *dyn_buf = NULL;
	unsigned char static_buf[256];
	int nominator, denominator, clocks, notes;
	fluid_midi_event_t *evt;
	int channel = 0;
	int param1 = 0;
	int param2 = 0;
	int size;

	/* read the delta-time of the event */
	if (fluid_midi_file_read_varlen (mf) != FLUID_OK) {
		return FLUID_FAILED;
	}

	mf->dtime += mf->varlen;

	/* read the status byte */
	status = fluid_midi_file_getc (mf);

	if (status < 0) {
		printf("Unexpected end of file");
		return FLUID_FAILED;
	}

	/* not a valid status byte: use the running status instead */
	if ((status & 0x80) == 0) {
		if ((mf->running_status & 0x80) == 0) {
			printf("Undefined status and invalid running status");
			return FLUID_FAILED;
		}

		fluid_midi_file_push (mf, status);
		status = mf->running_status;
	}

	/* check what message we have */

	mf->running_status = status;

	if (status == MIDI_SYSEX) {		/* system exclusif */
		/* read the length of the message */
		if (fluid_midi_file_read_varlen (mf) != FLUID_OK) {
			return FLUID_FAILED;
		}

		if (mf->varlen) {
			printf("%s: %d: alloc metadata, len = %d\n", __FILE__,
								 __LINE__, mf->varlen);
			metadata = FLUID_MALLOC (mf->varlen + 1);

			if (metadata == NULL) {
				printf("Out of memory");
				return FLUID_FAILED;
			}

			/* read the data of the message */
			if (fluid_midi_file_read (mf, metadata, mf->varlen) != FLUID_OK) {
				FLUID_FREE (metadata);
				return FLUID_FAILED;
			}

			evt = new_fluid_midi_event ();

			if (evt == NULL) {
				printf("Out of memory");
				FLUID_FREE (metadata);
				return FLUID_FAILED;
			}

			evt->dtime = mf->dtime;
			size = mf->varlen;

			if (metadata[mf->varlen - 1] == MIDI_EOX) {
				size--;
			}

			/* Add SYSEX event and indicate that its dynamically allocated and should be freed with event */
			fluid_midi_event_set_sysex (evt, metadata, size, TRUE);
			fluid_track_add_event (track, evt);
			mf->dtime = 0;
		}

		return FLUID_OK;

	} else if (status == MIDI_META_EVENT) {	/* meta events */
		int result = FLUID_OK;

		/* get the type of the meta message */
		type = fluid_midi_file_getc (mf);

		if (type < 0) {
			printf("Unexpected end of file\n");
			return FLUID_FAILED;
		}

		/* get the length of the data part */
		if (fluid_midi_file_read_varlen (mf) != FLUID_OK) {
			return FLUID_FAILED;
		}

		if (mf->varlen < 255) {
			metadata = &static_buf[0];
		} else {
			printf("%s: %d: alloc metadata, len = %d\n", __FILE__, __LINE__, mf->varlen);
			dyn_buf = FLUID_MALLOC (mf->varlen + 1);

			if (dyn_buf == NULL) {
				printf("Out of memory\n");
				return FLUID_FAILED;
			}

			metadata = dyn_buf;
		}

		/* read the data */
		if (mf->varlen) {
			if (fluid_midi_file_read (mf, metadata, mf->varlen) != FLUID_OK) {
				if (dyn_buf) {
					FLUID_FREE (dyn_buf);
				}

				return FLUID_FAILED;
			}
		}

		/* handle meta data */
		switch (type) {

		case MIDI_COPYRIGHT:
			metadata[mf->varlen] = 0;
			break;

		case MIDI_TRACK_NAME:
			metadata[mf->varlen] = 0;
			fluid_track_set_name (track, (char *) metadata);
			break;

		case MIDI_INST_NAME:
			metadata[mf->varlen] = 0;
			break;

		case MIDI_LYRIC:
		case MIDI_TEXT:{
				void *tmp;
				int size = mf->varlen + 1;

				/* NULL terminate strings for safety */
				metadata[size - 1] = '\0';

				evt = new_fluid_midi_event ();

				if (evt == NULL) {
					printf("Out of memory\n");
					result = FLUID_FAILED;
					break;
				}

				evt->dtime = mf->dtime;

				tmp = FLUID_MALLOC (size);

				if (tmp == NULL) {
					printf("Out of memory\n");
					delete_fluid_midi_event (evt);
					evt = NULL;
					result = FLUID_FAILED;
					break;
				}

				FLUID_MEMCPY (tmp, metadata, size);

				fluid_midi_event_set_sysex_LOCAL (evt, type, tmp, size, TRUE);
				fluid_track_add_event (track, evt);
				mf->dtime = 0;
			}
			break;

		case MIDI_MARKER:
			break;

		case MIDI_CUE_POINT:
			break;										/* don't care much for text events */

		case MIDI_EOT:
			if (mf->varlen != 0) {
				printf("Invalid length for EndOfTrack event\n");
				result = FLUID_FAILED;
				break;
			}

			mf->eot = 1;
			evt = new_fluid_midi_event ();

			if (evt == NULL) {
				printf("Out of memory\n");
				result = FLUID_FAILED;
				break;
			}

			evt->dtime = mf->dtime;
			evt->type = MIDI_EOT;
			fluid_track_add_event (track, evt);
			mf->dtime = 0;
			break;

		case MIDI_SET_TEMPO:
			if (mf->varlen != 3) {
				printf("Invalid length for SetTempo meta event\n");
				result = FLUID_FAILED;
				break;
			}

			tempo = (metadata[0] << 16) + (metadata[1] << 8) + metadata[2];
			evt = new_fluid_midi_event ();

			if (evt == NULL) {
				printf("Out of memory");
				result = FLUID_FAILED;
				break;
			}

			evt->dtime = mf->dtime;
			evt->type = MIDI_SET_TEMPO;
			evt->channel = 0;
			evt->param1 = tempo;
			evt->param2 = 0;
			fluid_track_add_event (track, evt);
			mf->dtime = 0;
			break;

		case MIDI_SMPTE_OFFSET:
			if (mf->varlen != 5) {
				printf("Invalid length for SMPTE Offset meta event\n");
				result = FLUID_FAILED;
				break;
			}

			break;										/* we don't use smtp */

		case MIDI_TIME_SIGNATURE:
			if (mf->varlen != 4) {
				printf("Invalid length for TimeSignature meta event\n");
				result = FLUID_FAILED;
				break;
			}

			nominator = metadata[0];
			denominator = pow (2.0, (double) metadata[1]);
			clocks = metadata[2];
			notes = metadata[3];

      printf(
								 "signature=%d/%d, metronome=%d, 32nd-notes=%d\n",
								 nominator, denominator, clocks, notes);

			break;

		case MIDI_KEY_SIGNATURE:
			if (mf->varlen != 2) {
				printf("Invalid length for KeySignature meta event\n");
				result = FLUID_FAILED;
				break;
			}

			/* We don't care about key signatures anyway */
			/* sf = metadata[0];
			   mi = metadata[1]; */
			break;

		case MIDI_SEQUENCER_EVENT:
			break;

		default:
			break;
		}

		if (dyn_buf) {
			printf("%s: %d: free metadata\n", __FILE__, __LINE__);
			FLUID_FREE (dyn_buf);
		}

		return result;

	} else {											/* channel messages */
		type = status & 0xf0;
		channel = status & 0x0f;

		/* all channel message have at least 1 byte of associated data */
		if ((param1 = fluid_midi_file_getc (mf)) < 0) {  // it is NOT a meta-event at this point. 
			printf("Unexpected end of file\n");
			return FLUID_FAILED;
		}

		switch (type) {

		case NOTE_ON:
		case NOTE_OFF:
		case KEY_PRESSURE:
		case CONTROL_CHANGE:
			if ((param2 = fluid_midi_file_getc (mf)) < 0) {
				printf("Unexpected end of file\n");
				return FLUID_FAILED;
			}
			break;

		case PROGRAM_CHANGE:
		case CHANNEL_PRESSURE:
			break;

		case PITCH_BEND:
			if ((param2 = fluid_midi_file_getc (mf)) < 0) {
				printf("Unexpected end of file\n");
				return FLUID_FAILED;
			}

			param1 = ((param2 & 0x7f) << 7) | (param1 & 0x7f);
			param2 = 0;
			break;

		default:
			/* Can't possibly happen !? */
			printf("Unrecognized MIDI event\n");
			return FLUID_FAILED;
		}

		evt = new_fluid_midi_event ();

		if (evt == NULL) {
			printf("Out of memory\n");
			return FLUID_FAILED;
		}

		evt->dtime = mf->dtime;
		evt->type = type;
		evt->channel = channel;
		evt->param1 = param1;
		evt->param2 = param2;
		fluid_track_add_event (track, evt);
		mf->dtime = 0;
	}

	return FLUID_OK;
}

/*
 * fluid_midi_file_get_division
 */
int fluid_midi_file_get_division (fluid_midi_file * midifile) {
	return midifile->division;
}

/******************************************************
 *
 *     fluid_track_t
 */

/**
 * Create a MIDI event structure.
 * @return New MIDI event structure or NULL when out of memory.
 */
fluid_midi_event_t *new_fluid_midi_event () {
	fluid_midi_event_t *evt;
	evt = FLUID_NEW (fluid_midi_event_t);

	if (evt == NULL) {
		printf("Out of memory\n");
		return NULL;
	}

	evt->dtime = 0;
	evt->type = 0;
	evt->channel = 0;
	evt->param1 = 0;
	evt->param2 = 0;
	evt->next = NULL;
	evt->paramptr = NULL;
	return evt;
}

/**
 * Delete MIDI event structure.
 * @param evt MIDI event structure
 */
void delete_fluid_midi_event (fluid_midi_event_t * evt) {
	fluid_midi_event_t *temp;
	fluid_return_if_fail (evt != NULL);

	while (evt) {
		temp = evt->next;

		/* Dynamic SYSEX event? - free (param2 indicates if dynamic) */
		if ((evt->type == MIDI_SYSEX || (evt->type == MIDI_TEXT)
				 || (evt->type == MIDI_LYRIC)) && evt->paramptr && evt->param2) {
			FLUID_FREE (evt->paramptr);
		}

		FLUID_FREE (evt);
		evt = temp;
	}
}

/**
 * Get the event type field of a MIDI event structure.
 * @param evt MIDI event structure
 * @return Event type field (MIDI status byte without channel)
 */
int fluid_midi_event_get_type (const fluid_midi_event_t * evt) {
	return evt->type;
}

/**
 * Set the event type field of a MIDI event structure.
 * @param evt MIDI event structure
 * @param type Event type field (MIDI status byte without channel)
 * @return Always returns #FLUID_OK
 */
int fluid_midi_event_set_type (fluid_midi_event_t * evt, int type) {
	evt->type = type;
	return FLUID_OK;
}

/**
 * Get the channel field of a MIDI event structure.
 * @param evt MIDI event structure
 * @return Channel field
 */
int fluid_midi_event_get_channel (const fluid_midi_event_t * evt) {
	return evt->channel;
}

/**
 * Set the channel field of a MIDI event structure.
 * @param evt MIDI event structure
 * @param chan MIDI channel field
 * @return Always returns #FLUID_OK
 */
int fluid_midi_event_set_channel (fluid_midi_event_t * evt, int chan) {
	evt->channel = chan;
	return FLUID_OK;
}

/**
 * Get the key field of a MIDI event structure.
 * @param evt MIDI event structure
 * @return MIDI note number (0-127)
 */
int fluid_midi_event_get_key (const fluid_midi_event_t * evt) {
	return evt->param1;
}

/**
 * Set the key field of a MIDI event structure.
 * @param evt MIDI event structure
 * @param v MIDI note number (0-127)
 * @return Always returns #FLUID_OK
 */
int fluid_midi_event_set_key (fluid_midi_event_t * evt, int v) {
	evt->param1 = v;
	return FLUID_OK;
}

/**
 * Get the velocity field of a MIDI event structure.
 * @param evt MIDI event structure
 * @return MIDI velocity number (0-127)
 */
int fluid_midi_event_get_velocity (const fluid_midi_event_t * evt) {
	return evt->param2;
}

/**
 * Set the velocity field of a MIDI event structure.
 * @param evt MIDI event structure
 * @param v MIDI velocity value
 * @return Always returns #FLUID_OK
 */
int fluid_midi_event_set_velocity (fluid_midi_event_t * evt, int v) {
	evt->param2 = v;
	return FLUID_OK;
}

/**
 * Get the control number of a MIDI event structure.
 * @param evt MIDI event structure
 * @return MIDI control number
 */
int fluid_midi_event_get_control (const fluid_midi_event_t * evt) {
	return evt->param1;
}

/**
 * Set the control field of a MIDI event structure.
 * @param evt MIDI event structure
 * @param v MIDI control number
 * @return Always returns #FLUID_OK
 */
int fluid_midi_event_set_control (fluid_midi_event_t * evt, int v) {
	evt->param1 = v;
	return FLUID_OK;
}

/**
 * Get the value field from a MIDI event structure.
 * @param evt MIDI event structure
 * @return Value field
 */
int fluid_midi_event_get_value (const fluid_midi_event_t * evt) {
	return evt->param2;
}

/**
 * Set the value field of a MIDI event structure.
 * @param evt MIDI event structure
 * @param v Value to assign
 * @return Always returns #FLUID_OK
 */
int fluid_midi_event_set_value (fluid_midi_event_t * evt, int v) {
	evt->param2 = v;
	return FLUID_OK;
}

/**
 * Get the program field of a MIDI event structure.
 * @param evt MIDI event structure
 * @return MIDI program number (0-127)
 */
int fluid_midi_event_get_program (const fluid_midi_event_t * evt) {
	return evt->param1;
}

/**
 * Set the program field of a MIDI event structure.
 * @param evt MIDI event structure
 * @param val MIDI program number (0-127)
 * @return Always returns #FLUID_OK
 */
int fluid_midi_event_set_program (fluid_midi_event_t * evt, int val) {
	evt->param1 = val;
	return FLUID_OK;
}

/**
 * Get the pitch field of a MIDI event structure.
 * @param evt MIDI event structure
 * @return Pitch value (14 bit value, 0-16383, 8192 is center)
 */
int fluid_midi_event_get_pitch (const fluid_midi_event_t * evt) {
	return evt->param1;
}

/**
 * Set the pitch field of a MIDI event structure.
 * @param evt MIDI event structure
 * @param val Pitch value (14 bit value, 0-16383, 8192 is center)
 * @return Always returns FLUID_OK
 */
int fluid_midi_event_set_pitch (fluid_midi_event_t * evt, int val) {
	evt->param1 = val;
	return FLUID_OK;
}

/**
 * Assign sysex data to a MIDI event structure.
 * @param evt MIDI event structure
 * @param data Pointer to SYSEX data
 * @param size Size of SYSEX data in bytes
 * @param dynamic TRUE if the SYSEX data has been dynamically allocated and
 *   should be freed when the event is freed (only applies if event gets destroyed
 *   with delete_fluid_midi_event())
 * @return Always returns #FLUID_OK
 */
int
fluid_midi_event_set_sysex (fluid_midi_event_t * evt, void *data, int size,
														int dynamic) {
	fluid_midi_event_set_sysex_LOCAL (evt, MIDI_SYSEX, data, size, dynamic);
	return FLUID_OK;
}

/**
 * Assign text data to a MIDI event structure.
 * @param evt MIDI event structure
 * @param data Pointer to text data
 * @param size Size of text data in bytes
 * @param dynamic TRUE if the data has been dynamically allocated and
 *   should be freed when the event is freed via delete_fluid_midi_event()
 * @return Always returns #FLUID_OK
 *
 * @since 2.0.0
 */
int
fluid_midi_event_set_text (fluid_midi_event_t * evt, void *data, int size,
													 int dynamic) {
	fluid_midi_event_set_sysex_LOCAL (evt, MIDI_TEXT, data, size, dynamic);
	return FLUID_OK;
}

/**
 * Get the text of a MIDI event structure.
 * @param evt MIDI event structure
 * @param data Pointer to return text data on.
 * @param size Pointer to return text size on.
 * @return Returns #FLUID_OK if \p data and \p size previously set by
 * fluid_midi_event_set_text() have been successfully retrieved.
 * Else #FLUID_FAILED is returned and \p data and \p size are not changed.
 * @since 2.0.3
 */
int fluid_midi_event_get_text (fluid_midi_event_t * evt, void **data,
															 int *size) {
	fluid_return_val_if_fail (evt != NULL, FLUID_FAILED);
	fluid_return_val_if_fail (evt->type == MIDI_TEXT, FLUID_FAILED);

	fluid_midi_event_get_sysex_LOCAL (evt, data, size);
	return FLUID_OK;
}

/**
 * Assign lyric data to a MIDI event structure.
 * @param evt MIDI event structure
 * @param data Pointer to lyric data
 * @param size Size of lyric data in bytes
 * @param dynamic TRUE if the data has been dynamically allocated and
 *   should be freed when the event is freed via delete_fluid_midi_event()
 * @return Always returns #FLUID_OK
 *
 * @since 2.0.0
 */
int
fluid_midi_event_set_lyrics (fluid_midi_event_t * evt, void *data, int size,
														 int dynamic) {
	fluid_midi_event_set_sysex_LOCAL (evt, MIDI_LYRIC, data, size, dynamic);
	return FLUID_OK;
}

/**
 * Get the lyric of a MIDI event structure.
 * @param evt MIDI event structure
 * @param data Pointer to return lyric data on.
 * @param size Pointer to return lyric size on.
 * @return Returns #FLUID_OK if \p data and \p size previously set by
 * fluid_midi_event_set_lyrics() have been successfully retrieved.
 * Else #FLUID_FAILED is returned and \p data and \p size are not changed.
 * @since 2.0.3
 */
int fluid_midi_event_get_lyrics (fluid_midi_event_t * evt, void **data,
																 int *size) {
	fluid_return_val_if_fail (evt != NULL, FLUID_FAILED);
	fluid_return_val_if_fail (evt->type == MIDI_LYRIC, FLUID_FAILED);

	fluid_midi_event_get_sysex_LOCAL (evt, data, size);
	return FLUID_OK;
}

static void fluid_midi_event_set_sysex_LOCAL (fluid_midi_event_t * evt,
																							int type, void *data, int size,
																							int dynamic) {
	evt->type = type;
	evt->paramptr = data;
	evt->param1 = size;
	evt->param2 = dynamic;
}

static void fluid_midi_event_get_sysex_LOCAL (fluid_midi_event_t * evt,
																							void **data, int *size) {
	if (data) {
		*data = evt->paramptr;
	}

	if (size) {
		*size = evt->param1;
	}
}

/******************************************************
 *
 *     fluid_track_t
 */

/*
 * new_fluid_track
 */
fluid_track_t *new_fluid_track (int num) {
	fluid_track_t *track;
	track = FLUID_NEW (fluid_track_t);

	if (track == NULL) {
		return NULL;
	}

	track->name = NULL;
	track->num = num;
	track->first = NULL;
	track->cur = NULL;
	track->last = NULL;
	track->ticks = 0;
	return track;
}

/*
 * delete_fluid_track
 */
void delete_fluid_track (fluid_track_t * track) {
	fluid_return_if_fail (track != NULL);

	FLUID_FREE (track->name);
	delete_fluid_midi_event (track->first);
	FLUID_FREE (track);
}

/*
 * fluid_track_set_name
 */
int fluid_track_set_name (fluid_track_t * track, char *name) {
	size_t len;

	if (track->name != NULL) {
		FLUID_FREE (track->name);
	}

	if (name == NULL) {
		track->name = NULL;
		return FLUID_OK;
	}

	len = FLUID_STRLEN (name);
	track->name = FLUID_MALLOC (len + 1);

	if (track->name == NULL) {
		printf("Out of memory\n");
		return FLUID_FAILED;
	}

	FLUID_STRCPY (track->name, name);
	return FLUID_OK;
}

/*
 * fluid_track_get_duration
 */
int fluid_track_get_duration (fluid_track_t * track) {
	int time = 0;
	fluid_midi_event_t *evt = track->first;

	while (evt != NULL) {
		time += evt->dtime;
		evt = evt->next;
	}

	return time;
}

/*
 * fluid_track_add_event
 */
// MB: WHY ARE YOU GUYS USING LINKED LISTS HERE!??
int fluid_track_add_event (fluid_track_t * track, fluid_midi_event_t * evt) {
	evt->next = NULL;

	if (track->first == NULL) {
		track->first = evt;
		track->cur = evt;
		track->last = evt;
	} else {
		track->last->next = evt;
		track->last = evt;
	}

	return FLUID_OK;
}

/*
 * fluid_track_next_event
 */
fluid_midi_event_t *fluid_track_next_event (fluid_track_t * track) {
	if (track->cur != NULL) {
		track->cur = track->cur->next;
	}

	return track->cur;
}

/*
 * fluid_track_reset
 */
int fluid_track_reset (fluid_track_t * track) {
	track->ticks = 0;
	track->cur = track->first;
	return FLUID_OK;
}


/**
 * Create a new MIDI player.
 * the needed midi information using the getter functions of fluid_midi_event_t.
 * This parser however is incomplete as it e.g. only provides a limited buffer to
 * store and process SYSEX data (i.e. doesn't allow arbitrary lengths)
 */
fluid_midi_event_t *fluid_midi_parser_parse (fluid_midi_parser_t * parser, unsigned char c) {
	fluid_midi_event_t *event;

	/* Real-time messages (0xF8-0xFF) can occur anywhere, even in the middle
	 * of another message. */
	if (c >= 0xF8) {
		if (c == MIDI_SYSTEM_RESET) {
			parser->event.type = c;
			parser->status = 0;				/* clear the status */
			return &parser->event;
		}

		return NULL;
	}

	/* Status byte? - If previous message not yet complete, it is discarded (re-sync). */
	if (c & 0x80) {
		/* Any status byte terminates SYSEX messages (not just 0xF7) */
		if (parser->status == MIDI_SYSEX && parser->nr_bytes > 0) {
			event = &parser->event;
			fluid_midi_event_set_sysex (event, parser->data, parser->nr_bytes,
																	FALSE);
		} else {
			event = NULL;
		}

		if (c < 0xF0) {							/* Voice category message? */
			parser->channel = c & 0x0F;
			parser->status = c & 0xF0;

			/* The event consumes x bytes of data... (subtract 1 for the status byte) */
			parser->nr_bytes_total = fluid_midi_event_length (parser->status)
				- 1;

			parser->nr_bytes = 0;			/* 0  bytes read so far */
		} else if (c == MIDI_SYSEX) {
			parser->status = MIDI_SYSEX;
			parser->nr_bytes = 0;
		} else {
			parser->status = 0;				/* Discard other system messages (0xF1-0xF7) */
		}

		return event;								/* Return SYSEX event or NULL */
	}

	/* Data/parameter byte */

	/* Discard data bytes for events we don't care about */
	if (parser->status == 0) {
		return NULL;
	}

	/* Max data size exceeded? (SYSEX messages only really) */
	if (parser->nr_bytes == FLUID_MIDI_PARSER_MAX_DATA_SIZE) {
		parser->status = 0;					/* Discard the rest of the message */
		return NULL;
	}

	/* Store next byte */
	parser->data[parser->nr_bytes++] = c;

	/* Do we still need more data to get this event complete? */
	if (parser->status == MIDI_SYSEX
			|| parser->nr_bytes < parser->nr_bytes_total) {
		return NULL;
	}

	/* Event is complete, return it.
	 * Running status byte MIDI feature is also handled here. */
	parser->event.type = parser->status;
	parser->event.channel = parser->channel;
	parser->nr_bytes = 0;					/* Reset data size, in case there are additional running status messages */

	switch (parser->status) {
	case NOTE_OFF:
	case NOTE_ON:
	case KEY_PRESSURE:
	case CONTROL_CHANGE:
	case PROGRAM_CHANGE:
	case CHANNEL_PRESSURE:
		parser->event.param1 = parser->data[0];	/* For example key number */
		parser->event.param2 = parser->data[1];	/* For example velocity */
		break;

	case PITCH_BEND:
		/* Pitch-bend is transmitted with 14-bit precision. */
		parser->event.param1 = (parser->data[1] << 7) | parser->data[0];
		break;

	default:											/* Unlikely */
		return NULL;
	}

	return &parser->event;
}

/* Purpose:
 * Returns the length of a MIDI message. */
static int fluid_midi_event_length (unsigned char event) {
	switch (event & 0xF0) {
	case NOTE_OFF:
	case NOTE_ON:
	case KEY_PRESSURE:
	case CONTROL_CHANGE:
	case PITCH_BEND:
		return 3;

	case PROGRAM_CHANGE:
	case CHANNEL_PRESSURE:
		return 2;
	}

	switch (event) {
	case MIDI_TIME_CODE:
	case MIDI_SONG_SELECT:
	case 0xF4:
	case 0xF5:
		return 2;

	case MIDI_TUNE_REQUEST:
		return 1;

	case MIDI_SONG_POSITION:
		return 3;
	}

	return 1;
}

/*
*** static int fluid_midi_file_read_mthd (fluid_midi_file * midifile);
static int fluid_midi_file_load_tracks (fluid_midi_file * midifile);
static int fluid_midi_file_read_track (fluid_midi_file * mf,
																			 fluid_player_t * player, int num);
static int fluid_midi_file_read_event (fluid_midi_file * mf,
																			 fluid_track_t * track);
static int fluid_midi_file_read_varlen (fluid_midi_file * mf);
static int fluid_midi_file_getc (fluid_midi_file * mf);
static int fluid_midi_file_push (fluid_midi_file * mf, int c);
static int fluid_midi_file_read (fluid_midi_file * mf, void *buf, int len);
static int fluid_midi_file_skip (fluid_midi_file * mf, int len);
static int fluid_midi_file_eof (fluid_midi_file * mf);
static int fluid_midi_file_read_tracklen (fluid_midi_file * mf);
static int fluid_midi_file_eot (fluid_midi_file * mf);
static int fluid_midi_file_get_division (fluid_midi_file * midifile);
*/

// MB: I'm commenting out everything that has to do with synth. We don't need it when
//     all we're doing is parsing MIDI files offline so fluidbean can fly even faster.
fluid_player_t * new_fluid_player() {
    int i;
    fluid_player_t *player;
    player = FLUID_NEW(fluid_player_t);

    if(player == NULL) {
        printf("Out of memory\n");
        return NULL;
    }

    //fluid_atomic_int_set(&player->status, FLUID_PLAYER_READY);
    //fluid_atomic_int_set(&player->stopping, 0);
    player->loop = 1;
    player->ntracks = 0;

    for(i = 0; i < MAX_NUMBER_OF_TRACKS; i++)
        player->track[i] = NULL;

    //player->synth = synth;
    player->system_timer = NULL;
    player->sample_timer = NULL;
    player->playlist = NULL;
    player->currentfile = NULL;
    player->division = 0;

    /* internal tempo (from MIDI file) in micro seconds per quarter note */
    player->sync_mode = 1; /* the player follows internal tempo change */
    player->miditempo = 500000;
    /* external tempo in micro seconds per quarter note */
    player->exttempo = 500000;
    /* tempo multiplier */
    player->multempo = 1.0F;

    player->deltatime = 4.0;
    player->cur_msec = 0;
    player->cur_ticks = 0;
    player->last_callback_ticks = -1;
    //fluid_atomic_int_set(&player->seek_ticks, -1);
    //fluid_player_set_playback_callback(player, fluid_synth_handle_midi_event, synth);
    //fluid_player_set_tick_callback(player, NULL, NULL);
    // The below tells the timer to use the system clock. I can default it to that in my own stuff.
    //player->use_system_timer = fluid_settings_str_equal(synth->settings,
                               //"player.timing-source", "system");
    if(player->use_system_timer) {
        //player->system_timer = new_fluid_timer((int) player->deltatime,
                                               //fluid_player_callback, player, TRUE, FALSE, TRUE);

        //if(player->system_timer == NULL)
            //goto err;
    }
    else {
        //player->sample_timer = new_fluid_sample_timer(player->synth,
                               //fluid_player_callback, player);

        //if(player->sample_timer == NULL) 
            //goto err;
    }

    //fluid_settings_getint(synth->settings, "player.reset-synth", &i);
    //fluid_player_handle_reset_synth(player, NULL, i);

    //fluid_settings_callback_int(synth->settings, "player.reset-synth",
                                //fluid_player_handle_reset_synth, player);

    return player;

//err:
    //delete_fluid_player(player);
    //return NULL;
}

int main(int argc, char **argv) {
  int e = 0;
  for (int i = 1; i < argc; ++i) // First arg is executable name.
    if(fluid_is_midifile(argv[i])) {
      printf("%s is a midi file!\n", argv[i]);
      // Read file into byte array and get the byte count.
      size_t nBytes = 0;
      FILE *fP = fopen(argv[i], "rb");
      char *fileContentsP = fluid_file_read_full(fP, &nBytes);
      fluid_midi_file *fluidMidiFileStructP = NULL;
      if (fileContentsP) 
        fluidMidiFileStructP = new_fluid_midi_file(fileContentsP, nBytes);
      // We need a player in order to populate the tracks.
      fluid_player_t *playerP = NULL;
      if (fluidMidiFileStructP) {
        playerP = new_fluid_player();
      }
      if (playerP) {
        e = fluid_midi_file_load_tracks(fluidMidiFileStructP, playerP);
      }
      if (!e) {
        for (int i = 0; i < playerP->ntracks; ++i) {
          printf("starting track %d...\n", i);
          fluid_track_t *trackP = playerP->track[i];
          for (fluid_midi_event_t *eventP = trackP->first;
               eventP != NULL;
               eventP = eventP->next) {
            printf("channel %d, event %d: key %d at velocity %d for %d ticks\n", eventP->channel, eventP->type, eventP->param1, eventP->param2, eventP->dtime);
          }
        }
        printf("Finished loading %s successfully.\n", argv[i]);  
      }
      else 
        return 1;
    } else {
      printf("%s is NOT a midi file. Exiting...\n", argv[i]);
      return 1;
    }
  
  return 0; 
}
