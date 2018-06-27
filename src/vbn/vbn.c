//
// Created by jad on 10/03/17.
//

#include "vbn.h"

vbn_t *init_vbn(int digit_count) {
    vbn_t *vbn;

    vbn = (vbn_t*)malloc(sizeof(vbn_t));
    vbn->digit_count = digit_count;
    vbn->digit_max = (int*)calloc((size_t)digit_count, sizeof(int));
    vbn->digit_value = (int*)calloc((size_t)digit_count, sizeof(int));

    return vbn;
}

int increment_vbn(vbn_t *vbn) {
    int i,j;

    for (i = vbn->digit_count; i > 0; --i) {
        if (vbn->digit_value[i] < vbn->digit_max[i]) {
            ++vbn->digit_value[i];
            return 1;
        } else if (vbn->digit_value[i-1] < vbn->digit_max[i-1]) {
            ++vbn->digit_value[i-1];
            for (j = i; j < vbn->digit_count; ++j) {
                vbn->digit_value[j] = 0;
            }
            return 1;
        }
    }
    return 0; // vbn is already at its max value
}

void print_vbn(vbn_t *vbn) {
    int i;

    for (i = 0; i < vbn->digit_count; ++i) {
        printf("%d ", vbn->digit_value[i]);
    }

    printf("\n");
}


int vbn_to_base10(vbn_t *vbn) {
    int total = 0;

    int i;
    for (i = 0; i < vbn->digit_count; ++i) {
        total += (vbn->digit_max[i] * vbn->digit_value[i]);
    }

    return total;
}

//int vbn_to_1d_index(vbn_t *vbn, ca_array_t ca_array) {
//    int index = 0;
//    int d2r = 0; // digits left to the right
//
//    int i,j;
//    int tmp = 0;
//    for (i = 0; i < vbn->digit_count; ++i) {
//
//        tmp = vbn->digit_value[i];
//
//
//        if (vbn->digit_count - i > 0) {
//            printf("(%d ", tmp);
//            for (j = i+1; j < vbn->digit_count; ++j) {
////                printf("%d: %d \n",j,vbn->digit_max[j]);
//                printf("* %d", vbn->digit_max[j]);
//                tmp *= vbn->digit_max[j];
//           }
//            printf(") ");
//            index += tmp;
//        } else {
//            printf(" + %d", tmp);
//            index += tmp;
//        }
//        if (i < vbn->digit_count -1 ) {
//            printf(" + ");
//        }
//    }
//
//    printf("\n");
//
//    return index;
//}