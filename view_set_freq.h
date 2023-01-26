#pragma once

#include "app.h"

#include "DG_dynarr.h"

DA_TYPEDEF(int, IntArrType);

typedef struct {
    int current_frequency;
} MainViewModel;

extern ViewConfig view_set_freq_config;