#include "geo_math.h"

double degs_to_rads(double degrees) {
	return degrees * (M_PI / 180.0f);
}

double rads_to_degs(double radians) {
	return radians * (180.0f / M_PI);
}

int32_t float_to_int32(float flt) {
	assert(flt >= INT_MIN-0.5);
	assert(flt <= INT_MAX+0.5);
	if (flt >= 0)
		return (int32_t) (flt+0.5);
	return (int32_t) (flt-0.5);
}

uint32_t float_to_uint32(float flt) {
	assert(flt <= UINT32_MAX+0.5);
	assert(flt >= 0);
    return (uint32_t) (flt+0.5);
}

int16_t float_to_int16(float flt) {
	assert(flt >= INT16_MIN-0.5);
	assert(flt <= INT16_MAX+0.5);
	if (flt >= 0)
		return (int16_t) (flt+0.5);
	return (int16_t) (flt-0.5);
}

uint16_t float_to_uint16(float flt) {
	assert(flt <= UINT16_MAX+0.5);
    return (uint16_t) (flt+0.5);
}

int32_t double_to_int32(double dbl) {
	assert(dbl >= INT_MIN-0.5);
	assert(dbl <= INT_MAX+0.5);
	if (dbl >= 0)
		return (int32_t) (dbl+0.5);
	return (int32_t) (dbl-0.5);
}

uint32_t double_to_uint32(double dbl) {
	assert(dbl <= UINT32_MAX+0.5);
	assert(dbl >= 0);
    return (uint32_t) (dbl+0.5);
}

int16_t double_to_int16(double dbl) {
	assert(dbl >= INT16_MIN-0.5);
	assert(dbl <= INT16_MAX+0.5);
	if (dbl >= 0)
		return (int16_t) (dbl+0.5);
	return (int16_t) (dbl-0.5);
}

uint16_t floor_double_to_uint16(double dbl) {
	assert(dbl <= UINT16_MAX+0.5);
	assert(dbl >= 0);
	dbl = floor(dbl);
	return (uint16_t) (dbl+0.5);
}

uint16_t round_double_to_uint16(double dbl) {
	assert(dbl <= UINT16_MAX+0.5);
	assert(dbl >= 0);
	return (uint16_t) (dbl+0.5);
}

float *gen_global_lat_array(int num_pixels) {
   int i;
   long double px_diff, px_start;
   px_diff = 180.0 / num_pixels;
   px_start = -90. + (px_diff / 2.);

   float *lats;
   //lats = float[num_pixels];
   lats = (float *)malloc(sizeof(float) * num_pixels);

   for (i = 0; i < num_pixels; ++i) {
      lats[i] = px_start + (px_diff * i);
   }
   return lats;
}

float *gen_global_lon_array(int num_pixels) {
   int i;
   long double px_diff, px_start;
   px_diff = 360.0 / num_pixels;
   px_start = -180. + (px_diff / 2.);

   float *lons;
   lons = (float *)malloc(sizeof(float) * num_pixels);

   for (i = 0; i < num_pixels; ++i) {
      lons[i] = px_start + (px_diff * i);
   }
   return lons;
}
//TODO(James): Add haversine distance function here

//TODO(James): add index arithmetic here (2/3/4d -> 1d index)
