//
// Created by jad on 26/02/18.
//

#include "solar.h"
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

    if (dawn == 12.0) {

    }

}
