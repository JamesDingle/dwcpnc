//
// Created by jad on 20/03/17.
//

#ifndef DWCPNC_DWCPNC_H
#define DWCPNC_DWCPNC_H

#include "profile.h"
#include "irradiance.h"
#include "solar.h"
#include "penguin.h"

typedef struct {
    int num_depths;
    double depth_step;
    double zmin;
    double zmax;
} dwcpn_params_t;

typedef struct {
    double lat;
    double lon;
    int julday;
    double alpha_b;
    double pm_b;
    double zm;
    double b0;
    double h;
    double sigma;
    double cloud;
    double yel_sub;
    double sat_par;
} req_data_t;

int dwcpn(double zmin, double zmax, int nmx);

float calc_pixel_pp(
        dwcpn_params_t *dwcpn_params,
        req_data_t *inp_data
);

#endif //DWCPNC_DWCPNC_H
