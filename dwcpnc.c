//
// Created by jad on 20/03/17.
//

#include "dwcpnc.h"


float calc_pixel_pp(dwcpn_params_t *dwcpn_params, req_data_t *inp_data) {
//    depth_profile_t *cprof = gen_chl_prof(
//            dwcpn_params->zmin,
//            dwcpn_params->zmax,
//            dwcpn_params->num_depths,
//            dwcpn_params->depth_step,
//            12, // longhurst province ID
//            inp_data->b0 // surface chl-a
//    );

    depth_profile_t *cprof = gen_chl_prof_no_lookup(
            dwcpn_params->zmin,
            dwcpn_params->zmax,
            dwcpn_params->num_depths,
            dwcpn_params->depth_step,
            inp_data->sigma,
            inp_data->h,
            inp_data->zm,
            inp_data->b0 // surface chl-a
    );

    // populate arrays holding a lot of the information we are going to need
    double sunrise = compute_sunrise(inp_data->julday, inp_data->lat);
    time_profile_t* tprof = gen_time_prof(sunrise, 12);
    wavelength_array_t* wl_array = gen_wavelength_array(400.0, 5.0, 61);
    double* zen_rad_arr = zenith_array_from_times(tprof, inp_data->julday, inp_data->lat);

    // Because the transmittance coefficients are only provided at 24 wavelengths
    // we will calculate for those only and then interpolate up to the full set of
    // 61 wavelengths afterwards
    wavelength_array_t* wl_trans_arr = (wavelength_array_t*)malloc(sizeof(wavelength_array_t));
    wl_trans_arr->count = 24;
    wl_trans_arr->values = (double[24]){400., 410., 420., 430., 440., 450., 460., 470.,
                                        480., 490., 500., 510., 520., 530., 540., 550.,
                                        570., 593., 610., 630., 656., 667.6, 690., 710.};

    double solar_correction = thekaekara_correction(inp_data->julday);

    double* surface_irradiance = (double*)calloc((size_t)tprof->count, sizeof(double));
    double* par_surface_irradiance = (double*)malloc(sizeof(double) * tprof->count);

    // albedo calc variables
    double albedo, cc, idir1, flux, idif1, dir_div, dif_div;

    // reflection calc variables
    double zenith_w, ref;

    // spectral surface irradiance variables
    double wl, wl_coeff;
    double* total = (double*)malloc(sizeof(double) * wl_array->count);

    // calculating surface irradiance from par variables
    double iom = inp_data->sat_par * M_PI / (2 * tprof->day_length);

    // adjustment of diffuse and direct component
    double* adjustment = (double*)malloc(sizeof(double) * tprof->count);

    double* i_zero = (double*)malloc(sizeof(double) * tprof->count);

    // for the sam penguin call we need to precalculate the bw/bbr/ay arrays
    // these won't change within the time/depth loops so we can do them here safely
    double* bw = calc_bw(wl_array);
    double* bbr = calc_bbr(wl_array);
    double* ay = calc_ay(wl_array);

    // loop variables
    int z, l, t; // l = lambda, z = depth, t = time

    // final container variables for time integration
    double* pp_t, *euph_t;
    double pp_day = 0, z_phot_day = 0, i_0_day = 0;


    pp_t = (double*)calloc((size_t)tprof->count, sizeof(double));
    euph_t = (double*)malloc(sizeof(double) * tprof->count);

    // start to loop over each time step now
    for (t = 0; t < tprof->count; ++t){
        double zen_rad = zen_rad_arr[t];
        double zen_deg = zen_rad * (180.0 / M_PI);

        // clamp the zenith angle to max 80
        if (zen_deg > 80.0) {
            zen_deg = 79.0;
            zen_rad = zen_deg * (M_PI / 180.0);
        }


        // compute direct/diffuse irradiance
        double airmass = compute_airmass(zen_rad, zen_deg);

        double* ta = compute_aerosol_transmittance(wl_trans_arr, airmass);
        double* tr = compute_rayleigh(airmass, wl_trans_arr);
        double* tw = compute_water_vapour_transmittance(wl_trans_arr, airmass);
        double* to = compute_ozone_transmittance(wl_trans_arr, airmass, zen_rad);
        double tu = compute_tu(airmass);

        double* ro_s = compute_air_albedo(wl_trans_arr, ta, to, tr, tu, tw);

        double* direct;
        direct = compute_direct_irradiance(wl_trans_arr, ta, to, tr, tu, tw);
        double* diffuse;
        diffuse = compute_diffuse_irradiance(wl_trans_arr, zen_deg, zen_rad, direct, ro_s, ta, to, tr, tu, tw);

        double* direct_itpl = interpolate_irradiances(wl_trans_arr, direct);
        double* diffuse_itpl = interpolate_irradiances(wl_trans_arr, diffuse);

        double i_direct = 0.0;
        double i_diffuse = 0.0;

        for (l = 0; l < wl_array->count; ++l) {
            // apply fractional correction to diffuse and direct components of irradiance
            // the max correction value is 1353.0, so this converts it to as though we were applying a percentage correction
            direct_itpl[l] = direct_itpl[l] * solar_correction / 1353.0;
            diffuse_itpl[l] = diffuse_itpl[l] * solar_correction / 1353.0;

            // add this value to the integrated direct/diffuse components
            i_direct = i_direct + (direct_itpl[l] * cos(zen_rad));
            i_diffuse = i_diffuse + diffuse_itpl[l];
        }

        surface_irradiance[t] = surface_irradiance[t] + i_direct + i_diffuse;

        // cloud effect calculations
        albedo = 0.28 / (1 + 6.43 * cos(zen_rad));
        cc = inp_data->cloud / 100.0;
        idir1 = i_direct * (1 - cc);
        flux = ((1 - 0.5 * cc) * (0.82 - albedo * (1 - cc)) * cos(zen_rad)) / ((0.82 - albedo) * cos(zen_rad));
        idif1 = surface_irradiance[t] * flux - idir1;
        dir_div = idir1 / i_direct;
        dif_div = idif1 / i_diffuse;

        for (l = 0; l < wl_array->count; ++l) {
            direct_itpl[l] = direct_itpl[l] + dir_div;
            diffuse_itpl[l] = diffuse_itpl[l] + dif_div;
        }

        // calculate reflection and convert watts/micronto einsteins/hr/nm
        zenith_w = asin(sin(zen_rad) / 1.333);
        ref = 0.5 * pow(sin(zen_rad - zenith_w), 2) / pow(sin(zen_rad + zenith_w), 2);
        ref = ref + 0.5 * pow(tan(zen_rad - zenith_w), 2) / pow(tan(zen_rad + zenith_w), 2);

        // recompute surface irradiance across spectrum
        surface_irradiance[t] = 0.0;

        for (l = 0; l < wl_array->count; ++l) {
            wl = wl_array->values[l];
            wl_coeff = wl * 36.0 / (19.87 * 6.022 * pow(10,7));
            direct_itpl[l] = direct_itpl[l] * wl_coeff * cos(zen_rad);
            diffuse_itpl[l] = diffuse_itpl[l] * wl_coeff;

            surface_irradiance[t] = surface_irradiance[t] + direct_itpl[l] + diffuse_itpl[l];

            direct_itpl[l] = direct_itpl[l] * (1 - ref);
            diffuse_itpl[l] = diffuse_itpl[l] * 0.945;

            total[l] = direct_itpl[l] + diffuse_itpl[l];
        }

        // compute surface irradiance from total daily surface irradiance (e.g. satellite par)
        par_surface_irradiance[t] = iom * sin(M_PI * (tprof->values[t] - tprof->sunrise) / tprof->day_length);
        surface_irradiance[t] = surface_irradiance[t] * 5.0;

        // Adjustment to the difuse and direct component: from use of measured total daily surface irradiance (
        // e.g. satellite PAR) to compute the surface irradiance at all time. SSP
        adjustment[t] = par_surface_irradiance[t] / surface_irradiance[t];

        // compute the adjusted irradiance surface value
        i_zero[t] = 0.0;
        for (l = 0; l < wl_array->count; ++l) {
            direct_itpl[l] = direct_itpl[l] * adjustment[t];
            diffuse_itpl[l] = diffuse_itpl[l] * adjustment[t];
            total[l] = direct_itpl[l] + diffuse_itpl[l];

            i_zero[t] = i_zero[t] + total[l] * 5.0;
        }

        // call the samanthised penguin module
        pp_result_t* pp_result = compute_pp_along_profile(
                cprof,
                wl_array,
                zen_rad,
                direct_itpl,
                diffuse_itpl,
                bw,
                ay,
                bbr,
                inp_data->alpha_b,
                inp_data->pm_b,
                inp_data->yel_sub
        );

        if (pp_result->success > 0) {

            // clamp euphotic depth to bathymetry if it's higher
            if (fabs(pp_result->euphotic_depth) > fabs(inp_data->bathymetry)) {
                pp_result->euphotic_depth_index = 499;
                pp_result->euphotic_depth = inp_data->bathymetry;
            }

            euph_t[t] = pp_result->euphotic_depth;

//            printf("PP Calculated: %.2f\n", pp_result->pp_profile[0]);
            for (z = 0; z < pp_result->euphotic_depth_index - 1; ++z) {
                pp_t[t] = pp_t[t] + cprof->step * (pp_result->pp_profile[z] + pp_result->pp_profile[z+1]) / 2.0;
            }

            if (pp_result->euphotic_depth_index == 0) {
                pp_result->euphotic_depth_index = 1;
            }

//            double pp_test = pp_result->pp_profile[pp_result->euphotic_depth_index] * (euph_t[t] - (pp_result->euphotic_depth_index - 1) * cprof->step);
//            if (!isnan(pp_test)) {
//                pp_t[t] = pp_t[t] + pp_test;
//            } else {
//                pp_t[t] = 0;
//                printf("Nan found\n");
//            }
            pp_t[t] = pp_t[t] + pp_result->pp_profile[pp_result->euphotic_depth_index] * (euph_t[t] - (pp_result->euphotic_depth_index - 1) * cprof->step);
        }



        free(ta);
        free(tr);
        free(tw);
        free(to);
        free(ro_s);
        free(direct);
        free(diffuse);
        free(direct_itpl);
        free(diffuse_itpl);

        free(pp_result->pp_profile);
        free(pp_result->par_profile);

        free(pp_result);

    }

    pp_day = pp_t[0] * tprof->delta_prestart / 2.0;
    z_phot_day = euph_t[0] * i_zero[0] * tprof->delta_prestart / 2.0;
    i_0_day = i_zero[0] * tprof->delta_prestart / 2.0;

    //integrate over time
    for (t = 0; t < tprof->count - 1; ++t) {
        pp_day = pp_day + (pp_t[t] + pp_t[t+1]) * tprof->delta_t / 2.0;
        z_phot_day = z_phot_day + (euph_t[t] * i_zero[t] + euph_t[t+1] * i_zero[t+1] * tprof->delta_t / 2.0);
        i_0_day = i_0_day + (i_zero[t] + i_zero[t+1]) * tprof->delta_t / 2.0;
    }

    // mutliply by two because we have only integrated over half of the day
    pp_day = pp_day * 2.0;
    z_phot_day = z_phot_day * 2.0;
    i_0_day = i_0_day * 2.0;

    // normalise photic depth to irradiance
    z_phot_day = z_phot_day / i_0_day;

    printf("PP: %f photic depth: %f\n", pp_day, z_phot_day);

    free(pp_t);
    free(euph_t);

    free(tprof->values);
    free(tprof);

    free(cprof->values);
    free(cprof->depths);
    free(cprof);

    free(wl_array->values);
    free(wl_array);

    free(zen_rad_arr);
    free(wl_trans_arr);
    free(surface_irradiance);
    free(par_surface_irradiance);
    free(total);
    free(adjustment);
    free(i_zero);
    free(bw);
    free(bbr);
    free(ay);




}






