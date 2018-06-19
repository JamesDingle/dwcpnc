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
    float lat;
    float lon;
    int julday;
    float bathymetry;
    float alpha_b;
    float pm_b;
    float zm;
    float b0;
    float h;
    float sigma;
    float rho;
    float cloud;
    float yel_sub;
    float sat_par;
} req_data_t;

typedef struct {
    float* lat;
    float* lon;
    int julday;
    float* bathymetry;
    float* alpha_b;
    float* pm_b;
    float* zm;
    float* b0;
    float* h;
    float* sigma;
    float* rho;
    float* cloud;
    float* yel_sub;
    float* sat_par;
} req_data_chunk_t;

int dwcpn(double zmin, double zmax, int nmx);

float calc_pixel_pp(
        dwcpn_params_t *dwcpn_params,
        req_data_t *inp_data
);

void print_req_data(req_data_t *data);

#endif //DWCPNC_DWCPNC_H
