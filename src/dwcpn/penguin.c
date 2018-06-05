//
// Created by jad on 26/02/18.
//

#include "penguin.h"
#include "profile.h"

pp_result_t* compute_pp_along_profile(
        depth_profile_t *chl_profile,
        wavelength_array_t *wl_array,
        double zenith_r,
        double *direct,
        double *diffuse,
        double *bw,
        double *ay,
        double *bbr,
        double pi_alpha,
        double pi_pmb,
        double yelsub
) {

    pp_result_t* pp_result = (pp_result_t*)malloc(sizeof(pp_result_t));
    pp_result->pp_profile = (double*)malloc(sizeof(double) * chl_profile->count);
    pp_result->par_profile = (double*)malloc(sizeof(double) * chl_profile->count);

    double* i_0 = (double*)malloc(sizeof(double) * wl_array->count);
    double* mu_d = (double*)malloc(sizeof(double) * wl_array->count);
    double* i_z = (double*)malloc(sizeof(double) * wl_array->count);
    double* k = (double*)malloc(sizeof(double) * wl_array->count);
    double* ac = (double*)malloc(sizeof(double) * wl_array->count);

    double zenith_w = asin(sin(zenith_r) / 1.333);

    int l;
    for (l = 0; l < wl_array->count; ++l) {
        i_0[l] = direct[l] + diffuse[l];
        mu_d[l] = (direct[l] * cos(zenith_w) + diffuse[l] * 0.831000) / i_0[l];
        i_z[l] = i_0[l];
        k[l] = 0;
    }

    // variables used within calculation loop
    double chl, almean, ac440, power, ay440, bc660, bbtilda, alpha_b, x, wl, a, bc, bb;

    int z;
    for (z = 0; z < chl_profile->count; ++z) {
        pp_result->par_profile[z] = 0.0;
        chl = chl_profile->values[z];

        almean = 0.0;
        for (l = 0; l < wl_array->count; ++l) {
            ac[l] =PC1[l] * (1.0 - exp(-RATE[l] * chl)) + PC2[l] * chl;
            almean = almean + ac[l];
        }
        almean = almean / wl_array->count;
        ac440 = ac[8];

        power = -log10(chl);
        ay440 = yelsub * ac440;

        bc660 = 0.407 * pow(chl, 0.795);
        bbtilda = (0.78 + 0.42 * power) * 0.01;

        if (bbtilda < 0.0005) {
            bbtilda = 0.0005;
        } else if (bbtilda > 0.01) {
            bbtilda = 0.01;
        }

        alpha_b = 0.0;

        for (l = 0; l < wl_array->count; ++l) {
            wl = wl_array->values[l];
            a = AW[l] + ac[l] + ay440 * ay[l] + 2.0 * bbr[l];
            bc = bc660 * pow(660.0 / wl, power);

            if (bc < 0.0) {
                bc = 0.0;
            }

            bb = bc * bbtilda + bw[l] * 0.50;

            k[l] = (a + bb) / mu_d[l];

            pp_result->par_profile[z] = pp_result->par_profile[z] + i_z[l] * 5.0;

            x = pi_alpha * ac[l] * 6022.0 / (2.77 * 36.0 * almean);

            alpha_b = alpha_b + x * 5.0 * i_z[l] / mu_d[l];
            i_z[l] = i_z[l] * exp(-k[l] * chl_profile->step);
        }

        pp_result->pp_profile[z] = (alpha_b / sqrt(1.0 + pow(alpha_b / pi_pmb, 2))) * chl;

//        if (isnan(pp_result->pp_profile[z])) {
//            printf("break here\n");
//        }

        if (z > 0) {
            if (pp_result->par_profile[z] < (0.01 * pp_result->par_profile[0])) {
                pp_result->euphotic_depth_index = z - 1;
                pp_result->euphotic_depth = chl_profile->depths[z-1] + chl_profile->step * log(100 * pp_result->par_profile[z-1] / pp_result->par_profile[0]) /\
                                                           log(pp_result->par_profile[z-1] / pp_result->par_profile[z]);
                pp_result->success = 1;

                goto doublebreak;
            }
        }
    }

    pp_result->success = 0;

    doublebreak:
    free(i_0);
    free(mu_d);
    free(i_z);
    free(k);
    free(ac);
    return pp_result;
}

double* calc_bw(wavelength_array_t* wl_array) {
    double* bw = (double*)malloc(sizeof(double) * wl_array->count);

    int i;
    for (i = 0; i < wl_array->count; ++i) {
        bw[i] = BW500 * pow(wl_array->values[i] / 500., -4.3);
    }
    return bw;
}
double* calc_bbr(wavelength_array_t* wl_array) {
    double* bbr = (double*)malloc(sizeof(double) * wl_array->count);

    int i;
    for (i = 0; i < wl_array->count; ++i) {
        bbr[i] = 0.5 * br488 * pow(wl_array->values[i] / 488., -5.3);
    }
    return bbr;
}
double* calc_ay(wavelength_array_t* wl_array) {
    double* ay = (double*)malloc(sizeof(double) * wl_array->count);

    int i;
    for (i = 0; i < wl_array->count; ++i) {
        ay[i] = exp(-0.014 * (wl_array->values[i] - 440.0));
    }
    return ay;
}

