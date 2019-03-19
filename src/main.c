#include "args.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "ezncdf/ezncdf.h"
#include "dwcpn/dwcpnc.h"
#include "vbn/vbn.h"

int main(int argc, char **argv) {

    float start_time = (float)clock() / CLOCKS_PER_SEC;

    args_t *args;
    args = parse_args(argc, argv);

    if (!validate_args(args)) {
        return EXIT_FAILURE;
    }


    ncfile_t *ncfile;
    ncfile = open_ncdf(args->infile, NC_WRITE);

//    print_ncdf_info(ncfile);

    ncdim_t *lat = get_file_dim_by_name(ncfile, "lat");
    ncdim_t *lon = get_file_dim_by_name(ncfile, "lon");

    size_t x, y;

    int chlid = file_has_var(ncfile, "chlor_a");

//    ncvar_t invar = ncfile->vars[chlid];
//    print_var_info(&invar,0);


//    printf("Element 0 is %f\n", *(float*)val);
//    printf("Element 1 is %f\n", *((float*)val+1));


    // current plan !!! write  a method to look up a specific element for a specific variable//

    // run dwcpn(ncfile)

    // initialisation of dwcpn parameters here, these will not change throughout the entire
    // execution
    dwcpn_params_t *dwcpn_params;
    dwcpn_params = (dwcpn_params_t*)malloc(sizeof(dwcpn_params_t));

    dwcpn_params->zmin = 0.0;
    dwcpn_params->zmax = 250.0;
    dwcpn_params->num_depths = 0;
    dwcpn_params->depth_step = 0.5; // we will let the profile generator auto calculate this

    req_data_t *tpos;
    tpos = (req_data_t*)malloc(sizeof(req_data_t));

    size_t ind1[1];
    size_t siz1[1] = {1};

    size_t ind[2];

    size_t count[] = {1,1};

    void *val = malloc(4);

    float result;

    for (x = 0; x < lon->size; ++x) {
//        printf("%d/%d\n", x, lon->size);
        for (y = 0; y < lat->size; ++y) {
            ind[0] = x;
            ind[1] = y;


//            nc_get_vara(ncfile->file_handle, chlid, ind, count, val);

            ind1[0] = y;
            nc_get_vara(ncfile->file_handle, file_has_var(ncfile, "lat"), ind1, siz1, val);
            tpos->lat = *(float *)val;

            ind1[0] = x;
            nc_get_vara(ncfile->file_handle, file_has_var(ncfile, "lon"), ind1, siz1, val);
            tpos->lon = *(float *)val;

            tpos->julday = 15;

            nc_get_vara(ncfile->file_handle, file_has_var(ncfile, "bathymetry"), ind, count, val);
            tpos->bathymetry = *(float *)val;

            nc_get_vara(ncfile->file_handle, file_has_var(ncfile, "PI_alpha"), ind, count, val);
            tpos->alpha_b = *(float *)val;

            nc_get_vara(ncfile->file_handle, file_has_var(ncfile, "PI_pmb"), ind, count, val);
            tpos->pm_b = *(float *)val;

            nc_get_vara(ncfile->file_handle, file_has_var(ncfile, "zm"), ind, count, val);
            tpos->zm = *(float *)val;

            nc_get_vara(ncfile->file_handle, file_has_var(ncfile, "chlor_a"), ind, count, val);
            tpos->b0 = *(float *)val;

            nc_get_vara(ncfile->file_handle, file_has_var(ncfile, "rho"), ind, count, val);
            tpos->rho = *(float *)val;

            nc_get_vara(ncfile->file_handle, file_has_var(ncfile, "h"), ind, count, val);
            tpos->h = *(float *)val;

            nc_get_vara(ncfile->file_handle, file_has_var(ncfile, "sigma"), ind, count, val);
            tpos->sigma = *(float *)val;


            tpos->cloud = 0.0;
            tpos->yel_sub = 0.3;

            nc_get_vara(ncfile->file_handle, file_has_var(ncfile, "par"), ind, count, val);
            tpos->sat_par = *(float *)val;

//            print_req_data(tpos);
            result = calc_pixel_pp(dwcpn_params, tpos);
//            printf("%f\n", result);
            nc_put_vara_float(ncfile->file_handle, file_has_var(ncfile, "pp"), ind, count, &result);
//
        }
//        nc_sync(ncfile->file_handle);

//        nc_close(ncfile->file_handle);
//        exit(EXIT_SUCCESS);
    }

//    printf("[%d,%d]: %f -> %f\n", (int)x, (int)y, *((float*)vals + offset), *(float *)val);

    float end_time = (float)clock() / CLOCKS_PER_SEC;

    printf("Time elapsed: %f\n", end_time-start_time);

//    free(tpos);
//    free(dwcpn_params);
}

