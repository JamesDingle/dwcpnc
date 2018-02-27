//
// Created by jad on 20/03/17.
//

#include "dwcpnc.h"
#include <stdio.h>

int dwcpn(double zmin, double zmax, int nmx) {
    depth_profile_t *prof = gen_chl_prof(zmin, zmax, 0, 0.5, 1, 0.3);
    int i;
    for (i = 0; i < prof->count; ++i) {
        printf("[%d] Chlor-a at depth %f = %f\n", i, prof->depths[i], prof->values[i]);
    }
//    printf("B0: %f\n", B0_from_surface_chl(0.3, 10, 10, 10));
    return 0;
}







