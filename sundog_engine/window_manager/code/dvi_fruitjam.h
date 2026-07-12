#ifndef DVI_FRUITJAM_H
#define DVI_FRUITJAM_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Initialize HSTX DVI output for native 640x480@60Hz.
// framebuffer: pointer to a 640x480 RGB565 buffer (614400 bytes) - pass
// Sundog's own `framebuffer` global (see wm_fruitjam.h) directly. The
// display continuously scans out this buffer via DMA with no CPU
// involvement after init, and no scaling/letterboxing (1:1 pixel mapping).
void dvi_init(uint16_t *framebuffer);

#ifdef __cplusplus
}
#endif

#endif // DVI_FRUITJAM_H
