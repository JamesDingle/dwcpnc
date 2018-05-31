#include "dwcpn/dwcpnc.h"
#include "args.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>



int main(int argc, char **argv) {

    float start_time = (float)clock() / CLOCKS_PER_SEC;

    args_t *args;
    args = parse_args(argc, argv);

    if (!validate_args(args)) {
        return EXIT_FAILURE;
    }



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

    tpos->lat = 35.9583343498;
    tpos->lon = -5.7083331719;
    tpos->julday = 15;
    tpos->bathymetry = -360.2880859375;
    tpos->alpha_b = 0.0585016608;
    tpos->pm_b = 4.3047113419;
    tpos->zm = 4.3246043165;
    tpos->b0 = 0.1989055441;
    tpos->rho = 0.7597410072;
    tpos->h = 67.2604851113;
    tpos->sigma = 42.6616115108;
    tpos->cloud = 0.0;
    tpos->yel_sub = 0.3;
    tpos->sat_par = 15.2781600952;
//
//    tpos->lat = 35.5416676714;
//    tpos->lon = -6.1249998268;
//    tpos->julday = 15;
//    tpos->bathymetry = -61.4626770020;
//    tpos->alpha_b = 0.0412193872;
//    tpos->pm_b = 3.0254180431;
//    tpos->zm = 3.0448300654;
//    tpos->b0 = 0.7429472239;
//    tpos->rho = 0.7610457516;
//    tpos->h = 261.2296164441;
//    tpos->sigma = 44.0431633987;
//    tpos->cloud = 0.0;
//    tpos->yel_sub = 0.3;
//    tpos->sat_par = 15.6864595413;


    calc_pixel_pp(dwcpn_params, tpos);

    float end_time = (float)clock() / CLOCKS_PER_SEC;

    printf("Time elapsed: %f\n", end_time-start_time);

    free(tpos);
    free(dwcpn_params);
}

