#include <math.h>

typedef struct { float x, y; } Point2D;

static float g_ear_history[10];
static float g_mar_history[10];
static int g_history_idx = 0;
static int g_history_count = 0;
static int g_blink_count = 0;
static int g_yawn_count = 0;
static int g_closed_frames = 0;
static int g_eyes_closed = 0;
static int g_initialized = 0;

static float dist(Point2D a, Point2D b) {
    float dx = b.x - a.x;
    float dy = b.y - a.y;
    return sqrtf(dx*dx + dy*dy);
}

int drowsiness_init(void) {
    int i;
    g_initialized = 1;
    g_blink_count = 0;
    g_yawn_count = 0;
    g_history_idx = 0;
    g_history_count = 0;
    g_closed_frames = 0;
    g_eyes_closed = 0;
    for (i = 0; i < 10; i++) {
        g_ear_history[i] = 0.0f;
        g_mar_history[i] = 0.0f;
    }
    return 0;
}

int process_5_landmarks(float* lm, float* ear_out, float* mar_out) {
    Point2D el = {lm[0], lm[1]};
    Point2D er = {lm[2], lm[3]};
    Point2D n = {lm[4], lm[5]};
    Point2D ml = {lm[6], lm[7]};
    Point2D mr = {lm[8], lm[9]};
    
    float ed = dist(el, er);
    float nea = (dist(n, el) + dist(n, er)) / 2.0f;
    float ear = nea / (ed + 0.1f);
    
    float mw = dist(ml, mr);
    float mna = (dist(n, ml) + dist(n, mr)) / 2.0f;
    float mar = mna / (mw + 0.1f);
    
    g_ear_history[g_history_idx] = ear;
    g_mar_history[g_history_idx] = mar;
    g_history_idx = (g_history_idx + 1) % 10;
    if (g_history_count < 10) g_history_count++;
    
    float ea = 0.0f, ma = 0.0f;
    int c = (g_history_count < 10) ? g_history_count : 10;
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
    
    int ec = (ea < 0.45f);
    
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
    
    if (ma > 0.75f) g_yawn_count++;
    
    int d = 0;
    if (g_closed_frames > 30) d = 2;
    else if (g_closed_frames > 15) d = 1;
    
    return d;
}

int drowsiness_get_blink_count(void) { return g_blink_count; }
int drowsiness_get_yawn_count(void) { return g_yawn_count; }
void drowsiness_cleanup(void) { g_initialized = 0; }
