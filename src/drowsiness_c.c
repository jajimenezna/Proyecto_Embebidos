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

typedef struct {
    int x, y, radius;
    unsigned char r, g, b;
} CircleToDraw;

typedef struct {
    int x, y, w, h;
    unsigned char r, g, b;
    int thickness;
} RectToDraw;

typedef struct {
    int x, y;
    char text[200];
    unsigned char r, g, b;
    float scale;
} TextToDraw;

typedef struct {
    CircleToDraw circles[20];
    int circle_count;
    RectToDraw rects[10];
    int rect_count;
    TextToDraw texts[15];
    int text_count;
} RenderCommands;

/* Variables globales de estado */
static float g_drowsiness_level = 0.0f;
static bool g_previous_eyes_closed = false;
static int g_total_blinks = 0;
static time_t g_eyes_closed_start = 0;
static time_t g_last_decay = 0;
static time_t g_session_start = 0;
static int g_microsleep_count = 0;
static int g_long_blinks = 0;
static int g_yawn_count = 0;

/* Distancia euclidiana */
static float distance(float x1, float y1, float x2, float y2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    return sqrtf(dx*dx + dy*dy);
}

/* Cálculo de EAR */
static float calculate_ear(Point2f* eye_points, int count) {
    if (count < 3) return 0.0f;
    float horizontal = distance(eye_points[0].x, eye_points[0].y, 
                                eye_points[2].x, eye_points[2].y);
    float vertical = fabsf(eye_points[1].y - (eye_points[0].y + eye_points[2].y)/2.0f) * 2.0f;
    return horizontal < 1e-6f ? 0.0f : vertical / horizontal;
}

/* Cálculo de MAR */
static float calculate_mar(Point2f* mouth_points, int count) {
    if (count < 4) return 0.0f;
    float vertical = distance(mouth_points[1].x, mouth_points[1].y, 
                             mouth_points[2].x, mouth_points[2].y);
    float horizontal = distance(mouth_points[0].x, mouth_points[0].y, 
                               mouth_points[3].x, mouth_points[3].y);
    return horizontal < 1e-6f ? 0.0f : vertical / horizontal;
}

/* Extracción y escalado de landmarks desde array completo */
void extract_and_scale_landmarks(float* all_landmarks_478, int face_x, int face_y, 
                                int face_w, int face_h,
                                int* left_eye_indices, int left_count,
                                int* right_eye_indices, int right_count,
                                int* mouth_indices, int mouth_count,
                                float* left_out, float* right_out, float* mouth_out) {
    
    float scale_x = face_w / 256.0f;
    float scale_y = face_h / 256.0f;
    
    int i;
    for (i = 0; i < left_count; i++) {
        int idx = left_eye_indices[i];
        left_out[i*2] = all_landmarks_478[idx*2] * scale_x + face_x;
        left_out[i*2+1] = all_landmarks_478[idx*2+1] * scale_y + face_y;
    }
    
    for (i = 0; i < right_count; i++) {
        int idx = right_eye_indices[i];
        right_out[i*2] = all_landmarks_478[idx*2] * scale_x + face_x;
        right_out[i*2+1] = all_landmarks_478[idx*2+1] * scale_y + face_y;
    }
    
    for (i = 0; i < mouth_count; i++) {
        int idx = mouth_indices[i];
        mouth_out[i*2] = all_landmarks_478[idx*2] * scale_x + face_x;
        mouth_out[i*2+1] = all_landmarks_478[idx*2+1] * scale_y + face_y;
    }
}

