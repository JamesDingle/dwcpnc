//
// Created by jad on 26/02/18.
//

#include "solar.h"
#include "profile.h"
#include <math.h>


double compute_sunrise(int jday, double lat) {
    double day = (double)jday; // convert to double for the maths
    double theta, delta, phi, phidel;
    theta = (2.0 * M_PI * day) / 365.0;
    delta = 0.006918 - 0.399912 * cos(theta) +
            0.006758 * cos(2.0 * theta) + 0.000907 * sin(2.0 * theta) -
            0.002697 * cos(3.0 * theta) + 0.001480 * sin(3.0 * theta);

    phi = lat * (M_PI / 180.);
    phidel = -tan(phi) * tan(delta);

    // check for 24hr darkness or sunlight
    if (phidel < -1.0) {
        phidel = -1.0;
    } else if (phidel > 1.0) {
        phidel = 1.0;
    }

    double dawn;
    dawn = 12.0 - acos(phidel) * (180.0 / M_PI) / 15.0;

    return dawn;

}

double* zenith_array_from_times(time_profile_t *tprof, int julday, double lat) {

    double theta, delta, phi;

    theta = (2 * M_PI * julday) / 365.0;
    delta = 0.006918 - 0.399912 * cos(theta) + 0.070257 * sin(theta) - 0.006758 * cos(2. * theta) + 0.000907 * sin(
            2. * theta) - 0.002697 * cos(3. * theta) + 0.001480 * sin(3. * theta);


    phi = lat * (M_PI / 180.);

    double* zenith_r_array = (double*)malloc(sizeof(double) * tprof->count);

    int i;
    for (i = 0; i < tprof->count; ++i) {
        zenith_r_array[i] = compute_zenith(tprof->values[i], delta, phi);
    }

    return zenith_r_array;
}

double compute_zenith(double local_time, double delta, double phi){
    double th;
    th = (local_time - 12) * (M_PI / 12);

    double zen;
    zen = sin(delta) * sin(phi) + cos(delta) * cos(phi) * cos(th);

    if (zen < -1.0){
        zen = -1.0;
    } else if (zen > 1.0) {
        zen = 1.0;
    }

    zen = (M_PI / 2.0) - asin(zen);

    return zen;
}