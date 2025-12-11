#include <math.h>
#include <stdbool.h>

#define HISTORY_SIZE 10

static bool g_initialized = false;
static float g_ear_history[HISTORY_SIZE];
static float g_mar_history[HISTORY_SIZE];
static int g_history_idx = 0;
static int g_history_count = 0;
static int g_blink_count = 0;
static int g_yawn_count = 0;
static int g_closed_frames = 0;
static bool g_eyes_closed = false;

typedef struct {
    float x, y;
} Point2D;

static float distance_2d(Point2D a, Point2D b) {
    float dx = b.x - a.x;
    float dy = b.y - a.y;
    return sqrtf(dx*dx + dy*dy);
}

int drowsiness_init(void) {
    g_initialized = true;
    g_blink_count = 0;
    g_yawn_count = 0;
    g_history_idx = 0;
    g_history_count = 0;
    g_closed_frames = 0;
    g_eyes_closed = false;
    
    int i;
    for (i = 0; i < HISTORY_SIZE; i++) {
        g_ear_history[i] = 0.0f;
        g_mar_history[i] = 0.0f;
    }
    
    return 0;
}

int process_5_landmarks(float* landmarks, float* ear_out, float* mar_out) {
    if (!g_initialized || !landmarks) {
        return -1;
    }
    
    Point2D eye_left = {landmarks[0], landmarks[1]};
    Point2D eye_right = {landmarks[2], landmarks[3]};
    Point2D nose = {landmarks[4], landmarks[5]};
    Point2D mouth_left = {landmarks[6], landmarks[7]};
    Point2D mouth_right = {landmarks[8], landmarks[9]};
    
    float eye_dist = distance_2d(eye_left, eye_right);
    float nose_to_eyes_avg = (distance_2d(nose, eye_left) + distance_2d(nose, eye_right)) / 2.0f;
    float ear = nose_to_eyes_avg / (eye_dist + 0.1f);
    
    float mouth_width = distance_2d(mouth_left, mouth_right);
    float mouth_to_nose_avg = (distance_2d(nose, mouth_left) + distance_2d(nose, mouth_right)) / 2.0f;
    float mar = mouth_to_nose_avg / (mouth_width + 0.1f);
    
    g_ear_history[g_history_idx] = ear;
    g_mar_history[g_history_idx] = mar;
    g_history_idx = (g_history_idx + 1) % HISTORY_SIZE;
    if (g_history_count < HISTORY_SIZE) g_history_count++;
    
    float ear_avg = 0.0f, mar_avg = 0.0f;
    int count = (g_history_count < HISTORY_SIZE) ? g_history_count : HISTORY_SIZE;
    int i;
    for (i = 0; i < count; i++) {
        ear_avg += g_ear_history[i];
        mar_avg += g_mar_history[i];
    }
    ear_avg /= count;
    mar_avg /= count;
    
    if (ear_out) *ear_out = ear_avg;
    if (mar_out) *mar_out = mar_avg;
    
    if (g_history_count < 5) {
        return 0;
    }
    
    bool eyes_currently_closed = (ear_avg < 0.45f);
    
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
    
    if (mar_avg > 0.75f) {
        g_yawn_count++;
    }
    
    int danger = 0;
    if (g_closed_frames > 30) {
        danger = 2;
    } else if (g_closed_frames > 15) {
        danger = 1;
    }
    
    return danger;
}

int drowsiness_get_blink_count(void) {
    return g_blink_count;
}

int drowsiness_get_yawn_count(void) {
    return g_yawn_count;
}

void drowsiness_cleanup(void) {
    g_initialized = false;
}
