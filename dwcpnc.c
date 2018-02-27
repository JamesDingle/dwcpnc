//
// Created by jad on 20/03/17.
//

#include "dwcpnc.h"
#include "profile.h"
#include <stdio.h>
#include <math.h>


int dwcpn(double zmin, double zmax, int nmx) {
    depth_profile_t *prof = gen_chl_prof(zmin, zmax, 0, 0.5, 1, 0.3);
    int i;
    for (i = 0; i < prof->count; ++i) {
        printf("[%d] Chlor-a at depth %f = %f\n", i, prof->depths[i], prof->values[i]);
    }
//    printf("B0: %f\n", B0_from_surface_chl(0.3, 10, 10, 10));
    return 0;
}

float calc_pixel_pp(dwcpn_params_t *dwcpn_params, req_data_t *inp_data) {
    depth_profile_t *cprof = gen_chl_prof(
            dwcpn_params->zmin,
            dwcpn_params->zmax,
            dwcpn_params->num_depths,
            dwcpn_params->depth_step,
            1, // longhurst province ID
            0.3 // surface chl-a
    );

    // populate arrays holding a lot of the information we are going to need
    double sunrise = compute_sunrise(inp_data->julday, inp_data->lat);
    time_profile_t* tprof = gen_time_prof(sunrise, 12);
    wavelength_array_t* wl_array = gen_wavelength_array(400.0, 5.0, 61);
    double* zen_rad_arr = zenith_array_from_times(tprof, inp_data->julday, inp_data->lat);

    // start to loop over each time step now
    int t_idx;
    for (t_idx = 0; t_idx < tprof->count; ++t_idx){
        double zen_rad = zen_rad_arr[t_idx];
        double zen_deg = zen_rad * (180.0 / M_PI);

        // clamp the zenith angle to max 80
        if (zen_deg > 80.0) {
            zen_deg = 79.0;
            zen_rad = zen_deg * (M_PI / 180.0);
        }

        double airmass = compute_airmass(zen_rad, zen_deg);



    }

//    int i;
//    for (i = 0; i < cprof->count; ++i) {
//        printf("[%d] Chlor-a at depth %f = %f\n", i, cprof->depths[i], cprof->values[i]);
//    }
//
//    for (i = 0; i < tprof->count; ++i) {
//        printf("[%d] time at step %d = %02d:%02d:%02d Zenith angle: %02f\n", i, i, (int)tprof->values[i], (int)(60 * fmod(tprof->values[i],1)), (int)(60 * fmod(60 * tprof->values[i],1)), zen_rad_arr[i]);
//    }
//
//    for (i = 0; i < wl_array->count; ++i) {
//        printf("[%d] wl: %f\n", i, wl_array->values[i]);
//    }

}





