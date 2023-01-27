#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>

#include <core/common_defines.h>
#include <furi.h>
#include <gui/gui.h>
#include <gui/view.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/submenu.h>
#include <input/input.h>
#include <furi_hal_uart.h>
#include <notification/notification_messages.h>

#include "DG_dynarr.h"

#define TAG "PiFM-F0"
#define BAUDRATE 115200

typedef struct app_t {
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    NotificationApp* notifications;
    Submenu* submenu;

    FuriThread* set_song_worker_thread;
    FuriStreamBuffer* set_song_worker_stream_buffer;
} App;

typedef enum {
    ViewMain,
    ViewSetFreq,
    ViewSetSong,
} ViewID;

typedef enum {
    PiFMStart,
    PiFMStop,
    PiFMSetFreq,
    PiFMSetSong,
} PiFMSubmenuEntries;

typedef struct {
    ViewID id;
    void (*handle_enter)(void*);
    void (*handle_exit)(void*);
    uint32_t (*handle_back)(void*);
    bool (*handle_input)(InputEvent* event, void* ctx);
    void (*handle_draw)(Canvas* const canvas, void* ctx);
} ViewConfig;

typedef struct {
    App* app;
    View* view;
} AppView;

typedef struct {
    ViewConfig* config;
    AppView* context;
} AppViewState;