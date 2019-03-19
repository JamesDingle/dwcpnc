#ifndef ARRAY_PRODUCT_H
#define ARRAY_PRODUCT_H

#include <math.h>

#include <stdio.h>
#include <stdlib.h>

#include "vbn.h"

typedef struct {
    int start;
    int end;
} chunk_t;

typedef struct {
    int count;
    chunk_t **chunks;
} chunk_array_t;

typedef struct {
    int ndims;
    chunk_array_t **chunk_arrays;
} ca_array_t;

typedef struct {
    int dim_count;
    chunk_t **chunks;
} slice_t;

typedef struct {
    int count;
    slice_t **slices;
} slice_array_t;

chunk_array_t * gen_chunks(int total_length, int chunk_length);
int add_chunk_to_slice(chunk_t *chunk, slice_t *slice);

int nd_to_1d(int* indices, int* extents, int count);

//void print_ca_array_from_vbn(ca_array_t* ca_array, vbn_t *vbn);

void print_chunk(chunk_t chunk);

#endif
