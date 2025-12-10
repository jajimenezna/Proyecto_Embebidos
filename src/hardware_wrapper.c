#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// Forward declarations de funciones del middleware
extern int mmf_init(void);
extern int mmf_deinit(void);
extern int mmf_vi_init(void);
extern int mmf_vi_deinit(void);
extern int mmf_get_vi_unused_channel(void);
extern int mmf_add_vi_channel(int ch, int width, int height, int format);
extern int mmf_del_vi_channel(int ch);
extern int mmf_vi_frame_pop(int ch, void** data, int* size, int* width, int* height, int* format);
extern int mmf_vi_frame_free(int ch);
extern int mmf_get_vo_unused_channel(void);
extern int mmf_add_vo_channel(int layer, int ch, int width, int height, int format);
extern int mmf_del_vo_channel(int layer, int ch);
extern int mmf_vo_frame_push(int layer, int ch, void* data, int size, int width, int height, int format);

#define PIXEL_FORMAT_NV21 12

static int g_vi_channel = -1;
static int g_vo_channel = -1;
static int g_initialized = 0;

int hw_init(int width, int height) {
    if (g_initialized) {
        return 0;
    }
    
    if (mmf_init() != 0) {
        printf("ERROR: mmf_init failed\n");
        return -1;
    }
    
    if (mmf_vi_init() != 0) {
        printf("ERROR: mmf_vi_init failed\n");
        mmf_deinit();
        return -1;
    }
    
    g_vi_channel = mmf_get_vi_unused_channel();
    if (mmf_add_vi_channel(g_vi_channel, width, height, PIXEL_FORMAT_NV21) != 0) {
        printf("ERROR: mmf_add_vi_channel failed\n");
        mmf_deinit();
        return -1;
    }
    
    g_initialized = 1;
    return 0;
}

int hw_camera_read(uint8_t** data, int* size, int* width, int* height, int* format) {
    if (!g_initialized || g_vi_channel < 0) {
        return -1;
    }
    
    void* frame_data;
    if (mmf_vi_frame_pop(g_vi_channel, &frame_data, size, width, height, format) != 0) {
        return -1;
    }
    
    *data = (uint8_t*)frame_data;
    return 0;
}

void hw_camera_free(void) {
    if (g_initialized && g_vi_channel >= 0) {
        mmf_vi_frame_free(g_vi_channel);
    }
}

int hw_display_init(int layer, int width, int height, int format) {
    if (!g_initialized) {
        return -1;
    }
    
    g_vo_channel = mmf_get_vo_unused_channel();
    if (mmf_add_vo_channel(layer, g_vo_channel, width, height, format) != 0) {
        printf("ERROR: mmf_add_vo_channel failed\n");
        return -1;
    }
    
    return 0;
}

int hw_display_show(int layer, uint8_t* data, int size, int width, int height, int format) {
    if (!g_initialized || g_vo_channel < 0) {
        return -1;
    }
    
    return mmf_vo_frame_push(layer, g_vo_channel, data, size, width, height, format);
}

void hw_deinit(void) {
    if (g_initialized) {
        if (g_vi_channel >= 0) {
            mmf_del_vi_channel(g_vi_channel);
        }
        if (g_vo_channel >= 0) {
            mmf_del_vo_channel(0, g_vo_channel);
        }
        mmf_deinit();
        g_initialized = 0;
    }
}
