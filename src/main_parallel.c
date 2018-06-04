//
// Created by jad on 01/06/18.
//

#include "main_parallel.h"
#include "args.h"
#include "threadpool/threadpool.h"

int main(int argc, char **argv) {

    float start_time = (float) clock() / CLOCKS_PER_SEC;

    args_t *args;
    args = parse_args(argc, argv);

    if (!validate_args(args)) {
        return EXIT_FAILURE;
    }


    printf("creating queue\n");
    work_queue_t *queue;
    printf("initialising queue\n");
    queue = init_queue(100); //initialise work queue with max length of 20

    printf("creating pool\n");
    thread_pool_t *pool;
    printf("initialising pool\n");
    pool = init_thread_pool((uint16_t )args->nthreads, queue);

    // initialisation of dwcpn parameters here, these will not change throughout the entire
    // execution
    dwcpn_params_t *dwcpn_params;
    dwcpn_params = (dwcpn_params_t *) malloc(sizeof(dwcpn_params_t));

    dwcpn_params->zmin = 0.0;
    dwcpn_params->zmax = 250.0;
    dwcpn_params->num_depths = 0;
    dwcpn_params->depth_step = 0.5; // we will let the profile generator auto calculate this

    ncfile_t *ncfile;
    ncfile = open_ncdf(args->infile, NC_WRITE);

//    print_ncdf_info(ncfile);

    ncdim_t *lat = get_file_dim_by_name(ncfile, "lat");
    ncdim_t *lon = get_file_dim_by_name(ncfile, "lon");

    req_data_t *tpos;
//    tpos = (req_data_t*)malloc(sizeof(req_data_t));

    size_t ind1[1];
    size_t siz1[1] = {1};

    size_t *ind;

    size_t count[] = {1, 1};

    void *val = malloc(4);

    int res;
    f_args_t *f_args;
    task_t *task;

    size_t x, y;
    for (x = 0; x < lon->size; ++x) {
//    for (x = 0; x < 1; ++x) {
        printf("%d/241\n", x);
        for (y = 0; y < lat->size; ++y) {

            tpos = (req_data_t *) malloc(sizeof(req_data_t));
            ind = (size_t*)malloc(2);

            ind[0] = x;
            ind[1] = y;


            ind1[0] = y;
            nc_get_vara(ncfile->file_handle, file_has_var(ncfile, "lat"), ind1, siz1, val);
            tpos->lat = (double) *(float *) val;

            ind1[0] = x;
            nc_get_vara(ncfile->file_handle, file_has_var(ncfile, "lon"), ind1, siz1, val);
            tpos->lon = (double) *(float *) val;

            tpos->julday = 15;

            nc_get_vara(ncfile->file_handle, file_has_var(ncfile, "bathymetry"), ind, count, val);
            tpos->bathymetry = (double) *(float *) val;

            nc_get_vara(ncfile->file_handle, file_has_var(ncfile, "PI_alpha"), ind, count, val);
            tpos->alpha_b = (double) *(float *) val;

            nc_get_vara(ncfile->file_handle, file_has_var(ncfile, "PI_pmb"), ind, count, val);
            tpos->pm_b = (double) *(float *) val;

            nc_get_vara(ncfile->file_handle, file_has_var(ncfile, "zm"), ind, count, val);
            tpos->zm = (double) *(float *) val;

            nc_get_vara(ncfile->file_handle, file_has_var(ncfile, "chlor_a"), ind, count, val);
            tpos->b0 = (double) *(float *) val;

            nc_get_vara(ncfile->file_handle, file_has_var(ncfile, "rho"), ind, count, val);
            tpos->rho = (double) *(float *) val;

            nc_get_vara(ncfile->file_handle, file_has_var(ncfile, "h"), ind, count, val);
            tpos->h = (double) *(float *) val;

            nc_get_vara(ncfile->file_handle, file_has_var(ncfile, "sigma"), ind, count, val);
            tpos->sigma = (double) *(float *) val;


            tpos->cloud = 0.0;
            tpos->yel_sub = 0.3;

            nc_get_vara(ncfile->file_handle, file_has_var(ncfile, "par"), ind, count, val);
            tpos->sat_par = (double) *(float *) val;

            f_args = (f_args_t *) malloc(sizeof(f_args_t));
            f_args->inputfile = ncfile;
            f_args->dwcpn_params = dwcpn_params;
            f_args->req_data = tpos;
            f_args->ind = ind;
            f_args->count = count;

            task = malloc(sizeof(task_t));
            task->function_ptr = &f;
            task->function_args = f_args;
            res = 1;
            while (res) {
                res = add_task(queue, task);
            }
        }

    }

    while (queue->item_count > 0) {
//        usleep(10);
        asm ("nop");
    }

    pool->status = pool_stopping;

    usleep(1000); // wait one second for threads to catch up!

    free_thread_pool(pool);


    free_queue(queue);
    free_ncfile(ncfile);
    free(args);
    return 0;

}

void f(void *params) {

    f_args_t f_args;
    f_args = *(f_args_t *)params;

    float result;

    result = calc_pixel_pp(f_args.dwcpn_params, f_args.req_data);

    // write chunk row to output file
    pthread_mutex_lock(f_args.inputfile->lock);

    //nc_put_vara_float(f_args.inputfile->file_handle, file_has_var(f_args.inputfile, "pp"), f_args.ind, f_args.count, &result);

    nc_sync(f_args.inputfile->file_handle);
    pthread_mutex_unlock(f_args.inputfile->lock);

    free(f_args.req_data);
    free(f_args.ind);
    free(params);

}