/* Análisis principal */
void analyze_drowsiness(float* left_eye_data, int left_count,
                       float* right_eye_data, int right_count,
                       float* mouth_data, int mouth_count,
                       DrowsinessMetrics* result) {
    
    Point2f left_eye[6], right_eye[6], mouth[4];
    int i;
    
    /* Conversión a Point2f */
    for (i = 0; i < left_count && i < 6; i++) {
        left_eye[i].x = left_eye_data[i*2];
        left_eye[i].y = left_eye_data[i*2 + 1];
    }
    for (i = 0; i < right_count && i < 6; i++) {
        right_eye[i].x = right_eye_data[i*2];
        right_eye[i].y = right_eye_data[i*2 + 1];
    }
    for (i = 0; i < mouth_count && i < 4; i++) {
        mouth[i].x = mouth_data[i*2];
        mouth[i].y = mouth_data[i*2 + 1];
    }
    
    time_t now = time(NULL);
    if (g_session_start == 0) {
        g_session_start = now;
        g_last_decay = now;
    }
    
    /* Cálculo de métricas */
    result->ear_left = calculate_ear(left_eye, left_count);
    result->ear_right = calculate_ear(right_eye, right_count);
    result->ear_avg = (result->ear_left + result->ear_right) / 2.0f;
    result->mar = calculate_mar(mouth, mouth_count);
    result->eyes_closed = (result->ear_avg < 0.18f);
    
    /* Estado de boca */
    if (result->mar >= 0.70f) {
        result->mouth_state = 2;
        if (!g_previous_eyes_closed) {
            g_yawn_count++;
        }
    } else if (result->mar >= 0.40f) {
        result->mouth_state = 1;
    } else {
        result->mouth_state = 0;
    }
    
    /* Análisis de parpadeos */
    if (result->eyes_closed) {
        if (!g_previous_eyes_closed) {
            g_eyes_closed_start = now;
            g_total_blinks++;
        }
        
        float duration = difftime(now, g_eyes_closed_start);
        result->eyes_closed_duration = duration;
        
        if (duration > 2.5f) {
            g_drowsiness_level = fminf(100.0f, g_drowsiness_level + 40.0f);
            if (duration >= 2.5f && duration < 3.0f) {
                g_microsleep_count++;
            }
        } else if (duration > 1.0f) {
            g_drowsiness_level = fminf(100.0f, g_drowsiness_level + 15.0f);
            if (duration >= 1.0f && duration < 1.2f) {
                g_long_blinks++;
            }
        } else {
            g_drowsiness_level = fminf(100.0f, g_drowsiness_level + 2.0f);
        }
    } else {
        result->eyes_closed_duration = 0.0f;
    }
    
    /* Bostezo */
    if (result->mouth_state == 2) {
        g_drowsiness_level = fminf(100.0f, g_drowsiness_level + 10.0f);
    }
    
    /* Decaimiento */
    if (difftime(now, g_last_decay) >= 5.0) {
        g_drowsiness_level = fmaxf(0.0f, g_drowsiness_level - 5.0f);
        g_last_decay = now;
    }
    
    /* Parpadeos por minuto */
    float minutes_elapsed = difftime(now, g_session_start) / 60.0f;
    if (minutes_elapsed > 0.0f) {
        result->blinks_per_minute = (int)(g_total_blinks / minutes_elapsed);
    } else {
        result->blinks_per_minute = 0;
    }
    
    result->drowsiness_level = g_drowsiness_level;
    result->blink_count = g_total_blinks;
    
    /* Evaluación de peligro */
    if (g_microsleep_count >= 3 || g_drowsiness_level >= 80.0f) {
        result->danger_level = 2;
        strcpy(result->danger_text, "DETENER VEHICULO");
    } else if (g_long_blinks >= 5 || g_yawn_count >= 3 || g_drowsiness_level >= 60.0f) {
        result->danger_level = 1;
        strcpy(result->danger_text, "DESCANSO NECESARIO");
    } else {
        result->danger_level = 0;
        strcpy(result->danger_text, "CONDUCCION SEGURA");
    }
    
    g_previous_eyes_closed = result->eyes_closed;
}

