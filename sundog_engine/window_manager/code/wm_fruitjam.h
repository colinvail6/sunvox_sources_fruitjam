/*
    wm_fruitjam.h
    SunDog window-manager backend for the Adafruit Fruit Jam (RP2350B).

    device_start/end/event_handler/screen_lock/unlock are implemented here.
    Line/box/bitmap drawing and device_redraw_framebuffer are inherited from
    wm_framebuffer.h (see #include at the bottom) - they already operate on
    a plain pitch-addressed `framebuffer` array, so they need no changes.

    Display (dvi_fruitjam.c/.h) is now wired up. TinyUSB HID input is still
    a TODO.
*/

#include "pico/stdlib.h"
#include "dvi_fruitjam.h"
//TODO: #include "tusb.h" or your TinyUSB host wrapper header

COLORPTR framebuffer = 0; //same convention as wm_unixgraphics.h / wm_win32.h etc.

window_manager *g_fj_wm = 0;

int device_start( const UTF8_CHAR *windowname, int xsize, int ysize, int flags, window_manager *wm )
{
    g_fj_wm = wm;

    wm->screen_xsize = xsize;
    wm->screen_ysize = ysize;
    wm->screen_lock_counter = 0;
    wm->screen_is_active = 1;

    wm->fb_xpitch = 1;			//COLOR16BITS -> COLORPTR indexes whole pixels
    wm->fb_ypitch = xsize;		//no padding between scanlines
    wm->fb_offset = 0;

    //Sundog owns this buffer (COLORLEN=2 for COLOR16BITS -> RGB565, matching
    //dvi_fruitjam.c exactly). dvi_init() below hands the DMA engine this same
    //pointer directly, so there's no copy step - Sundog draws into `framebuffer`
    //and the DVI output reflects it continuously, with no explicit present call.
    if( xsize != 640 || ysize != 480 )
    {
	dprint( "ERROR: Fruit Jam DVI driver is fixed at 640x480, got %dx%d\n", xsize, ysize );
	return 1;
    }
    framebuffer = (COLORPTR)MEM_NEW( HEAP_STORAGE, (long)xsize * ysize * COLORLEN );
    if( !framebuffer )
    {
	dprint( "ERROR: Fruit Jam framebuffer alloc failed\n" );
	return 1; //non-zero = failure, matches win_init()'s contract in main.cpp
    }
    mem_set( framebuffer, (long)xsize * ysize * COLORLEN, 0 );

    dvi_init( (uint16_t*)framebuffer );

    //TODO: init TinyUSB host mode here (tuh_init(), root hub port, etc).

    dprint( "WM: Fruit Jam device_start() ok (%dx%d)\n", xsize, ysize );
    return 0;
}

void device_end( window_manager *wm )
{
    //dvi_fruitjam.c has no explicit teardown (matches wili8jam - the DMA/HSTX
    //setup is meant to run for the device's lifetime). Nothing to stop here
    //beyond freeing our own buffer.

    if( framebuffer ) mem_free( framebuffer );
    framebuffer = 0;

    dprint( "WM: Fruit Jam device_end()\n" );
}

//Polled once per engine loop iteration (see EVENT_LOOP_BEGIN/END in main.cpp).
//Pump TinyUSB HID reports here and turn them into send_event() calls.
long device_event_handler( window_manager *wm )
{
    //TODO: call your TinyUSB host task function here, e.g. tuh_task();

    //TODO: for each pending HID report, translate to Sundog key/mouse codes and call:
    //  send_event( 0, EVT_MOUSEMOVE, 0, x, y, 0, 0, 1024, 0, wm );
    //  send_event( 0, EVT_MOUSEBUTTONDOWN, 0, x, y, key, 0, 1024, 0, wm );
    //  send_event( 0, EVT_MOUSEBUTTONUP, 0, x, y, key, 0, 1024, 0, wm );
    //  send_event( 0, EVT_KEYDOWN, 0, 0, 0, key, scancode, 1024, unicode, wm );
    //  send_event( 0, EVT_KEYUP, 0, 0, 0, key, scancode, 1024, unicode, wm );
    //(see wm_unixgraphics.h's device_event_handler() for the equivalent X11 version)

    return 0;
}

void device_screen_lock( WINDOWPTR win, window_manager *wm )
{
    if( wm->screen_lock_counter == 0 )
    {
	//nothing extra needed - we own `framebuffer` directly, no OS surface to lock
    }
    wm->screen_lock_counter++;

    if( wm->screen_lock_counter > 0 )
	wm->screen_is_active = 1;
    else
	wm->screen_is_active = 0;
}

void device_screen_unlock( WINDOWPTR win, window_manager *wm )
{
    if( wm->screen_lock_counter == 1 )
    {
	//nothing extra needed, mirrors device_screen_lock() above
    }
    if( wm->screen_lock_counter > 0 )
    {
	wm->screen_lock_counter--;
    }
}

//device_draw_line / device_draw_box / device_draw_bitmap / device_redraw_framebuffer
//come from here - they already just walk `framebuffer` with fb_xpitch/fb_ypitch/fb_offset,
//so no FRUITJAM-specific changes are needed inside wm_framebuffer.h itself.
#include "wm_framebuffer.h"
