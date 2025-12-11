#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#define HISTORY_SIZE 10

static bool g_initialized = false;
static float g_apertura_history[HISTORY_SIZE];
static int g_history_idx = 0;
static int g_history_count = 0;
static int g_blink_count = 0;
static int g_closed_frames = 0;
static bool g_eyes_closed = false;

// Índices MediaPipe - PROBAR VARIOS HASTA ENCONTRAR LOS CORRECTOS
// Estos son aproximados, pueden necesitar ajuste
static int LEFT_EYE_TOP[] = {159, 145, 386};      
static int LEFT_EYE_BOTTOM[] = {144, 153, 374};   
static int RIGHT_EYE_TOP[] = {386, 374, 159};     
static int RIGHT_EYE_BOTTOM[] = {373, 380, 144};

__attribute__((visibility("default")))
int drowsiness_init(void) {
    g_initialized = true;
    g_blink_count = 0;
    g_history_idx = 0;
    g_history_count = 0;
    g_closed_frames = 0;
    g_eyes_closed = false;
    memset(g_apertura_history, 0, sizeof(g_apertura_history));
    printf("[C] Detector inicializado\n");
    return 0;
}

static float get_avg_apertura() {
    if (g_history_count == 0) return 0.0f;
    float sum = 0.0f;
    int count = (g_history_count < HISTORY_SIZE) ? g_history_count : HISTORY_SIZE;
    for (int i = 0; i < count; i++) {
        sum += g_apertura_history[i];
    }
    return sum / count;
}

__attribute__((visibility("default")))
int process_landmarks(float* landmarks, int num_points, float* ear_out) {
    if (!g_initialized || !landmarks || num_points != 478) {
        return -1;
    }
    
    // Landmarks vienen en rango [0, 256] directamente desde el modelo
    // Calcular apertura vertical promedio
    float left_v = 0.0f, right_v = 0.0f;
    int left_count = 0, right_count = 0;
    
    // Probar múltiples pares de puntos y promediar
    for (int i = 0; i < 3; i++) {
        int top_idx = LEFT_EYE_TOP[i];
        int bottom_idx = LEFT_EYE_BOTTOM[i];
        if (top_idx < 478 && bottom_idx < 478) {
            float dist = fabsf(landmarks[top_idx*2 + 1] - landmarks[bottom_idx*2 + 1]);
            if (dist > 0.1f) {  // Filtrar valores inválidos
                left_v += dist;
                left_count++;
            }
        }
    }
    
    for (int i = 0; i < 3; i++) {
        int top_idx = RIGHT_EYE_TOP[i];
        int bottom_idx = RIGHT_EYE_BOTTOM[i];
        if (top_idx < 478 && bottom_idx < 478) {
            float dist = fabsf(landmarks[top_idx*2 + 1] - landmarks[bottom_idx*2 + 1]);
            if (dist > 0.1f) {
                right_v += dist;
                right_count++;
            }
        }
    }
    
    if (left_count == 0 || right_count == 0) {
        return -1;  // No se pudieron calcular distancias válidas
    }
    
    float apertura = ((left_v / left_count) + (right_v / right_count)) / 2.0f;
    
    if (ear_out) {
        *ear_out = apertura;
    }
    
    // Guardar en historial
    g_apertura_history[g_history_idx] = apertura;
    g_history_idx = (g_history_idx + 1) % HISTORY_SIZE;
    if (g_history_count < HISTORY_SIZE) g_history_count++;
    
    if (g_history_count < 5) {
        return 0;
    }
    
    // Detectar por caída brusca
    float avg = get_avg_apertura();
    float drop_percent = ((avg - apertura) / avg) * 100.0f;
    
    bool eyes_currently_closed = (drop_percent > 30.0f);  // Caída del 30%
    
    if (eyes_currently_closed) {
        if (!g_eyes_closed) {
            g_eyes_closed = true;
            g_closed_frames = 1;
        } else {
            g_closed_frames++;
        }
    } else {
        if (g_eyes_closed) {
            if (g_closed_frames > 2 && g_closed_frames < 15) {
                g_blink_count++;
            }
            g_eyes_closed = false;
            g_closed_frames = 0;
        }
    }
    
    int danger = 0;
    if (g_closed_frames > 30) danger = 2;
    else if (g_closed_frames > 15) danger = 1;
    
    static int fc = 0;
    if (fc++ % 10 == 0) {
        printf("[C] Aper=%.1f, Prom=%.1f, Caída=%.0f%%, %s, Frames=%d, Parp=%d\n", 
               apertura, avg, drop_percent,
               g_eyes_closed ? "CERR" : "ABIE",
               g_closed_frames, g_blink_count);
    }
    
    return danger;
}

__attribute__((visibility("default")))
float drowsiness_get_ear(void) {
    return get_avg_apertura();
}

__attribute__((visibility("default")))
void drowsiness_cleanup(void) {
    g_initialized = false;
    printf("[C] Detector finalizado - Parpadeos: %d\n", g_blink_count);
}
