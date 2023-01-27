#pragma once

#include "app.h"

DA_TYPEDEF(FuriString*, FuriStringArray)

typedef struct {
    FuriStringArray songs;
    FuriString* current_string;
} SetSongModel;

extern ViewConfig view_set_song_config;