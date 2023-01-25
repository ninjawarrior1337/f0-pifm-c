#include "view_main.h"
#include <furi.h>
#include <furi_hal.h>

static uint32_t handle_back(void* ctx) {
    UNUSED(ctx);
    return VIEW_NONE;
}

static void handle_enter(void* ctx) {
    AppView* appview = ctx;
    da_init(appview->app->int_arr);
}

static void handle_exit(void* ctx) {
    AppView* appview = ctx;
    da_free(appview->app->int_arr);
}

static bool handle_input(InputEvent* event, void* ctx) {
    UNUSED(event);
    AppView* appview = ctx;
    da_push(appview->app->int_arr, 0);
    FURI_LOG_I("App", "Input event %d", da_count(appview->app->int_arr));
    return false;
}

static void handle_draw(Canvas* const canvas, void* ctx) {
    AppView* appview = ctx;
    // UNUSED(appview);

    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 64, 32, AlignCenter, AlignCenter, "View Dispatcher example");
    char str[32];
    FURI_LOG_I("App", "Input event %d", da_count(appview->app->int_arr));
    canvas_draw_str_aligned(canvas, 64, 48, AlignCenter, AlignCenter, str);
}

ViewConfig view_main_config = {
    .id = ViewMain,
    .handle_back = handle_back,
    .handle_input = handle_input,
    .handle_draw = handle_draw,
    .handle_enter = handle_enter,
    .handle_exit = handle_exit,
};