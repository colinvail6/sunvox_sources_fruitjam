/*
    sound_sndout_fruitjam.h
    SunDog audio backend for the Adafruit Fruit Jam (RP2350B), via I2S DMA
    to the onboard TLV320DAC3100.

    Modeled on the double-buffer ping-pong pattern used by WIN_MMSOUND in
    sound_sndout_win.h: two fixed buffers, one being played by DMA while
    the other is refilled by main_sound_output_callback().

    TODO before this compiles: swap the placeholder pico_i2s_* calls below
    for whatever I2S/DMA API you land on (Pico SDK's hardware_i2s examples,
    or a board-support I2S driver if Fruit Jam ships one). The buffer
    lifecycle and g_snd fields are the part that has to stay correct;
    the I2S calls themselves are the part that's genuinely device-specific
    and worth verifying against current Pico SDK docs rather than trusting
    the names used here.
*/

#include "pico/stdlib.h"
#include "hardware/dma.h"
#include <cstdlib>	//malloc/free
//TODO: #include "hardware/i2s.h" or your chosen I2S driver's header

#define FJ_NUM_BUFFERS	2
#define FJ_FRAME_SIZE_BYTES( channels )	( 2 * (channels) ) //always render to int16 for DMA

int   g_fj_buffer_size = DEFAULT_BUFFER_SIZE; //in frames
int   g_fj_channels = 2;
short *g_fj_buffers[ FJ_NUM_BUFFERS ] = { 0, 0 };
volatile int g_fj_active_buffer = 0; //buffer currently owned by DMA
volatile int g_fj_playing = 0;
int   g_fj_dma_channel = -1;

//Called from the I2S/DMA "buffer consumed" IRQ - refill the buffer that just
//finished playing, then hand it back to DMA for the next cycle.
void fj_refill_buffer( int buf_index )
{
    g_snd.out_time = time_ticks() +
	( ( ( ( g_fj_buffer_size << 15 ) / g_snd.freq ) * time_ticks_per_second() ) >> 15 );
    g_snd.out_frames = g_fj_buffer_size;

    if( g_snd.mode & SOUND_MODE_INT16 )
    {
	g_snd.out_buffer = g_fj_buffers[ buf_index ];
	main_sound_output_callback( &g_snd );
    }
    if( g_snd.mode & SOUND_MODE_FLOAT32 )
    {
	//Render float32 into a scratch buffer, then downconvert to int16 in place,
	//same approach as sound_sndout_win.h's DSound path.
	static float *scratch = 0;
	if( !scratch ) scratch = (float*)malloc( g_fj_buffer_size * g_fj_channels * sizeof( float ) );
	g_snd.out_buffer = scratch;
	main_sound_output_callback( &g_snd );
	short *dst = g_fj_buffers[ buf_index ];
	for( int i = 0; i < g_fj_buffer_size * g_fj_channels; i++ )
	{
	    FLOAT32_TO_INT16( dst[ i ], scratch[ i ] );
	}
    }

    //TODO: re-arm DMA to stream g_fj_buffers[buf_index] out over I2S.
    //e.g. dma_channel_set_read_addr( g_fj_dma_channel, g_fj_buffers[ buf_index ], true );
}

//DMA completion IRQ handler - swap buffers and refill the one that just finished.
void fj_i2s_dma_irq_handler( void )
{
    //TODO: acknowledge the DMA IRQ for g_fj_dma_channel here
    //(e.g. dma_hw->ints0 = 1u << g_fj_dma_channel;)

    if( !g_fj_playing ) return;

    int finished = g_fj_active_buffer;
    g_fj_active_buffer = ( g_fj_active_buffer + 1 ) % FJ_NUM_BUFFERS;
    fj_refill_buffer( finished );
}

int device_sound_stream_init( int mode, int freq, int channels )
{
    g_fj_channels = channels;
    g_fj_buffer_size = DEFAULT_BUFFER_SIZE;

    for( int b = 0; b < FJ_NUM_BUFFERS; b++ )
    {
	g_fj_buffers[ b ] = (short*)malloc( g_fj_buffer_size * FJ_FRAME_SIZE_BYTES( channels ) );
	if( !g_fj_buffers[ b ] )
	{
	    dprint( "ERROR: Fruit Jam sound buffer alloc failed\n" );
	    return 1;
	}
	mem_set( g_fj_buffers[ b ], g_fj_buffer_size * FJ_FRAME_SIZE_BYTES( channels ), 0 );
    }

    //TODO: configure I2S peripheral for `freq` Hz, `channels` ch, 16-bit,
    //against the TLV320DAC3100 (I2S pins per Fruit Jam schematic).
    //TODO: claim a DMA channel into g_fj_dma_channel, wire fj_i2s_dma_irq_handler
    //as its IRQ handler via irq_set_exclusive_handler() + irq_set_enabled().

    //Pre-fill both buffers before playback starts, same as WIN_MMSOUND does
    //for its MAX_BUFFERS loop:
    fj_refill_buffer( 0 );
    fj_refill_buffer( 1 );

    dprint( "SOUND: Fruit Jam I2S stream initialized (%d Hz, %d ch)\n", freq, channels );
    return 0;
}

void device_sound_stream_play( void )
{
    g_fj_active_buffer = 0;
    g_fj_playing = 1;
    //TODO: start DMA streaming g_fj_buffers[0] out over I2S.
}

void device_sound_stream_stop( void )
{
    g_snd.stream_stoped = 0;
    g_snd.need_to_stop = 1;
    g_fj_playing = 0;
    //TODO: halt DMA/I2S here.
    g_snd.stream_stoped = 1;
}

void device_sound_stream_close( void )
{
    device_sound_stream_stop();

    //TODO: release the DMA channel and disable I2S clocks.

    for( int b = 0; b < FJ_NUM_BUFFERS; b++ )
    {
	if( g_fj_buffers[ b ] ) free( g_fj_buffers[ b ] );
	g_fj_buffers[ b ] = 0;
    }
}
