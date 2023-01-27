#include "view_set_song.h"

static void uart_callback(UartIrqEvent e, uint8_t data, void* ctx) {
    UNUSED(ctx);
    UNUSED(e);
    // AppView* appview = ctx;
    // SetSongModel* model = view_get_model(appview->view);
    // UNUSED(model);

    FURI_LOG_I(TAG, "%c", data);
    // view_commit_model(appview->view, false);
}

typedef enum {
    WorkerEventStop = (1 << 0),
    WorkerEventRx = (1 << 1),
} WorkerEventFlags;

#define WORKER_EVENTS_MASK (WorkerEventStop | WorkerEventRx)

static int32_t set_song_uart_worker(void* ctx) {
    furi_assert(ctx);
    AppView* appview = ctx;
    UNUSED(appview);

    while(1) {
        uint32_t events =
            furi_thread_flags_wait(WORKER_EVENTS_MASK, FuriFlagWaitAny, FuriWaitForever);
        furi_check((events & FuriFlagError) == 0);

        if(events & WorkerEventStop) {
            break;
        }
    }

    return 0;
}

static void handle_enter(void* ctx) {
    AppView* appview = ctx;
    view_allocate_model(appview->view, ViewModelTypeLocking, sizeof(SetSongModel));

    appview->app->set_song_worker_thread =
        furi_thread_alloc_ex("PiFMUartWorker", 1024, set_song_uart_worker, ctx);
    furi_thread_start(appview->app->set_song_worker_thread);

    furi_hal_uart_init(FuriHalUartIdLPUART1, BAUDRATE);
    furi_hal_uart_tx(FuriHalUartIdLPUART1, (uint8_t*)"get songs\n", 10);
    furi_delay_ms(100);

    // UNUSED(uart_callback);
    furi_hal_uart_set_irq_cb(FuriHalUartIdLPUART1, uart_callback, ctx);
}

static void handle_exit(void* ctx) {
    AppView* appview = ctx;
    furi_hal_uart_deinit(FuriHalUartIdLPUART1);
    furi_thread_flags_set(
        furi_thread_get_id(appview->app->set_song_worker_thread), WorkerEventStop);
    furi_thread_join(appview->app->set_song_worker_thread);
    furi_thread_free(appview->app->set_song_worker_thread);
}

static uint32_t handle_back(void* ctx) {
    UNUSED(ctx);

    return ViewMain;
}

static void handle_draw(Canvas* canvas, void* model) {
    UNUSED(model);
    canvas_draw_str_aligned(canvas, 64, 32, AlignCenter, AlignCenter, "Hello World");
}

static bool handle_input(InputEvent* e, void* ctx) {
    UNUSED(e);
    UNUSED(ctx);

    return false;
}

ViewConfig view_set_song_config = {
    .id = ViewSetSong,
    .handle_enter = handle_enter,
    .handle_input = handle_input,
    .handle_draw = handle_draw,
    .handle_exit = handle_exit,
    .handle_back = handle_back,
};