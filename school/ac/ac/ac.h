/*
A C Implementation of the Fast Arithmetic Coder
Introduction

The arithmetic coding program given here is based on the fast coder discussed in previous
sections. It is optimised for readability as well as speed, and is intended to be used as a
general-purpose arithmetic coding library—experience has shown that it’s very hard to get
arithmetic coding algorithms to run correctly, so it is much better to reuse tried and tested
code!
The encoded data produced by this code contains a few dummy bytes in addition to
the useful data. This is to make the program more flexible in practical applications. In
particular, the encoder and decoder will both leave the file pointer in the same location
after compression / decompression has been completed. Therefore, one can simply append
further data to the file without having to re-position the file pointer after decoding. This
convenience was not present in other arithmetic coders.
Before using this code, the reader should be aware that arithmetic coding was covered
by a number of patents. Many of these have now expired, since the method is over 20 years
old; others may still be in force. You need to perform your own checks. None of the novel
ideas in this paper are covered by additional patents, so you can use them as you wish.

ac.h

*/

/* The programmers’ interface for this arithmetic coder is fairly self-explanatory. The denominator used by AC_Encode and AC_Decode is that passed to the previous call to those
functions. Initially, the denominator is set to 1, so you will almost certainly want to say
AC_Encode (0, 1, new_denom) before beginning the encoding of the file, and equivalently
value = AC_Decode (0, 1, new_denom) before decoding.
In order to ensure correct operation, both the compressor and decompressor must make
matching calls to AC_Encode and AC_Decode. If the compression software makes a call to
AC_Encode (low, high, new_denom) then the decompressor must make a matching call to
AC_Decode (low, high, new_denom). */
/*
* ac.h
* By Barney Stratford
* Version 12, 13th January 2005
*/
#ifndef AC_H
#define AC_H
#define MAX_DENOM 65536
void AC_InitialiseEncoder (FILE *compressed);
void AC_InitialiseDecoder (FILE *compressed);
void AC_Encode (register long low, register long high, long new_denom);
long AC_Decode (register long low, register long high, long new_denom);
void AC_FinaliseEncoder (void);
void AC_FinaliseDecoder (void);
#endif
