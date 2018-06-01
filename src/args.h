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
    int chunk_length;
    char *dim_name;
} chunk_def_t;

typedef struct {
    int count;
    chunk_def_t *dim_chunks;
} chunk_info_t;

typedef struct {
    char *infile;
    int overwrite;
    int debug;
    int nthreads;
    chunk_info_t *scale_info;
} args_t;

// generic/self explanatory functions

args_t *parse_args(int argc, char **argv);

chunk_info_t *parse_chunk_arg(char *chunk_string);

int validate_args(args_t *args);

int how_many_of_char_in_str(const char *chr, char *str);

char *lstrip(char *str);

char *rstrip(char *str);

char *strip(char *str);

#endif //DWCPNC_ARGS_H
