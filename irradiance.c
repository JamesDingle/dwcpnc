//
// Created by jad on 27/02/18.
//

#include "irradiance.h"


double compute_airmass(double zenith_r, double zenith_d) {
    double airmass;
    airmass = pow(cos(zenith_r) + 0.15 * pow(93.885 - zenith_d, -1.253), -1.0);
    if (airmass < 1.0) {
        airmass = 1.0;
    }

    return airmass;
}