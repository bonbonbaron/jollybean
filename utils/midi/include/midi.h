typedef int (*handle_midi_event_func_t)(void *data, fluid_midi_event_t *event);
typedef int (*handle_midi_tick_func_t)(void *data, int tick);
fluid_midi_event_t *new_fluid_midi_event(void);
void delete_fluid_midi_event(fluid_midi_event_t *event);
int fluid_midi_event_set_type(fluid_midi_event_t *evt, int type);
int fluid_midi_event_get_type(const fluid_midi_event_t *evt);
int fluid_midi_event_set_channel(fluid_midi_event_t *evt, int chan);
int fluid_midi_event_get_channel(const fluid_midi_event_t *evt);
int fluid_midi_event_get_key(const fluid_midi_event_t *evt);
int fluid_midi_event_set_key(fluid_midi_event_t *evt, int key);
int fluid_midi_event_get_velocity(const fluid_midi_event_t *evt);
int fluid_midi_event_set_velocity(fluid_midi_event_t *evt, int vel);
int fluid_midi_event_get_control(const fluid_midi_event_t *evt);
int fluid_midi_event_set_control(fluid_midi_event_t *evt, int ctrl);
int fluid_midi_event_get_value(const fluid_midi_event_t *evt);
int fluid_midi_event_set_value(fluid_midi_event_t *evt, int val);
int fluid_midi_event_get_program(const fluid_midi_event_t *evt);
int fluid_midi_event_set_program(fluid_midi_event_t *evt, int val);
int fluid_midi_event_get_pitch(const fluid_midi_event_t *evt);
int fluid_midi_event_set_pitch(fluid_midi_event_t *evt, int val);
int fluid_midi_event_set_sysex(fluid_midi_event_t *evt, void *data,
        int size, int dynamic);
int fluid_midi_event_set_text(fluid_midi_event_t *evt,
        void *data, int size, int dynamic);
int fluid_midi_event_get_text(fluid_midi_event_t *evt,
        void **data, int *size);
int fluid_midi_event_set_lyrics(fluid_midi_event_t *evt,
        void *data, int size, int dynamic);
int fluid_midi_event_get_lyrics(fluid_midi_event_t *evt,
        void **data, int *size);
typedef enum
{
    FLUID_MIDI_ROUTER_RULE_NOTE,                  /**< MIDI note rule */
    FLUID_MIDI_ROUTER_RULE_CC,                    /**< MIDI controller rule */
    FLUID_MIDI_ROUTER_RULE_PROG_CHANGE,           /**< MIDI program change rule */
    FLUID_MIDI_ROUTER_RULE_PITCH_BEND,            /**< MIDI pitch bend rule */
    FLUID_MIDI_ROUTER_RULE_CHANNEL_PRESSURE,      /**< MIDI channel pressure rule */
    FLUID_MIDI_ROUTER_RULE_KEY_PRESSURE,          /**< MIDI key pressure rule */
    FLUID_MIDI_ROUTER_RULE_COUNT                  /**< @internal Total count of rule types. This symbol
                                                    is not part of the public API and ABI stability
                                                    guarantee and may change at any time!*/
} fluid_midi_router_rule_type;


/** @startlifecycle{MIDI Router} */
fluid_midi_router_t *new_fluid_midi_router(fluid_settings_t *settings,
        handle_midi_event_func_t handler,
        void *event_handler_data);
void delete_fluid_midi_router(fluid_midi_router_t *handler);
/** @endlifecycle */

int fluid_midi_router_set_default_rules(fluid_midi_router_t *router);
int fluid_midi_router_clear_rules(fluid_midi_router_t *router);
int fluid_midi_router_add_rule(fluid_midi_router_t *router,
        fluid_midi_router_rule_t *rule, int type);

fluid_midi_router_rule_t *new_fluid_midi_router_rule(void);
void delete_fluid_midi_router_rule(fluid_midi_router_rule_t *rule);
/** @endlifecycle */

void fluid_midi_router_rule_set_chan(fluid_midi_router_rule_t *rule,
        int min, int max, float mul, int add);
void fluid_midi_router_rule_set_param1(fluid_midi_router_rule_t *rule,
        int min, int max, float mul, int add);
void fluid_midi_router_rule_set_param2(fluid_midi_router_rule_t *rule,
        int min, int max, float mul, int add);
int fluid_midi_router_handle_midi_event(void *data, fluid_midi_event_t *event);
int fluid_midi_dump_prerouter(void *data, fluid_midi_event_t *event);
int fluid_midi_dump_postrouter(void *data, fluid_midi_event_t *event);
/** @startlifecycle{MIDI Driver} */
FLUIDSYNTH_API
fluid_midi_driver_t *new_fluid_midi_driver(fluid_settings_t *settings,
        handle_midi_event_func_t handler,
        void *event_handler_data);

void delete_fluid_midi_driver(fluid_midi_driver_t *driver);
/** @endlifecycle */
enum fluid_player_status
{
    FLUID_PLAYER_READY,           /**< Player is ready */
    FLUID_PLAYER_PLAYING,         /**< Player is currently playing */
    FLUID_PLAYER_STOPPING,        /**< Player is stopping, but hasn't finished yet (currently unused) */
    FLUID_PLAYER_DONE             /**< Player is finished playing */
};

enum fluid_player_set_tempo_type
{
    FLUID_PLAYER_TEMPO_INTERNAL,      /**< Use midi file tempo set in midi file (120 bpm by default). Multiplied by a factor */
    FLUID_PLAYER_TEMPO_EXTERNAL_BPM,  /**< Set player tempo in bpm, supersede midi file tempo */
    FLUID_PLAYER_TEMPO_EXTERNAL_MIDI, /**< Set player tempo in us per quarter note, supersede midi file tempo */
    FLUID_PLAYER_TEMPO_NBR        /**< @internal Value defines the count of player tempo type (#fluid_player_set_tempo_type) @warning This symbol is not part of the public API and ABI stability guarantee and may change at any time! */
};

/** @startlifecycle{MIDI File Player} */
fluid_player_t *new_fluid_player(fluid_synth_t *synth);
void delete_fluid_player(fluid_player_t *player);
/** @endlifecycle */

int fluid_player_add(fluid_player_t *player, const char *midifile);
int fluid_player_add_mem(fluid_player_t *player, const void *buffer, size_t len);
int fluid_player_play(fluid_player_t *player);
int fluid_player_stop(fluid_player_t *player);
int fluid_player_join(fluid_player_t *player);
int fluid_player_set_loop(fluid_player_t *player, int loop);
int fluid_player_set_tempo(fluid_player_t *player, int tempo_type, double tempo);
int fluid_player_set_midi_tempo(fluid_player_t *player, int tempo);
int fluid_player_set_bpm(fluid_player_t *player, int bpm);
int fluid_player_set_playback_callback(fluid_player_t *player, handle_midi_event_func_t handler, void *handler_data);
int fluid_player_set_tick_callback(fluid_player_t *player, handle_midi_tick_func_t handler, void *handler_data);

int fluid_player_get_status(fluid_player_t *player);
int fluid_player_get_current_tick(fluid_player_t *player);
int fluid_player_get_total_ticks(fluid_player_t *player);
int fluid_player_get_bpm(fluid_player_t *player);
int fluid_player_get_midi_tempo(fluid_player_t *player);
int fluid_player_seek(fluid_player_t *player, int ticks);
