//
// Created by jad on 26/02/18.
//

#include "profile.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

depth_profile_t* gen_chl_prof(
        double zmin,
        double zmax,
        int count,
        double step,
        int province,
        double surface_chl
) {
    depth_profile_t *prof;
    prof = (depth_profile_t*)malloc(sizeof(depth_profile_t));

    prof->zmin = zmin;
    prof->zmax = zmax;

    if (count != 0 && step != 0) {
        // if user has provided both step and count, check they are compatible
        double step_check = (zmax - zmin) / count;
        if (step_check != step) {
            return NULL;
        } else {
            prof->step = step;
            prof->count = count;
        }
    } else if (count != 0) {
        prof->count = count;
        prof->step = (zmax - zmin) / count;
    } else if (step != 0) {
        prof->step = step;
        prof->count = (int)((zmax - zmin) / step);
    } else {
        return NULL;
    }

    prof->depths = (double*)malloc(sizeof(double) * (size_t)prof->count);
    prof->values = (double*)malloc(sizeof(double) * (size_t)prof->count);

    int i;
    for (i = 0; i < prof->count; ++i) {
        prof->depths[i] = prof->zmin + (i * prof->step);
    }

    // read in the parameters
    double sigma = lookup_sigma(2, province);
    double zm = lookup_zm(2, province);
    double rho = lookup_rho(2, province);

    double B0 = B0_from_surface_chl(surface_chl, rho, zm, sigma);
    double h = compute_h(sigma, rho, B0);

    // calculate chlorophyll depth profile
    // shifted gaussian

    // gaussian height
    double gh;
    gh = h / ( sigma * (sqrt(2 * M_PI)) );

    double itm; // intermediate
    double res;
    for (i = 0; i < prof->count; ++i) {
        prof->values[i] = 0.0;

        itm = - 0.5 * pow((prof->depths[i] - zm) / sigma, 2);
        if (fabs(itm) > 675.) {
            res = 0.0;
        } else {
            res = gh * exp(itm);
        }
        prof->values[i] = res + B0;
    }

    return prof;
}

depth_profile_t* gen_chl_prof_no_lookup(
        double zmin,
        double zmax,
        int count,
        double step,
        double sigma,
        double h,
        double zm,
        double B0
) {
    depth_profile_t *prof;
    prof = (depth_profile_t*)malloc(sizeof(depth_profile_t));

    prof->zmin = zmin;
    prof->zmax = zmax;

    if (count != 0 && step != 0) {
        // if user has provided both step and count, check they are compatible
        double step_check = (zmax - zmin) / count;
        if (step_check != step) {
            return NULL;
        } else {
            prof->step = step;
            prof->count = count;
        }
    } else if (count != 0) {
        prof->count = count;
        prof->step = (zmax - zmin) / count;
    } else if (step != 0) {
        prof->step = step;
        prof->count = (int)((zmax - zmin) / step);
    } else {
        return NULL;
    }

    prof->depths = (double*)malloc(sizeof(double) * (size_t)prof->count);
    prof->values = (double*)malloc(sizeof(double) * (size_t)prof->count);

    int i;
    for (i = 0; i < prof->count; ++i) {
        prof->depths[i] = prof->zmin + (i * prof->step);
    }

//    double B0 = B0_from_surface_chl(surface_chl, rho, zm, sigma);
//    double h = compute_h(sigma, rho, B0);

    // calculate chlorophyll depth profile
    // shifted gaussian

    // gaussian height
    double gh;
    gh = h / ( sigma * (sqrt(2 * M_PI)) );

    double itm; // intermediate
    double res;
    for (i = 0; i < prof->count; ++i) {
        prof->values[i] = 0.0;

        itm = - 0.5 * pow((prof->depths[i] - zm) / sigma, 2);
        if (fabs(itm) > 675.) {
            res = 0.0;
        } else {
            res = gh * exp(itm);
        }
        prof->values[i] = res + B0;
    }

    return prof;
}

time_profile_t* gen_time_prof(double sunrise, int count) {
    time_profile_t *prof;
    prof = (time_profile_t*)malloc(sizeof(time_profile_t));
    prof->sunrise = sunrise;
    prof->count = count;
    prof->values = (double*)malloc(sizeof(double) * (size_t)prof->count);

    prof->delta_t = (12.0 - sunrise) / count;

    int i;
    for (i = 0; i < count; ++i) {
        prof->values[i] = sunrise + prof->delta_t * (i);
    }

    prof->day_length =  2 * (12.0 - sunrise);
    prof->delta_prestart = 1 + prof->values[0] - sunrise;

    return prof;
}

wavelength_array_t* gen_wavelength_array(double start, double step, int count) {
    wavelength_array_t *prof;
    prof = (wavelength_array_t*)malloc(sizeof(wavelength_array_t));
    prof->start = start;
    prof->step = step;
    prof->count = count;
    prof->values = (double*)malloc(sizeof(double) * (size_t)prof->count);

    int i;
    for (i = 0; i < prof->count; ++i){
        prof->values[i] = start + i * step;
    }

    return prof;
}

double B0_from_surface_chl(double chl, double rho, double zm, double sigma) {
    return chl / (1 + (rho / (1 - rho)) * exp(-pow(zm, 2) / (2 * pow(sigma, 2))) );
}

double compute_h(double sigma, double rho, double B0) {
    return sigma * (rho / (1 - rho)) * B0 * sqrt(2 * M_PI);
}



double lookup_sigma(int season, int province) {
    switch(season) {
        case WINTER:
            return sigma_winter[province];
        case SPRING:
            return sigma_spring[province];
        case SUMMER:
            return sigma_summer[province];
        case AUTUMN:
            return sigma_autumn[province];
        default:
            return -999.;
    }
}

double lookup_zm(int season, int province) {
    switch(season) {
        case WINTER:
            return zm_winter[province];
        case SPRING:
            return zm_spring[province];
        case SUMMER:
            return zm_summer[province];
        case AUTUMN:
            return zm_autumn[province];
        default:
            return -999.;
    }
}

double lookup_rho(int season, int province) {
    switch(season) {
        case WINTER:
            return rho_winter[province];
        case SPRING:
            return rho_spring[province];
        case SUMMER:
            return rho_summer[province];
        case AUTUMN:
            return rho_autumn[province];
        default:
            return -999.;
    }
}