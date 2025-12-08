/**
 * DETECTOR DE SOMNOLENCIA PARA CONDUCTORES - C PURO
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <stdbool.h>

typedef struct {
    float x, y;
} Point2f;

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

// Estado global
static float g_drowsiness_level = 0.0f;
static bool g_previous_eyes_closed = false;
static int g_total_blinks = 0;
static time_t g_eyes_closed_start = 0;
static time_t g_last_decay = 0;
static time_t g_session_start = 0;
static int g_microsleep_count = 0;
static int g_long_blinks = 0;
static int g_yawn_count = 0;

static float distance(float x1, float y1, float x2, float y2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    return sqrtf(dx*dx + dy*dy);
}

static float calculate_ear(Point2f* eye_points, int count) {
    if (count < 3) return 0.0f;
    float horizontal = distance(eye_points[0].x, eye_points[0].y, eye_points[2].x, eye_points[2].y);
    float vertical = fabsf(eye_points[1].y - (eye_points[0].y + eye_points[2].y)/2.0f) * 2.0f;
    return horizontal < 1e-6f ? 0.0f : vertical / horizontal;
}

static float calculate_mar(Point2f* mouth_points, int count) {
    if (count < 4) return 0.0f;
    float vertical = distance(mouth_points[1].x, mouth_points[1].y, mouth_points[2].x, mouth_points[2].y);
    float horizontal = distance(mouth_points[0].x, mouth_points[0].y, mouth_points[3].x, mouth_points[3].y);
    return horizontal < 1e-6f ? 0.0f : vertical / horizontal;
}

void analyze_drowsiness(float* left_eye_data, int left_count,
                       float* right_eye_data, int right_count,
                       float* mouth_data, int mouth_count,
                       DrowsinessMetrics* result) {
    
    Point2f left_eye[6], right_eye[6], mouth[4];
    
    for (int i = 0; i < left_count && i < 6; i++) {
        left_eye[i].x = left_eye_data[i*2];
        left_eye[i].y = left_eye_data[i*2 + 1];
    }
    for (int i = 0; i < right_count && i < 6; i++) {
        right_eye[i].x = right_eye_data[i*2];
        right_eye[i].y = right_eye_data[i*2 + 1];
    }
    for (int i = 0; i < mouth_count && i < 4; i++) {
        mouth[i].x = mouth_data[i*2];
        mouth[i].y = mouth_data[i*2 + 1];
    }
    
    time_t now = time(NULL);
    if (g_session_start == 0) {
        g_session_start = now;
        g_last_decay = now;
    }
    
    result->ear_left = calculate_ear(left_eye, left_count);
    result->ear_right = calculate_ear(right_eye, right_count);
    result->ear_avg = (result->ear_left + result->ear_right) / 2.0f;
    result->mar = calculate_mar(mouth, mouth_count);
    result->eyes_closed = (result->ear_avg < 0.18f);
    
    // Estado de boca
    if (result->mar >= 0.70f) {
        result->mouth_state = 2;
        if (g_previous_eyes_closed == false) {
            g_yawn_count++;
        }
    } else if (result->mar >= 0.40f) {
        result->mouth_state = 1;
    } else {
        result->mouth_state = 0;
    }
    
    // ANÁLISIS DE PARPADEOS
    if (result->eyes_closed) {
        if (!g_previous_eyes_closed) {
            g_eyes_closed_start = now;
            g_total_blinks++;
        }
        
        float duration = difftime(now, g_eyes_closed_start);
        result->eyes_closed_duration = duration;
        
        // MICROSUEÑO: Ojos cerrados > 2.5 segundos (más estricto)
        if (duration > 2.5f) {
            g_drowsiness_level = fminf(100.0f, g_drowsiness_level + 40.0f);
            if (duration >= 2.5f && duration < 3.0f) {
                g_microsleep_count++;
            }
        }
        // PARPADEO LARGO: 1.0 - 2.5 segundos
        else if (duration > 1.0f && duration <= 2.5f) {
            g_drowsiness_level = fminf(100.0f, g_drowsiness_level + 15.0f);
            if (duration >= 1.0f && duration < 1.2f) {
                g_long_blinks++;
            }
        }
        // Parpadeo normal: < 1.0 segundo (NO incrementa mucho)
        else if (duration <= 1.0f) {
            g_drowsiness_level = fminf(100.0f, g_drowsiness_level + 2.0f);
        }
    } else {
        result->eyes_closed_duration = 0.0f;
    }
    
    // Bostezo
    if (result->mouth_state == 2) {
        g_drowsiness_level = fminf(100.0f, g_drowsiness_level + 10.0f);
    }
    
    // Decaimiento más rápido si todo está bien
    if (difftime(now, g_last_decay) >= 5.0) {
        g_drowsiness_level = fmaxf(0.0f, g_drowsiness_level - 5.0f);
        g_last_decay = now;
    }
    
    // Parpadeos por minuto
    float minutes_elapsed = difftime(now, g_session_start) / 60.0f;
    if (minutes_elapsed > 0.0f) {
        result->blinks_per_minute = (int)(g_total_blinks / minutes_elapsed);
    } else {
        result->blinks_per_minute = 0;
    }
    
    result->drowsiness_level = g_drowsiness_level;
    result->blink_count = g_total_blinks;
    
    // NIVEL DE PELIGRO AJUSTADO
    // Peligro crítico: 3+ microsueños O nivel muy alto
    if (g_microsleep_count >= 3 || g_drowsiness_level >= 80.0f) {
        result->danger_level = 2;
        strcpy(result->danger_text, "DETENER VEHICULO");
    }
    // Cansancio: 5+ parpadeos largos O 3+ bostezos O nivel medio-alto
    else if (g_long_blinks >= 5 || g_yawn_count >= 3 || g_drowsiness_level >= 60.0f) {
        result->danger_level = 1;
        strcpy(result->danger_text, "DESCANSO NECESARIO");
    }
    // Seguro
    else {
        result->danger_level = 0;
        strcpy(result->danger_text, "CONDUCCION SEGURA");
    }
    
    g_previous_eyes_closed = result->eyes_closed;
}

void reset_drowsiness(void) {
    g_drowsiness_level = 0.0f;
    g_previous_eyes_closed = false;
    g_total_blinks = 0;
    g_session_start = time(NULL);
    g_last_decay = g_session_start;
    g_microsleep_count = 0;
    g_long_blinks = 0;
    g_yawn_count = 0;
}
