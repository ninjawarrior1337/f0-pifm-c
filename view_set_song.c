#include "view_set_song.h"

typedef enum {
    WorkerEventStop = (1 << 0),
    WorkerEventRx = (1 << 1),
} WorkerEventFlags;

#define WORKER_EVENTS_MASK (WorkerEventStop | WorkerEventRx)

static AppViewState view_select_song_state = {
    .config = &view_set_song_config,
};

static void uart_callback(UartIrqEvent e, uint8_t data, void* ctx) {
    furi_assert(ctx);
    AppView* av = ctx;

    if(e == UartIrqEventRXNE) {
        furi_stream_buffer_send(av->app->song_select_stream_buf, &data, 1, 0);
        furi_thread_flags_set(
            furi_thread_get_id(av->app->song_select_worker_thread), WorkerEventRx);
    }
}

static void handle_rx(SetSongModel* model, const char data) {
    UNUSED(model);
    FURI_LOG_D(TAG, "Worker RX: %c", data);
}

static int32_t set_song_uart_worker(void* ctx) {
    furi_assert(ctx);
    AppView* appview = ctx;

    FURI_LOG_D(TAG, "PiFM Uart Worker Started");

    while(1) {
        uint32_t events =
            furi_thread_flags_wait(WORKER_EVENTS_MASK, FuriFlagWaitAny, FuriWaitForever);
        furi_check((events & FuriFlagError) == 0);

        if(events & WorkerEventStop) {
            break;
        }

        if(events & WorkerEventRx) {
            size_t length = 0;
            do {
                uint8_t data[64];

                length = furi_stream_buffer_receive(
                    appview->app->song_select_stream_buf, data, sizeof(data), 0);

                if(length > 0) {
                    SetSongModel* model = view_get_model(appview->view);
                    for(size_t i = 0; i < length; i++) {
                        handle_rx(model, data[i]);
                    }
                    view_commit_model(appview->view, false);
                }
            } while(length > 0);
            with_view_model(
                appview->view, SetSongModel * m, { UNUSED(m); }, true);
        }
    }

    FURI_LOG_D(TAG, "PiFM Uart Worker Stopped");

    return 0;
}

static void handle_enter(void* ctx) {
    UNUSED(ctx);
    AppView* appview = view_select_song_state.context;
    // view_allocate_model(appview->view, ViewModelTypeLocking, sizeof(SetSongModel));

    appview->app->song_select_stream_buf = furi_stream_buffer_alloc(2048, 1);
    appview->app->song_select_worker_thread =
        furi_thread_alloc_ex("PiFMUartWorker", 1024, set_song_uart_worker, ctx);
    furi_thread_start(appview->app->song_select_worker_thread);

    furi_hal_uart_init(FuriHalUartIdLPUART1, BAUDRATE);
    furi_hal_uart_tx(FuriHalUartIdLPUART1, (uint8_t*)"get songs\n", 10);
    furi_delay_ms(100);

    // UNUSED(uart_callback);
    furi_hal_uart_set_irq_cb(FuriHalUartIdLPUART1, uart_callback, ctx);
}

static void handle_exit(void* ctx) {
    UNUSED(ctx);
    AppView* appview = view_select_song_state.context;
    furi_hal_uart_deinit(FuriHalUartIdLPUART1);
    furi_thread_flags_set(
        furi_thread_get_id(appview->app->song_select_worker_thread), WorkerEventStop);
    furi_thread_join(appview->app->song_select_worker_thread);
    furi_thread_free(appview->app->song_select_worker_thread);
    furi_stream_buffer_free(appview->app->song_select_stream_buf);
}

static uint32_t handle_back(void* ctx) {
    UNUSED(ctx);

    return ViewMain;
}

void song_select_view_alloc(App* app) {
    app->song_select_submenu = submenu_alloc();
    submenu_set_header(app->song_select_submenu, "Set Song");
    View* v = submenu_get_view(app->song_select_submenu);

    view_select_song_state.context = malloc(sizeof(AppView));
    view_select_song_state.context->view = v;
    view_select_song_state.context->app = app;

    view_set_previous_callback(v, view_select_song_state.config->handle_back);
    view_set_enter_callback(v, view_select_song_state.config->handle_enter);
    view_set_exit_callback(v, view_select_song_state.config->handle_exit);
    view_dispatcher_add_view(app->view_dispatcher, view_select_song_state.config->id, v);
}

void song_select_view_free(App* a) {
    view_dispatcher_remove_view(a->view_dispatcher, ViewSetSong);
    free(view_select_song_state.context);
    // view_free_model(submenu_get_view(a->song_select_submenu));
    submenu_free(a->song_select_submenu);
}

ViewConfig view_set_song_config = {
    .id = ViewSetSong,
    .handle_enter = handle_enter,
    .handle_exit = handle_exit,
    .handle_back = handle_back,
};