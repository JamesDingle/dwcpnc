//
// Created by jad on 01/06/18.
//

#ifndef DWCPNC_MAIN_PARALLEL_H
#define DWCPNC_MAIN_PARALLEL_H

#include <stdlib.h>
#include "ezncdf/ezncdf.h"
#include "dwcpn/dwcpnc.h"
#include "vbn/vbn.h"
#include "threadpool/threadpool.h"
#include "args.h"

typedef struct {
    ncfile_t *inputfile;
    dwcpn_params_t *dwcpn_params;
    req_data_t *req_data;
    size_t *ind;
    size_t *count;
} f_args_t;

int main(int argc, char * argv[]);
void f(void *params);

#endif //DWCPNC_MAIN_PARALLEL_H
