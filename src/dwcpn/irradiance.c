//
// Created by jad on 27/02/18.
//

#include "irradiance.h"
#include "profile.h"


double compute_airmass(double zenith_r, double zenith_d) {
    double airmass;
//    airmass = pow(cos(zenith_r) + 0.15 * pow(93.885 - zenith_d, -1.253), -1.0);

    airmass = 1.0 / (cos(zenith_r) + 0.15 * (pow(93.885 - zenith_d, -1.253)));

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
            ta[i] = exp(-BETA1 * pow(wld, -ALPHA1) * airmass);
        } else {
            ta[i] = exp(-BETA2 * pow(wld, -ALPHA2) * airmass);
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
//        to[i] = exp(-AO[i] * 0.03 * em0);
        to[i] = exp(-AO[i] * 0.344 * em0); // to be uncommented once equivalency tests are done
    }
    return to;
}

double compute_tu(double airmass){
    return exp(-1.41 * 0.15 * airmass / pow(1 + 118.3 * 0.15 * airmass, 0.45));
}

double* compute_air_albedo(wavelength_array_t *wl_array, double* ta, double* to, double* tr, double tu, double* tw){
    double* ro_s = (double*)malloc(sizeof(double) * wl_array->count);

    int i;
    for (i = 0; i < wl_array->count; ++i){
        ro_s[i] = to[i] * tw[i] * (ta[i] * (1.0 - tr[i]) * 0.5 + tr[i] * (1 - ta[i]) * 0.22 * 0.928);
    }
    ro_s[23] = ro_s[23] * tu;
    return ro_s;
}

double* compute_direct_irradiance(wavelength_array_t *wl_array, double* ta, double* to, double* tr, double tu, double* tw){
    double* direct = (double*)malloc(sizeof(double) * wl_array->count);
    // note EXTSPIR is Extra-Terrestrial spectral irradiance

    int i;
    for (i = 0; i < wl_array->count; ++i) {
        direct[i] = EXTSPIR[i] * tr[i] * ta[i] * tw[i] * to[i];
    }
    direct[23] = direct[23] * tu;

    return direct;
}

double* compute_diffuse_irradiance(wavelength_array_t *wl_array, double zenith_d, double zenith_r, const double* direct, const double* ro_s, const double* ta, const double* to, const double* tr, double tu, const double* tw){
    double* diffuse = (double*)malloc(sizeof(double) * wl_array->count);

    double* zen_lut = (double[7]){0., 37., 48.19, 60., 70., 75., 80.};

    int c_idx = find_zenith_array_pos(zenith_d, zen_lut, 7);

    if (zenith_d >= 80.) {
        c_idx = 6;
    }

    double* c = c_array_lookup(c_idx);

    double* cm1 = c_array_lookup(c_idx-1);

    double fraction;
    fraction = (zenith_d - zen_lut[c_idx - 1]) / (zen_lut[c_idx] - zen_lut[c_idx - 1]);

    double* c1 = (double*)malloc(sizeof(double) * 5); // compute along the 5 wavelengths in the LUT

    int i;
    for (i = 0; i < 5; ++i) {
        c1[i] = (c[i] - cm1[i]) * fraction + cm1[i];
    }

    double* c2 = (double*)malloc(sizeof(double) * wl_array->count);
    double cinc;
    int l_in, l_out, l;

    c2[0] = c1[0];
    l = 0;

    for (l_out = 1; l_out < 4; ++l_out){
        cinc = (c1[l_out-1] - c1[l_out]) / 5.0;
        for (l_in = 0; l_in < 5; ++l_in) {
            l = l + 1;
            c2[l] = c2[l - 1] - cinc;
        }
    }

    double cdif = c1[4] - c1[3];
    double wldif = 160.0;
    double wlinc;

    for (l_out = 16; l_out < 24; ++l_out) {
        wlinc = (wl_array->values[l_out] - wl_array->values[l_out-1]) / wldif;
        cinc = cdif * wlinc;
        l = l + 1;
        c2[l] = c2[l - 1] + cinc;
    }

    double xx, r, a, g;

    for (l = 0; l < 24; ++l) {
        xx= EXTSPIR[l] * cos(zenith_r) * to[l] * tw[l];
        r = xx * ta[l] * (1.0 - tr[l]) * 0.5;
        a = xx * tr[l] * (1.0 - ta[l]) * 0.928 * 0.82;

        if (l == 22) {
            r = r * tu;
            a = a * tu;
        }

        g = (direct[l] * cos(zenith_r) + (r + a) * c2[l]) * ro_s[l] * 0.05 /
                            (1.0 - 0.05 * ro_s[l]);

        diffuse[l] = (r + a) * c2[l] + g;
    }
    free(c1); c1=NULL;
    free(c2); c2=NULL;
    free(c); c=NULL;
    free(cm1);cm1=NULL;
    return diffuse;
}

int find_zenith_array_pos(double zenith_d, const double* zen_array, int num) {
    int i;
    for (i = 0; i < num; ++i){
        if (zenith_d < zen_array[i]) {
            return i;
        }
    }
    return num;
}

double* c_array_lookup(int index) {

    // TESTING JUST IN CASE THIS IS THE WRONG WAY AROUND
//    double* c_array = (double[35]){
//            1.11, 1.04, 1.15, 1.12, 1.32,
//            1.13, 1.05, 1.00, 0.96, 1.12,
//            1.18, 1.09, 1.00, 0.96, 1.07,
//            1.24, 1.11, 0.99, 0.94, 1.02,
//            1.46, 1.24, 1.06, 0.99, 1.10,
//            1.70, 1.34, 1.07, 0.96, 0.90,
//            2.61, 1.72, 1.22, 1.04, 0.80
//    };

    double* c_array = (double[35]){
            1.11, 1.13, 1.18, 1.24, 1.46, 1.70, 2.61,
            1.04, 1.05, 1.09, 1.11, 1.24, 1.34, 1.72,
            1.15, 1.00, 1.00, 0.99, 1.06, 1.07, 1.22,
            1.12, 0.96, 0.96, 0.94, 0.99, 0.96, 1.04,
            1.32, 1.12, 1.07, 1.02, 1.10, 0.90, 0.80
    };

    double* ret_array = (double*)malloc(sizeof(double) * 5);

    int i;
    for (i = 0; i < 5; ++i) {
        ret_array[i] = c_array[(i * 7) + index];
    }

    return ret_array;

}

// TODO: (James) this is a stupid function and assumes that there will always be 61 even wavelengths from 400nm
// write an updated version of this to accept two wavelength arrays and interpolate one to the other
double* interpolate_irradiances(wavelength_array_t *wl_array, const double* irradiance) {

    double* output = (double*) malloc(sizeof(double) * 61);

    output[0] = irradiance[0]; // first element is correct

    int l = 1;
    int l1 = 1;
    int length = 405;

    double fraction;
    double dif;

    while (l < 61) {
        if (length >= wl_array->values[l1+1]) {
            l1 = l1 + 1;
        } else {
            fraction = (length - wl_array->values[l1]) / (wl_array->values[l1+1] - wl_array->values[l1]);
            dif = irradiance[l1 + 1]  - irradiance[l1];
            output[l] = irradiance[l1] + dif * fraction;
            l = l + 1;
            length = length + 5;
        }
    }

//    printf("Interp: [");
//    for (l = 0; l < 61; ++l) {
//        printf("%.2f,", output[l]);
//    }
//    printf("]\n");

    return output;

}