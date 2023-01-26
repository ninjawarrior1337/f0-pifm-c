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

    model->current_frequency = 88 * 1000000;

    view_commit_model(appview->view, true);
}

static void handle_exit(void* ctx) {
    UNUSED(ctx);
}

static bool handle_input(InputEvent* event, void* ctx) {
    AppView* appview = ctx;
    MainViewModel* model = view_get_model(appview->view);

    if(event->key == InputKeyUp) {
        if(event->type == InputTypeLong) {
            model->current_frequency += 1000000;
        }

        if(event->type == InputTypeShort) {
            model->current_frequency += 500000;
        }
    }

    if(event->key == InputKeyDown) {
        if(event->type == InputTypeLong) {
            model->current_frequency -= 1000000;
        }

        if(event->type == InputTypeShort) {
            model->current_frequency -= 500000;
        }
    }

    FURI_LOG_I("App", "Set frequency %d", model->current_frequency);
    view_commit_model(appview->view, true);

    return false;
}

static void handle_draw(Canvas* const canvas, void* ctx) {
    MainViewModel* model = ctx;
    UNUSED(model);

    canvas_clear(canvas);
    canvas_set_font(canvas, FontBigNumbers);
    char temp_str[32];

    snprintf(
        temp_str,
        32,
        "%d.%.2d MHz",
        model->current_frequency / 1000000,
        model->current_frequency % 1000000);

    canvas_draw_str_aligned(canvas, 64, 32, AlignCenter, AlignCenter, temp_str);
}

ViewConfig view_set_freq_config = {
    .id = ViewSetFreq,
    .handle_back = handle_back,
    .handle_input = handle_input,
    .handle_draw = handle_draw,
    .handle_enter = handle_enter,
    .handle_exit = handle_exit,
};