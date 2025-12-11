#include <math.h>

#define HISTORY_SIZE 10
#define LANDMARK_SMOOTH 5

typedef struct { float x, y; } Point2D;

static float g_ear_history[HISTORY_SIZE];
static float g_mar_history[HISTORY_SIZE];
static int g_history_idx = 0;
static int g_history_count = 0;
static int g_blink_count = 0;
static int g_yawn_count = 0;
static int g_closed_frames = 0;
static int g_eyes_closed = 0;
static int g_initialized = 0;

static float g_landmark_buffer[478][2][LANDMARK_SMOOTH];
static int g_landmark_idx = 0;
static int g_landmark_filled = 0;

static float dist(Point2D a, Point2D b) {
    float dx = b.x - a.x;
    float dy = b.y - a.y;
    return sqrtf(dx*dx + dy*dy);
}

__attribute__((visibility("default")))
int drowsiness_init(void) {
    int i, j, k;
    g_initialized = 1;
    g_blink_count = 0;
    g_yawn_count = 0;
    g_history_idx = 0;
    g_history_count = 0;
    g_closed_frames = 0;
    g_eyes_closed = 0;
    g_landmark_idx = 0;
    g_landmark_filled = 0;
    
    for (i = 0; i < HISTORY_SIZE; i++) {
        g_ear_history[i] = 0.0f;
        g_mar_history[i] = 0.0f;
    }
    
    for (i = 0; i < 478; i++) {
        for (j = 0; j < 2; j++) {
            for (k = 0; k < LANDMARK_SMOOTH; k++) {
                g_landmark_buffer[i][j][k] = 0.0f;
            }
        }
    }
    
    return 0;
}

static void smooth_landmarks(float* raw_landmarks, float* smoothed_landmarks, int num_points) {
    int i, j;
    
    for (i = 0; i < num_points; i++) {
        g_landmark_buffer[i][0][g_landmark_idx] = raw_landmarks[i*2];
        g_landmark_buffer[i][1][g_landmark_idx] = raw_landmarks[i*2 + 1];
    }
    
    g_landmark_idx = (g_landmark_idx + 1) % LANDMARK_SMOOTH;
    if (g_landmark_filled < LANDMARK_SMOOTH) g_landmark_filled++;
    
    int count = g_landmark_filled;
    for (i = 0; i < num_points; i++) {
        float sum_x = 0.0f, sum_y = 0.0f;
        for (j = 0; j < count; j++) {
            sum_x += g_landmark_buffer[i][0][j];
            sum_y += g_landmark_buffer[i][1][j];
        }
        smoothed_landmarks[i*2] = sum_x / count;
        smoothed_landmarks[i*2 + 1] = sum_y / count;
    }
}

__attribute__((visibility("default")))
int process_478_landmarks(float* lm, float* ear_out, float* mar_out) {
    if (!g_initialized || !lm) return -1;
    
    float smoothed[956];
    smooth_landmarks(lm, smoothed, 478);
    
    int left_eye_v[] = {159, 145};
    int left_eye_h[] = {33, 133};
    int right_eye_v[] = {386, 374};
    int right_eye_h[] = {362, 263};
    
    Point2D le_top = {smoothed[left_eye_v[0]*2], smoothed[left_eye_v[0]*2+1]};
    Point2D le_bot = {smoothed[left_eye_v[1]*2], smoothed[left_eye_v[1]*2+1]};
    Point2D le_left = {smoothed[left_eye_h[0]*2], smoothed[left_eye_h[0]*2+1]};
    Point2D le_right = {smoothed[left_eye_h[1]*2], smoothed[left_eye_h[1]*2+1]};
    
    float le_vert = dist(le_top, le_bot);
    float le_horiz = dist(le_left, le_right);
    float ear_left = le_vert / (le_horiz + 0.1f);
    
    Point2D re_top = {smoothed[right_eye_v[0]*2], smoothed[right_eye_v[0]*2+1]};
    Point2D re_bot = {smoothed[right_eye_v[1]*2], smoothed[right_eye_v[1]*2+1]};
    Point2D re_left = {smoothed[right_eye_h[0]*2], smoothed[right_eye_h[0]*2+1]};
    Point2D re_right = {smoothed[right_eye_h[1]*2], smoothed[right_eye_h[1]*2+1]};
    
    float re_vert = dist(re_top, re_bot);
    float re_horiz = dist(re_left, re_right);
    float ear_right = re_vert / (re_horiz + 0.1f);
    
    float ear = (ear_left + ear_right) / 2.0f;
    
    int mouth_v[] = {13, 14};
    int mouth_h[] = {61, 291};
    
    Point2D m_top = {smoothed[mouth_v[0]*2], smoothed[mouth_v[0]*2+1]};
    Point2D m_bot = {smoothed[mouth_v[1]*2], smoothed[mouth_v[1]*2+1]};
    Point2D m_left = {smoothed[mouth_h[0]*2], smoothed[mouth_h[0]*2+1]};
    Point2D m_right = {smoothed[mouth_h[1]*2], smoothed[mouth_h[1]*2+1]};
    
    float m_vert = dist(m_top, m_bot);
    float m_horiz = dist(m_left, m_right);
    float mar = m_vert / (m_horiz + 0.1f);
    
    g_ear_history[g_history_idx] = ear;
    g_mar_history[g_history_idx] = mar;
    g_history_idx = (g_history_idx + 1) % HISTORY_SIZE;
    if (g_history_count < HISTORY_SIZE) g_history_count++;
    
    float ea = 0.0f, ma = 0.0f;
    int c = (g_history_count < HISTORY_SIZE) ? g_history_count : HISTORY_SIZE;
    int i;
    for (i = 0; i < c; i++) {
        ea += g_ear_history[i];
        ma += g_mar_history[i];
    }
    ea /= c;
    ma /= c;
    
    if (ear_out) *ear_out = ea;
    if (mar_out) *mar_out = ma;
    
    if (g_history_count < 5) return 0;
    
    int ec = (ea < 0.15f);
    
    if (ec) {
        if (!g_eyes_closed) {
            g_eyes_closed = 1;
            g_closed_frames = 1;
        } else {
            g_closed_frames++;
        }
    } else {
        if (g_eyes_closed) {
            if (g_closed_frames > 2 && g_closed_frames < 15) {
                g_blink_count++;
            }
            g_eyes_closed = 0;
            g_closed_frames = 0;
        }
    }
    
    if (ma > 0.6f) g_yawn_count++;
    
    int d = 0;
    if (g_closed_frames > 30) d = 2;
    else if (g_closed_frames > 15) d = 1;
    
    return d;
}

__attribute__((visibility("default")))
int drowsiness_get_blink_count(void) { 
    return g_blink_count; 
}

__attribute__((visibility("default")))
int drowsiness_get_yawn_count(void) { 
    return g_yawn_count; 
}

__attribute__((visibility("default")))
void drowsiness_cleanup(void) { 
    g_initialized = 0; 
}
