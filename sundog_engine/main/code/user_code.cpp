/*
    user_code.cpp
    Minimal stub implementation of the app-level contract that main.cpp expects.
    This is NOT Pixilang or SunVox yet - it's just enough to produce a linkable,
    bootable Sundog engine binary so the display/audio/input plumbing in
    wm_fruitjam.h / sound_sndout_fruitjam.h can be brought up and tested on its
    own, before wiring in a real interpreter on top.
*/

#include "user_code.h"

//TODO: confirm against the actual Fruit Jam DVI resolution you bring up in
//wm_fruitjam.h's device_start() - this just needs to match whatever xsize/ysize
//you pass to win_init() there.
const UTF8_CHAR *user_window_name = (const UTF8_CHAR*)"Sundog Fruit Jam";
const UTF8_CHAR *user_profile_name = (const UTF8_CHAR*)"A:/sundog.ini";
const UTF8_CHAR *user_debug_log_file_name = 0; //no log file yet - NODEBUG is set in core.h anyway
int user_window_xsize = 720;
int user_window_ysize = 400;
int user_window_flags = 0;

void user_init( window_manager *wm )
{
    //TODO: real app init goes here eventually (Pixilang VM bring-up, etc).
    //For now, nothing - just proves the engine boots to the event loop.
}

int user_event_handler( sundog_event *evt, window_manager *wm )
{
    //Return 0 = event not consumed by app-level logic, let the engine's own
    //handling continue. Real apps would check evt->type here (EVT_KEYDOWN,
    //EVT_MOUSEBUTTONDOWN, etc) - see wmanager.h for the EVT_* constants.
    return 0;
}

void user_close( window_manager *wm )
{
    //TODO: app-level teardown goes here eventually.
}

//Audio render callback - see sound_player.cpp: returning 0 tells the engine
//to fill the output buffer with silence, which is the correct stub behavior
//until real audio rendering (Pixilang's mixer, or SunVox) is wired in.
int render_piece_of_sound( sound_struct *ss )
{
    return 0;
}