/* Generación de comandos de renderizado */
void generate_render_commands(float* left_eye_data, int left_count,
                              float* right_eye_data, int right_count,
                              float* mouth_data, int mouth_count,
                              int face_x, int face_y, int face_w, int face_h,
                              DrowsinessMetrics* metrics,
                              RenderCommands* commands) {
    
    commands->circle_count = 0;
    commands->rect_count = 0;
    commands->text_count = 0;
    
    int i;
    
    /* Rectángulo de cara */
    commands->rects[commands->rect_count].x = face_x;
    commands->rects[commands->rect_count].y = face_y;
    commands->rects[commands->rect_count].w = face_w;
    commands->rects[commands->rect_count].h = face_h;
    commands->rects[commands->rect_count].r = 0;
    commands->rects[commands->rect_count].g = 255;
    commands->rects[commands->rect_count].b = 0;
    commands->rects[commands->rect_count].thickness = 1;
    commands->rect_count++;
    
    /* Círculos de ojos */
    unsigned char eye_r = metrics->eyes_closed ? 255 : 0;
    unsigned char eye_g = metrics->eyes_closed ? 0 : 255;
    
    for (i = 0; i < left_count && i < 6; i++) {
        commands->circles[commands->circle_count].x = (int)left_eye_data[i*2];
        commands->circles[commands->circle_count].y = (int)left_eye_data[i*2+1];
        commands->circles[commands->circle_count].radius = 3;
        commands->circles[commands->circle_count].r = eye_r;
        commands->circles[commands->circle_count].g = eye_g;
        commands->circles[commands->circle_count].b = 0;
        commands->circle_count++;
    }
    
    for (i = 0; i < right_count && i < 6; i++) {
        commands->circles[commands->circle_count].x = (int)right_eye_data[i*2];
        commands->circles[commands->circle_count].y = (int)right_eye_data[i*2+1];
        commands->circles[commands->circle_count].radius = 3;
        commands->circles[commands->circle_count].r = eye_r;
        commands->circles[commands->circle_count].g = eye_g;
        commands->circles[commands->circle_count].b = 0;
        commands->circle_count++;
    }
    
    /* Círculos de boca */
    unsigned char mouth_r = (metrics->mouth_state == 2) ? 255 : (metrics->mouth_state == 1 ? 255 : 0);
    unsigned char mouth_g = (metrics->mouth_state == 1) ? 255 : 0;
    
    for (i = 0; i < mouth_count && i < 4; i++) {
        commands->circles[commands->circle_count].x = (int)mouth_data[i*2];
        commands->circles[commands->circle_count].y = (int)mouth_data[i*2+1];
        commands->circles[commands->circle_count].radius = 3;
        commands->circles[commands->circle_count].r = mouth_r;
        commands->circles[commands->circle_count].g = mouth_g;
        commands->circles[commands->circle_count].b = 0;
        commands->circle_count++;
    }
    
    /* Textos de métricas */
    snprintf(commands->texts[commands->text_count].text, 200, "Parpadeos: %d", metrics->blink_count);
    commands->texts[commands->text_count].x = 10;
    commands->texts[commands->text_count].y = 80;
    commands->texts[commands->text_count].r = 255;
    commands->texts[commands->text_count].g = 255;
    commands->texts[commands->text_count].b = 255;
    commands->texts[commands->text_count].scale = 1.3f;
    commands->text_count++;
    
    snprintf(commands->texts[commands->text_count].text, 200, "Por min: %d", metrics->blinks_per_minute);
    commands->texts[commands->text_count].x = 10;
    commands->texts[commands->text_count].y = 110;
    commands->texts[commands->text_count].r = 255;
    commands->texts[commands->text_count].g = 255;
    commands->texts[commands->text_count].b = 255;
    commands->texts[commands->text_count].scale = 1.3f;
    commands->text_count++;
    
    snprintf(commands->texts[commands->text_count].text, 200, "EAR: %.2f", metrics->ear_avg);
    commands->texts[commands->text_count].x = 10;
    commands->texts[commands->text_count].y = 140;
    commands->texts[commands->text_count].r = 0;
    commands->texts[commands->text_count].g = 200;
    commands->texts[commands->text_count].b = 255;
    commands->texts[commands->text_count].scale = 1.2f;
    commands->text_count++;
    
    /* Alerta de peligro */
    if (metrics->danger_level > 0) {
        unsigned char alert_r = (metrics->danger_level == 2) ? 255 : 255;
        unsigned char alert_g = (metrics->danger_level == 1) ? 200 : 0;
        unsigned char alert_b = 0;
        
        commands->rects[commands->rect_count].x = 150;
        commands->rects[commands->rect_count].y = 200;
        commands->rects[commands->rect_count].w = 350;
        commands->rects[commands->rect_count].h = 80;
        commands->rects[commands->rect_count].r = alert_r;
        commands->rects[commands->rect_count].g = alert_g;
        commands->rects[commands->rect_count].b = alert_b;
        commands->rects[commands->rect_count].thickness = -1;
        commands->rect_count++;
        
        unsigned char text_r = (metrics->danger_level == 1) ? 0 : 255;
        unsigned char text_g = (metrics->danger_level == 1) ? 0 : 255;
        unsigned char text_b = (metrics->danger_level == 1) ? 0 : 255;
        
        snprintf(commands->texts[commands->text_count].text, 200, "%s", metrics->danger_text);
        commands->texts[commands->text_count].x = 160;
        commands->texts[commands->text_count].y = 230;
        commands->texts[commands->text_count].r = text_r;
        commands->texts[commands->text_count].g = text_g;
        commands->texts[commands->text_count].b = text_b;
        commands->texts[commands->text_count].scale = 1.8f;
        commands->text_count++;
    }
}

/* Reinicio */
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
