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

static void on_view_select_song_submenu_select(void* context, uint32_t index) {
    UNUSED(context);

    char tmp[32];
    snprintf(tmp, sizeof(tmp), "set song %ld\n", index);

    furi_hal_uart_tx(FuriHalUartIdLPUART1, (uint8_t*)tmp, strlen(tmp) - 1);
    furi_delay_ms(100);

    view_dispatcher_switch_to_view(view_select_song_state.context->app->view_dispatcher, ViewMain);
}

static void populate_submenu(SelectSongModel* model) {
    AppView* appview = view_select_song_state.context;
    submenu_reset(appview->app->song_select_submenu);
    for(size_t i = 0; i < da_count(model->songs); i++) {
        FuriString* song = da_get(model->songs, i);
        submenu_add_item(
            appview->app->song_select_submenu,
            furi_string_get_cstr(song),
            i,
            on_view_select_song_submenu_select,
            NULL);
    }
}

static void handle_rx(SelectSongModel* model, const char data) {
    FURI_LOG_D(TAG, "Worker RX: %c", data);
    switch(data) {
    case '\n':
        da_push(model->songs, model->current_string);
        model->current_string = furi_string_alloc();
        populate_submenu(model);
        break;
    case ',':
        da_push(model->songs, model->current_string);
        model->current_string = furi_string_alloc();
        break;
    default:
        furi_string_cat_printf(model->current_string, "%c", data);
        break;
    }
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
                    SelectSongModel* model = view_select_song_state.raw_model;
                    FuriStatus status = furi_mutex_acquire(model->mutex, FuriWaitForever);
                    furi_assert(status == FuriStatusOk);
                    for(size_t i = 0; i < length; i++) {
                        handle_rx(model, data[i]);
                    }
                    furi_mutex_release(model->mutex);
                }
            } while(length > 0);
        }
    }

    FURI_LOG_D(TAG, "PiFM Uart Worker Stopped");

    return 0;
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
    submenu_free(a->song_select_submenu);
}

static SelectSongModel* select_song_model_alloc() {
    SelectSongModel* m = malloc(sizeof(SelectSongModel));
    m->mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    m->current_string = furi_string_alloc();
    da_init(m->songs);
    return m;
}

static void select_song_model_free(SelectSongModel* m) {
    furi_string_free(m->current_string);
    for(size_t i = 0; i < da_count(m->songs); i++) {
        furi_string_free(m->songs.p[i]);
    }
    da_free(m->songs);
    furi_mutex_free(m->mutex);
    free(m);
}

static void handle_enter(void* ctx) {
    UNUSED(ctx);
    AppView* appview = view_select_song_state.context;

    view_select_song_state.raw_model = select_song_model_alloc();

    appview->app->song_select_stream_buf = furi_stream_buffer_alloc(2048, 1);
    appview->app->song_select_worker_thread =
        furi_thread_alloc_ex("PiFMUartWorker", 1024, set_song_uart_worker, appview);
    furi_thread_start(appview->app->song_select_worker_thread);

    furi_hal_uart_set_irq_cb(FuriHalUartIdLPUART1, uart_callback, appview);
    furi_hal_uart_tx(FuriHalUartIdLPUART1, (uint8_t*)"get songs\n", 10);
    furi_delay_ms(100);
}

static void handle_exit(void* ctx) {
    UNUSED(ctx);
    AppView* appview = view_select_song_state.context;

    furi_hal_uart_set_irq_cb(FuriHalUartIdLPUART1, NULL, NULL);

    furi_thread_flags_set(
        furi_thread_get_id(appview->app->song_select_worker_thread), WorkerEventStop);
    furi_thread_join(appview->app->song_select_worker_thread);
    furi_thread_free(appview->app->song_select_worker_thread);
    furi_stream_buffer_free(appview->app->song_select_stream_buf);

    select_song_model_free(view_select_song_state.raw_model);
}

static uint32_t handle_back(void* ctx) {
    UNUSED(ctx);

    return ViewMain;
}

ViewConfig view_set_song_config = {
    .id = ViewSetSong,
    .handle_enter = handle_enter,
    .handle_exit = handle_exit,
    .handle_back = handle_back,
};