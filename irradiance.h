//
// Created by jad on 27/02/18.
//

#ifndef DWCPNC_IRRADIANCE_H
#define DWCPNC_IRRADIANCE_H

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "profile.h"

static const double ALPHA1 = 1.0274;
static const double BETA1 = 0.1324;
static const double ALPHA2 = 1.206;
static const double BETA2 = 0.117;

// Water vapour absorption is set to 2.0
static const double W = 2.0;

// wavelengths for which to calculate irradiance
static const int NUM_WL = 24;

// water vapour absorption coefficient
static const double AV[24] = {0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0.,
                        0.075, 0., 0., 0., 0., 0.016, 0.0125};

// ozone absorption coefficient
static const double AO[24] = {0., 0., 0., 0., 0., 0.003, 0.006, 0.009, 0.014, 0.021, 0.030, 0.040,
                              0.048, 0.063, 0.075, 0.095, 0.120, 0.119, 0.132, 0.120, 0.065,
                              0.060, 0.028, 0.018};

static const double EXTSPIR[24] = {1479.1, 1701.3, 1740.4, 1587.2, 1837.0, 2005.0, 2043.0,
                                   1987.0, 2027.0, 1896.0, 1909.0, 1927.0, 1831.0, 1891.0,
                                   1898.0, 1892.0, 1840.0, 1768.0, 1728.0, 1658.0, 1524.0,
                                   1531.0, 1420.0, 1399.0};

double compute_airmass(double zenith_r, double zenith_d);
double* compute_rayleigh(double airmass, wavelength_array_t *wl_array);
double* compute_aerosol_transmittance(wavelength_array_t *wl_array, double airmass);
double* compute_water_vapour_transmittance(wavelength_array_t *wl_array, double airmass);
double* compute_ozone_transmittance(wavelength_array_t *wl_array, double airmass, double zenith_r);
double compute_tu(double airmass);
double* compute_air_albedo(wavelength_array_t *wl_array, double* ta, double* to, double* tr, double tu, double* tw);
double* compute_direct_irradiance(wavelength_array_t *wl_array, double* ta, double* to, double* tr, double tu, double* tw);
int find_zenith_array_pos(double zenith_d, const double* zen_array, int len);
double* c_array_lookup(int idx);
double* compute_diffuse_irradiance(wavelength_array_t *wl_array, double zenith_d, double zenith_r, const double* direct, const double* ro_s, const double* ta, const double* to, const double* tr, double tu, const double* tw);
double* interpolate_irradiances(wavelength_array_t *wl_array, const double* irradiance);
#endif //DWCPNC_IRRADIANCE_H
