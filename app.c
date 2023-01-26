#define DG_DYNARR_IMPLEMENTATION

#include "app.h"
#include "view_set_freq.h"

static AppViewState views[] = {
    {
        .config = &view_set_freq_config,
    },
};

static const unsigned views_count = COUNT_OF(views);

static void app_views_free(App* app) {
    for(unsigned i = 0; i < views_count; i++) {
        furi_assert(views[i].context);
        view_dispatcher_remove_view(app->view_dispatcher, views[i].config->id);
        view_free(views[i].context->view);
        view_free_model(views[i].context->view);
        free(views[i].context);
    }
}

static void app_views_alloc(App* app) {
    for(unsigned i = 0; i < views_count; i++) {
        views[i].context = malloc(sizeof(AppView));
        views[i].context->view = view_alloc();
        views[i].context->app = app;
        view_set_context(views[i].context->view, views[i].context);
        view_set_enter_callback(views[i].context->view, views[i].config->handle_enter);
        view_set_exit_callback(views[i].context->view, views[i].config->handle_exit);
        view_set_draw_callback(views[i].context->view, views[i].config->handle_draw);
        view_set_input_callback(views[i].context->view, views[i].config->handle_input);
        view_dispatcher_add_view(
            app->view_dispatcher, views[i].config->id, views[i].context->view);
        view_set_previous_callback(views[i].context->view, views[i].config->handle_back);
    }
}

static void app_free(App* app) {
    furi_assert(app);
    app_views_free(app);
    submenu_free(app->submenu);
    furi_record_close(RECORD_NOTIFICATION);
    view_dispatcher_free(app->view_dispatcher);
    furi_record_close(RECORD_GUI);
    free(app);
}

static void submenu_select_callback(void* ctx, uint32_t index) {
    PiFMSubmenuEntries e = index;
    App* app = ctx;
    UNUSED(app);

    FURI_LOG_I(TAG, "selected %d", e);
    switch(e) {
    case PiFMStart:
    case PiFMStop:
        furi_hal_uart_init(FuriHalUartIdLPUART1, BAUDRATE);
        FuriString* cmd = furi_string_alloc();
        if(e == PiFMStart) {
            furi_string_cat_str(cmd, "start");
        } else if(e == PiFMStop) {
            furi_string_cat_str(cmd, "stop");
        }
        furi_hal_uart_tx(
            FuriHalUartIdLPUART1, (uint8_t*)furi_string_get_cstr(cmd), furi_string_size(cmd));
        furi_delay_ms(100);
        furi_hal_uart_deinit(FuriHalUartIdLPUART1);
        furi_string_free(cmd);
        break;
    case PiFMSetFreq:
        view_dispatcher_switch_to_view(app->view_dispatcher, ViewSetFreq);
        break;
    case PiFMSetSong:
        break;
    }
}

static uint32_t submenu_prev_callback(void* ctx) {
    UNUSED(ctx);
    return VIEW_NONE;
}

static App* app_alloc() {
    App* app = malloc(sizeof(App));

    app->gui = furi_record_open(RECORD_GUI);
    app->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_enable_queue(app->view_dispatcher);
    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);
    app->notifications = furi_record_open(RECORD_NOTIFICATION);

    app_views_alloc(app);

    app->submenu = submenu_alloc();
    submenu_set_header(app->submenu, "PiFM");
    submenu_add_item(app->submenu, "Start", PiFMStart, submenu_select_callback, app);
    submenu_add_item(app->submenu, "Stop", PiFMStop, submenu_select_callback, app);
    submenu_add_item(app->submenu, "Set Frequency", PiFMSetFreq, submenu_select_callback, app);
    submenu_add_item(app->submenu, "Set Song", PiFMSetSong, submenu_select_callback, app);
    view_set_previous_callback(submenu_get_view(app->submenu), submenu_prev_callback);

    view_dispatcher_add_view(app->view_dispatcher, ViewMain, submenu_get_view(app->submenu));
    view_dispatcher_switch_to_view(app->view_dispatcher, ViewMain);

    return app;
}

int32_t app_entry_point(void) {
    App* app = app_alloc();
    view_dispatcher_run(app->view_dispatcher);
    app_free(app);
    return 0;
}