//
// Created by jad on 26/02/18.
//

#ifndef DWCPNC_SOLAR_H
#define DWCPNC_SOLAR_H

#include "profile.h"
#include <stdlib.h>
#include <stdio.h>


static int DayNo[25] = {0,3,31,42,59,78,90,93,120,133,151,170,
                        181,183,206,212,243,265,273,277,304,306,
                        334,355,365};

// Extraterrestrial Solar Irradiance from Thekaekara 1977 (ref in JGR 1988 - s & p)
static int SolIr[25] = {1399,1399,1393,1389,1378,1364,1355,1353,
                        1332,1324,1316,1310,1309,1309,1312,1313,
                        1329,1344,1350,1353,1347,1375,1392,1398,
                        1399};

double compute_sunrise(int jday, double lat);
double* zenith_array_from_times(time_profile_t *tprof, int julday, double lat);
double compute_zenith(double local_time, double delta, double phi);

#endif //DWCPNC_SOLAR_H
