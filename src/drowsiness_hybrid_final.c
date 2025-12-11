#include <math.h>
#include <stddef.h>

static const float EAR_CLOSED_THR     = 0.18f;
static const int   BLINK_MIN_FRAMES   = 2;
static const int   BLINK_MAX_FRAMES   = 8;
static const int   MICROSLEEP_FRAMES  = 45;

static const float MAR_YAWN_THR       = 0.75f;
static const int   YAWN_MIN_FRAMES    = 10;

static const float EAR_ALPHA          = 0.3f;
static const float MAR_ALPHA          = 0.3f;

// Estado global
static int   g_total_blinks        = 0;
static int   g_total_yawns         = 0;

static int   g_eyes_closed         = 0;
static int   g_closed_frames       = 0;

static int   g_mouth_open          = 0;
static int   g_mouth_open_frames   = 0;

static float g_ear_smooth          = 0.0f;
static float g_mar_smooth          = 0.0f;

// Utilidades
static float dist2d(float x1, float y1, float x2, float y2)
{
    float dx = x2 - x1;
    float dy = y2 - y1;
    return sqrtf(dx * dx + dy * dy);
}

static void get_xy(const float *lm, int idx, float *x, float *y)
{
    *x = lm[2 * idx + 0];
    *y = lm[2 * idx + 1];
}

// EAR con 4 puntos (esquina, esquina, arriba, abajo)
static float eye_ear(const float *lm, int i_left, int i_right, int i_up, int i_down)
{
    float xl, yl, xr, yr, xu, yu, xd, yd;
    get_xy(lm, i_left,  &xl, &yl);
    get_xy(lm, i_right, &xr, &yr);
    get_xy(lm, i_up,    &xu, &yu);
    get_xy(lm, i_down,  &xd, &yd);

    float hor = dist2d(xl, yl, xr, yr);
    float ver = dist2d(xu, yu, xd, yd);

    if (hor <= 1e-6f)
        return 0.0f;
    return ver / hor;
}

// MAR con 4 puntos (boca)
static float mouth_mar(const float *lm, int i_left, int i_right, int i_up, int i_down)
{
    float xl, yl, xr, yr, xu, yu, xd, yd;
    get_xy(lm, i_left,  &xl, &yl);
    get_xy(lm, i_right, &xr, &yr);
    get_xy(lm, i_up,    &xu, &yu);
    get_xy(lm, i_down,  &xd, &yd);

    float hor = dist2d(xl, yl, xr, yr);
    float ver = dist2d(xu, yu, xd, yd);

    if (hor <= 1e-6f)
        return 0.0f;
    return ver / hor;
}

// Init
int drowsiness_init(void)
{
    g_total_blinks      = 0;
    g_total_yawns       = 0;
    g_eyes_closed       = 0;
    g_closed_frames     = 0;
    g_mouth_open        = 0;
    g_mouth_open_frames = 0;
    g_ear_smooth        = 0.0f;
    g_mar_smooth        = 0.0f;
    return 0;
}

// Procesamiento principal
int process_hybrid(const float *mediapipe_lm,
                   const float *retinaface_lm,
                   float *ear_out,
                   float *mar_out)
{
    (void)retinaface_lm;

    if (!mediapipe_lm || !ear_out || !mar_out)
        return 0;

    // Indices de MediaPipe: ojo y boca
    int L_OUT  = 33;
    int L_IN   = 133;
    int L_UP   = 159;
    int L_DOWN = 145;

    int R_OUT  = 263;
    int R_IN   = 362;
    int R_UP   = 386;
    int R_DOWN = 374;

    int M_LEFT  = 78;
    int M_RIGHT = 308;
    int M_UP    = 13;
    int M_DOWN  = 14;

    float ear_left  = eye_ear(mediapipe_lm, L_OUT, L_IN, L_UP, L_DOWN);
    float ear_right = eye_ear(mediapipe_lm, R_OUT, R_IN, R_UP, R_DOWN);
    float ear_raw   = 0.5f * (ear_left + ear_right);

    float mar_raw   = mouth_mar(mediapipe_lm, M_LEFT, M_RIGHT, M_UP, M_DOWN);

    if (g_ear_smooth == 0.0f)
        g_ear_smooth = ear_raw;
    else
        g_ear_smooth = EAR_ALPHA * ear_raw + (1.0f - EAR_ALPHA) * g_ear_smooth;

    if (g_mar_smooth == 0.0f)
        g_mar_smooth = mar_raw;
    else
        g_mar_smooth = MAR_ALPHA * mar_raw + (1.0f - MAR_ALPHA) * g_mar_smooth;

    *ear_out = g_ear_smooth;
    *mar_out = g_mar_smooth;

    int danger = 0;
    int microsleep = 0;

    // Parpadeo y microsueno
    if (g_ear_smooth < EAR_CLOSED_THR) {
        if (!g_eyes_closed) {
            g_eyes_closed = 1;
            g_closed_frames = 1;
        } else {
            g_closed_frames++;
        }
    } else {
        if (g_eyes_closed) {
            if (g_closed_frames >= BLINK_MIN_FRAMES &&
                g_closed_frames <= BLINK_MAX_FRAMES) {
                g_total_blinks++;
            }
            if (g_closed_frames >= MICROSLEEP_FRAMES) {
                microsleep = 1;
            }
            g_eyes_closed = 0;
            g_closed_frames = 0;
        }
    }

    // Bostezos
    if (g_mar_smooth > MAR_YAWN_THR) {
        if (!g_mouth_open) {
            g_mouth_open = 1;
            g_mouth_open_frames = 1;
        } else {
            g_mouth_open_frames++;
        }
    } else {
        if (g_mouth_open) {
            if (g_mouth_open_frames >= YAWN_MIN_FRAMES) {
                g_total_yawns++;
            }
            g_mouth_open = 0;
            g_mouth_open_frames = 0;
        }
    }

    // Nivel de peligro
    if (microsleep) {
        danger = 2;
    } else if (g_eyes_closed || g_mouth_open) {
        danger = 1;
    } else {
        danger = 0;
    }

    return danger;
}

// Contadores
int drowsiness_get_blink_count(void)
{
    return g_total_blinks;
}

int drowsiness_get_yawn_count(void)
{
    if (g_total_yawns <= 0)
        return 0;
    return g_total_yawns - 1;
}

// Limpieza
void drowsiness_cleanup(void)
{
}
