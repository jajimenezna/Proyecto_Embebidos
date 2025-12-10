#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// Variables globales para métricas
static float g_last_ear = 0.0f;
static bool g_initialized = false;

// Declaraciones de funciones existentes en drowsiness_c.c
extern void extract_and_scale_landmarks(float* all_landmarks_478, int face_x, int face_y, 
                                       float* left_eye_data, int* left_count,
                                       float* right_eye_data, int* right_count,
                                       float* mouth_data, int* mouth_count);

typedef struct {
    float ear_left;
    float ear_right;
    float ear_avg;
    float mar;
    bool eyes_closed;
    int mouth_state;
    float drowsiness_level;
    int blink_count;
    float eyes_closed_duration;
    int blinks_per_minute;
    int danger_level;
    char danger_text[100];
} DrowsinessMetrics;

extern void analyze_drowsiness(float* left_eye_data, int left_count,
                              float* right_eye_data, int right_count,
                              float* mouth_data, int mouth_count,
                              DrowsinessMetrics* metrics);

extern void reset_drowsiness(void);

// Funciones wrapper para Python
__attribute__((visibility("default"))) int drowsiness_init(void) {
    if (g_initialized) {
        return 0;
    }
    reset_drowsiness();
    g_initialized = true;
    return 0;
}

__attribute__((visibility("default"))) int drowsiness_process_frame(unsigned char* image_data, int width, int height) {
    // Por ahora solo retorna éxito
    // La detección real se hace con YOLO en Python
    return 0;
}

__attribute__((visibility("default"))) float drowsiness_get_ear(void) {
    return g_last_ear;
}

__attribute__((visibility("default"))) void drowsiness_cleanup(void) {
    g_initialized = false;
}

// Función para procesar landmarks desde Python
int process_landmarks(float* landmarks, int num_points, float* ear_out) {
    if (!g_initialized || !landmarks || num_points != 478) {
        return -1;
    }
    
    float left_eye[12], right_eye[12], mouth[8];
    int left_count, right_count, mouth_count;
    
    // Extraer landmarks de ojos y boca
    extract_and_scale_landmarks(landmarks, 0, 0, 
                               left_eye, &left_count,
                               right_eye, &right_count,
                               mouth, &mouth_count);
    
    // Analizar somnolencia
    DrowsinessMetrics metrics;
    analyze_drowsiness(left_eye, left_count,
                      right_eye, right_count,
                      mouth, mouth_count,
                      &metrics);
    
    g_last_ear = metrics.ear_avg;
    if (ear_out) {
        *ear_out = metrics.ear_avg;
    }
    
    return metrics.danger_level;
}
