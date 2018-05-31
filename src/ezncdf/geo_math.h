#ifndef GEO_MATH_H
#define GEO_MATH_H

#include <math.h>
#include <limits.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef M_PI
#define M_PI=3.14159265359
#endif

double degs_to_rads(double degrees);

double rads_to_degs(double radians);

int32_t float_to_int32(float flt);
uint32_t float_to_uint32(float flt);

int16_t float_to_int16(float flt);
uint16_t float_to_uint16(float flt);

int32_t double_to_int32(double dbl);
uint32_t double_to_uint32(double dbl);

int16_t double_to_int16(double dbl);
uint16_t floor_double_to_uint16(double dbl);
uint16_t round_double_to_uint16(double dbl);

float *gen_global_lat_array(int num_pixels);
float *gen_global_lon_array(int num_pixels);

#endif
