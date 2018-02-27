//
// Created by jad on 20/03/17.
//

#ifndef DWCPNC_DWCPNC_H
#define DWCPNC_DWCPNC_H

#include "profile.h"
#include "solar.h"
#include "penguin.h"

typedef struct {
    int count;
    double *winter_vals;
    double *spring_vals;
    double *summer_vals;
    double *autumn_vals;
} dwcpn_params;

int dwcpn(double zmin, double zmax, int nmx);

#endif //DWCPNC_DWCPNC_H
