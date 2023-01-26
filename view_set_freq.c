#include "view_set_freq.h"
#include <furi.h>
#include <furi_hal.h>

static uint32_t handle_back(void* ctx) {
    UNUSED(ctx);

    return ViewMain;
}

static void handle_enter(void* ctx) {
    AppView* appview = ctx;
    view_allocate_model(appview->view, ViewModelTypeLocking, sizeof(MainViewModel));

    MainViewModel* model = view_get_model(appview->view);
    da_init(model->int_arr);

    view_commit_model(appview->view, true);
}

static void handle_exit(void* ctx) {
    AppView* appview = ctx;
    MainViewModel* model = view_get_model(appview->view);
    da_free(model->int_arr);
    view_commit_model(appview->view, false);
}

static bool handle_input(InputEvent* event, void* ctx) {
    AppView* appview = ctx;
    UNUSED(event);
    MainViewModel* model = view_get_model(appview->view);

    if(model != NULL) {
        da_push(model->int_arr, 0);
        FURI_LOG_I("App", "Input event %d", da_count(model->int_arr));
        view_commit_model(appview->view, true);
    }

    return false;
}

static void handle_draw(Canvas* const canvas, void* ctx) {
    MainViewModel* model = ctx;

    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 64, 32, AlignCenter, AlignCenter, "View Dispatcher example");
    char str[32];
    snprintf(str, 32, "Input events: %d", da_count(model->int_arr));
    canvas_draw_str_aligned(canvas, 64, 48, AlignCenter, AlignCenter, str);
}

ViewConfig view_set_freq_config = {
    .id = ViewSetFreq,
    .handle_back = handle_back,
    .handle_input = handle_input,
    .handle_draw = handle_draw,
    .handle_enter = handle_enter,
    .handle_exit = handle_exit,
};