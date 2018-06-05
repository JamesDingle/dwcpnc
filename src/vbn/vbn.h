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
} variable_base_num;

variable_base_num * init_vbn(int digit_count);

int increment_vbn(variable_base_num *vbn);

void print_vbn(variable_base_num *vbn);

int vbn_to_base10(variable_base_num *vbn);


#endif //C_NC_TOOLS_VBN_H
