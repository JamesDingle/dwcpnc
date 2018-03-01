//
// Created by jad on 27/02/18.
//

#include "irradiance.h"
#include "profile.h"


double compute_airmass(double zenith_r, double zenith_d) {
    double airmass;
    airmass = pow(cos(zenith_r) + 0.15 * pow(93.885 - zenith_d, -1.253), -1.0);
    if (airmass < 1.0) {
        airmass = 1.0;
    }

    return airmass;
}

double* compute_rayleigh(double airmass, wavelength_array_t *wl_array) {

    double* tr = (double*)malloc(sizeof(double) * wl_array->count);

    int i;
    double wld;
    for (i = 0; i < wl_array->count; ++i){
        wld = wl_array->values[i] / 1000.0;
        tr[i] = exp(-airmass / (pow(wld, 4) * (115.6406 - 1.335 / pow(wld, 2))));
    }

    return tr;
}

double* compute_aerosol_transmittance(wavelength_array_t *wl_array, double airmass) {
    double* ta = (double*)malloc(sizeof(double) * wl_array->count);

    int i;
    double wld;
    for (i = 0; i < wl_array->count; ++i){
        wld = wl_array->values[i] / 1000.0;
        if (i < 10) {
            ta[i] = exp(-BETA1 * pow(wld, ALPHA1) * airmass);
        } else {
            ta[i] = exp(-BETA2 * pow(wld, ALPHA2) * airmass);
        }
    }

    return ta;
}

double* compute_water_vapour_transmittance(wavelength_array_t *wl_array, double airmass){

    double* tw = (double*)malloc(sizeof(double) * wl_array->count);

    int i;
    for (i = 0; i < wl_array->count; ++i) {
        tw[i] = exp(-0.3285 * AV[i] * (W + (1.42 - W) * 0.5) * airmass / pow(1.0 + 20.07 * AV[i] * airmass, 0.45));
    }
    return tw;
}

double* compute_ozone_transmittance(wavelength_array_t *wl_array, double airmass, double zenith_r){
    double* to = (double*)malloc(sizeof(double) * wl_array->count);

    double em0;

    int i;
    for (i = 0; i < wl_array->count; ++i) {
        em0 = 35.0 / pow((1224.0 * pow(cos(zenith_r), 2.0) + 1.0), 0.5);
        to[i] = exp(-AO[i] * 0.03 * em0);
//        to[i] = exp(-AO[i] * 0.344 * em0); // to be uncommented once equivalency tests are done
    }
    return to;
}

double compute_tu(double airmass){
    return exp(-1.41 * 0.3 * airmass / pow(1 + 118.93 * 0.3 * airmass, 0.45));
}

double* compute_air_albedo(wavelength_array_t *wl_array, double* ta, double* to, double* tr, double tu, double* tw){
    double* ro_s = (double*)malloc(sizeof(double) * wl_array->count);

    int i;
    for (i = 0; i < wl_array->count; ++i){
        ro_s[i] = to[i] * tw[i] * (ta[i] * (1.0 - tr[i]) * 0.5 + tr[i] * (1 - ta[i]) * 0.22 * 0.928);
    }
    ro_s[wl_array->count - 1] = ro_s[wl_array->count - 1] * tu;
    return ro_s;
}

double* compute_direct_irradiance(wavelength_array_t *wl_array, double* ta, double* to, double* tr, double tu, double* tw){
    double* direct = (double*)malloc(sizeof(double) * wl_array->count);

    // note EXTSPIR is Extra-Terrestrial spectral irradiance

    int i;
    for (i = 0; i < wl_array->count; ++i) {
        direct[i] = EXTSPIR[i] * tr[i] * ta[i] * tw[i] * to[i];
    }
    direct[wl_array->count - 1] = direct[wl_array->count - 1] * tu;

    return direct;
}

double* compute_diffuse_irradiance(wavelength_array_t *wl_array, double zenith_d, double zenith_r, double* ta, double* to, double* tr, double tu, double* tw){
    double* c = c_array_lookup(zenith_d);

    int i;
    for (i = 0; i < 7; ++i) {

    }
}

double* c_array_lookup(double zenith_d, int* idx) {
    double* c;

    // this is icky and hardcoded but since its such a small LUT I have just made an if statement
    if (zenith_d < 0.0) {
        c = (double[5]){1.11, 1.04, 1.15, 1.12, 1.32};
    } else if (zenith_d < 37.0) {
        c = (double[5]){1.13, 1.05, 1.00, 0.96, 1.12};
    } else if (zenith_d < 48.19) {
        c = (double[5]){1.18, 1.09, 1.00, 0.96, 1.07};
    } else if (zenith_d < 60.0) {
        c = (double[5]){1.24, 1.11, 0.99, 0.94, 1.02};
    } else if (zenith_d < 70.0) {
        c = (double[5]){1.46, 1.24, 1.06, 0.99, 1.10};
    } else if (zenith_d < 75.0) {
        c = (double[5]){1.70, 1.34, 1.07, 0.96, 0.90};
    } else if (zenith_d <= 80.0) {
        c = (double[5]){2.61, 1.72, 1.22, 1.04, 0.80};
    }

    return c;
}