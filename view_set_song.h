#pragma once

#include "app.h"

DA_TYPEDEF(FuriString*, FuriStringArray)

typedef struct {
    FuriStringArray songs;
    FuriString* current_string;
    FuriMutex* mutex;
} SelectSongModel;

extern ViewConfig view_set_song_config;

void song_select_view_alloc(App* a);
void song_select_view_free(App* a);