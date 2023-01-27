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
    AppView* appview = ctx;
    MainViewModel* model = view_get_model(appview->view);

    furi_hal_uart_init(FuriHalUartIdLPUART1, BAUDRATE);
    FuriString* s = furi_string_alloc_printf(
        "set freq %d.%.1d\n",
        model->current_frequency / 1000000,
        (model->current_frequency % 1000000) / 100000);
    furi_hal_uart_tx(FuriHalUartIdLPUART1, (uint8_t*)furi_string_get_cstr(s), furi_string_size(s));
    furi_delay_ms(100);
    furi_hal_uart_deinit(FuriHalUartIdLPUART1);
    furi_string_free(s);

    view_commit_model(appview->view, false);
}

static bool handle_input(InputEvent* event, void* ctx) {
    AppView* appview = ctx;
    MainViewModel* model = view_get_model(appview->view);

    if(event->key == InputKeyUp) {
        if(event->type == InputTypeRepeat) {
            model->current_frequency += 1000000;
        }

        if(event->type == InputTypeShort) {
            model->current_frequency += 100000;
        }
    }

    if(event->key == InputKeyDown) {
        if(event->type == InputTypeRepeat) {
            model->current_frequency -= 1000000;
        }

        if(event->type == InputTypeShort) {
            model->current_frequency -= 100000;
        }
    }

    FURI_LOG_I(TAG, "Set frequency %d", model->current_frequency);
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
        "%d.%.1d",
        model->current_frequency / 1000000,
        (model->current_frequency % 1000000) / 100000);

    canvas_draw_str_aligned(canvas, 64, 32, AlignCenter, AlignCenter, temp_str);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 64, 32 + 16, AlignCenter, AlignCenter, "MHz");
}

ViewConfig view_set_freq_config = {
    .id = ViewSetFreq,
    .handle_back = handle_back,
    .handle_input = handle_input,
    .handle_draw = handle_draw,
    .handle_enter = handle_enter,
    .handle_exit = handle_exit,
};