//
// Created by jad on 10/03/17.
//

#ifndef C_NC_TOOLS_VBN_H
#define C_NC_TOOLS_VBN_H

#include <stdio.h>
#include <stdlib.h>
#include "array_product.h"

typedef struct {
    int digit_count;
    int *digit_max;
    int *digit_value;
} vbn_t;

vbn_t * init_vbn(int digit_count);

int increment_vbn(vbn_t *vbn);

void print_vbn(vbn_t *vbn);

int vbn_to_base10(vbn_t *vbn);


#endif //C_NC_TOOLS_VBN_H
