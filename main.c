#include "dwcpnc.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>



int main() {

    float start_time = (float)clock() / CLOCKS_PER_SEC;

    // initialisation of dwcpn parameters here, these will not change throughout the entire
    // execution
    dwcpn_params_t *dwcpn_params;
    dwcpn_params = (dwcpn_params_t*)malloc(sizeof(dwcpn_params_t));

    dwcpn_params->zmin = 0.0;
    dwcpn_params->zmax = 250.0;
    dwcpn_params->num_depths = 0;
    dwcpn_params->depth_step = 0.5; // we will let the profile generator auto calculate this


    req_data_t *tpos;
    tpos = (req_data_t*)malloc(sizeof(req_data_t));

    tpos->lat = 34.95833206;
    tpos->lon = -10.04166698;
    tpos->julday = 15;
    tpos->alpha_b = 0.03224407136440277;
    tpos->pm_b = 2.13877010345459;
    tpos->zm = 60.28186714542189;
    tpos->b0 = 0.1298753627732041;
    tpos->h = 46.99779480845467;
    tpos->sigma = 31.983231597845602;
    tpos->cloud = 0.0;
    tpos->yel_sub = 0.3;
    tpos->sat_par = 18.708280563354492;

    calc_pixel_pp(dwcpn_params, tpos);

    float end_time = (float)clock() / CLOCKS_PER_SEC;

    printf("Time elapsed: %f", end_time-start_time);

}

