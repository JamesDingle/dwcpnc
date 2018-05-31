//
// Created by jad on 31/05/18.
//

#ifndef DWCPNC_ARGS_H
#define DWCPNC_ARGS_H

#include <ctype.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef struct {
    int scale_factor;
    char *dim_name;
} dim_scale_t;

typedef struct {
    int count;
    dim_scale_t *scaled_dims;
} scale_info_t;

typedef struct {
    char *infile;
    int overwrite;
    int debug;
    scale_info_t *scale_info;
} args_t;

// generic/self explanatory functions

args_t *parse_args(int argc, char **argv);

scale_info_t *parse_scale_arg(char *scale_string);

int validate_args(args_t *args);

int how_many_of_char_in_str(const char *chr, char *str);

char *lstrip(char *str);

char *rstrip(char *str);

char *strip(char *str);

#endif //DWCPNC_ARGS_H
