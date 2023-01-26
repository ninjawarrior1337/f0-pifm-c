#pragma once

#include "app.h"

#include "DG_dynarr.h"

DA_TYPEDEF(int, IntArrType);

typedef struct {
    IntArrType int_arr;
} MainViewModel;

extern ViewConfig view_set_freq_config;