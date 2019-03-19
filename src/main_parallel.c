//
// Created by jad on 01/06/18.
//

#include "main_parallel.h"
#include "args.h"
#include "threadpool/threadpool.h"
#include "ezncdf/ezncdf.h"
#include "dwcpn/dwcpnc.h"
#include <sys/sysinfo.h>

int main(int argc, char **argv) {

    args_t *args;
    args = parse_args(argc, argv);

    if (!validate_args(args)) {
        return EXIT_FAILURE;
    }

    printf("creating queue\n");
    work_queue_t *queue;
    printf("initialising queue\n");
    queue = init_queue((uint16_t)(1024)); //initialise work queue with max length of 20


    printf("creating pool\n");
    thread_pool_t *pool;
    if (args->nthreads == 0) {
        printf("Automatic thread count choice....\n");
        printf("\tThis system has %d procs configured and %d procs available\n",get_nprocs_conf(), get_nprocs());
        printf("\tSetting threadpool to use %d cores\n", get_nprocs() / 2);
        printf("initialising pool\n");
        pool = init_thread_pool((uint16_t )(get_nprocs() / 2), queue);
    } else {
        printf("initialising pool\n");
        pool = init_thread_pool((uint16_t )args->nthreads, queue);
    }



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

    // variable objects
    ncvar_t bathymetry = ncfile->vars[file_has_var(ncfile, "bathymetry")];
    ncvar_t PI_alpha = ncfile->vars[file_has_var(ncfile, "PI_alpha")];
    ncvar_t PI_pmb = ncfile->vars[file_has_var(ncfile, "PI_pmb")];
    ncvar_t zm = ncfile->vars[file_has_var(ncfile, "zm")];
    ncvar_t chlor_a = ncfile->vars[file_has_var(ncfile, "chlor_a")];
    ncvar_t rho = ncfile->vars[file_has_var(ncfile, "rho")];
    ncvar_t h = ncfile->vars[file_has_var(ncfile, "h")];
    ncvar_t sigma = ncfile->vars[file_has_var(ncfile, "sigma")];
    ncvar_t par = ncfile->vars[file_has_var(ncfile, "par")];

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
//    for (x = 0; x < 50; ++x) {
        printf("%d/%d\n", (int)x, (int)lon->size);
        for (y = 0; y < lat->size; ++y) {

            tpos = (req_data_t *) malloc(sizeof(req_data_t));
            pthread_mutex_lock(ncfile->lock);

            ind = (size_t*)malloc(2);



            ind[0] = x;
            ind[1] = y;


            ind1[0] = y;
//            nc_get_vara_float(ncfile->file_handle, file_has_var(ncfile, "lat"), ind1, siz1, &tpos->lat);

            nc_get_vara(ncfile->file_handle, file_has_var(ncfile, "lat"), ind1, siz1, val);
            tpos->lat = *(float *) val;

            ind1[0] = x;
            nc_get_vara(ncfile->file_handle, file_has_var(ncfile, "lon"), ind1, siz1, val);
            tpos->lon = *(float *) val;
//            nc_get_vara_float(ncfile->file_handle, file_has_var(ncfile, "lon"), ind1, siz1, &tpos->lon);


            tpos->julday = 180;

            nc_get_vara(ncfile->file_handle, bathymetry.index, ind, count, val);
            tpos->bathymetry = *(float *) val;
//            nc_get_vara_float(ncfile->file_handle, file_has_var(ncfile, "bathymetry"), ind1, siz1, &tpos->bathymetry);
            if (tpos->bathymetry == *(float*)bathymetry.fill_info->value) {
                pthread_mutex_unlock(ncfile->lock);
                continue;
            }

            nc_get_vara(ncfile->file_handle, PI_alpha.index, ind, count, val);
            tpos->alpha_b = *(float *) val;
//            nc_get_vara_float(ncfile->file_handle, file_has_var(ncfile, "PI_alpha"), ind1, siz1, &tpos->alpha_b);
            if (tpos->alpha_b == *(float*)PI_alpha.fill_info->value) {
                pthread_mutex_unlock(ncfile->lock);
                continue;
            }

            nc_get_vara(ncfile->file_handle, PI_pmb.index, ind, count, val);
            tpos->pm_b = *(float *) val;
            if (tpos->pm_b == *(float*)PI_pmb.fill_info->value) {
                pthread_mutex_unlock(ncfile->lock);
                continue;
            }

            nc_get_vara(ncfile->file_handle, zm.index, ind, count, val);
            tpos->zm = *(float *) val;
//            nc_get_vara_float(ncfile->file_handle, file_has_var(ncfile, "zm"), ind1, siz1, &tpos->zm);
            if (tpos->zm == *(float*)zm.fill_info->value) {
                pthread_mutex_unlock(ncfile->lock);
                continue;
            }

            nc_get_vara(ncfile->file_handle, chlor_a.index, ind, count, val);
            tpos->b0 = *(float *) val;
//            nc_get_vara_float(ncfile->file_handle, file_has_var(ncfile, "chlor_a"), ind1, siz1, &tpos->b0);
            if (tpos->b0 == *(float*)chlor_a.fill_info->value) {
                pthread_mutex_unlock(ncfile->lock);
                continue;
            }

            nc_get_vara(ncfile->file_handle, rho.index, ind, count, val);
            tpos->rho = *(float *) val;
//            nc_get_vara_float(ncfile->file_handle, file_has_var(ncfile, "rho"), ind1, siz1, &tpos->rho);
            if (tpos->rho == *(float*)rho.fill_info->value) {
                pthread_mutex_unlock(ncfile->lock);
                continue;
            }

            nc_get_vara(ncfile->file_handle, h.index, ind, count, val);
            tpos->h = *(float *) val;
//            nc_get_vara_float(ncfile->file_handle, file_has_var(ncfile, "h"), ind1, siz1, &tpos->h);
//            if (h.fill_info->no_fill == 0) {
//                if (tpos->h == *(float *) h.fill_info->value) {
//                    continue;
//                }
//            }

            nc_get_vara(ncfile->file_handle, sigma.index, ind, count, val);
            tpos->sigma = *(float *) val;
//            nc_get_vara_float(ncfile->file_handle, file_has_var(ncfile, "sigma"), ind1, siz1, &tpos->sigma);
            if (tpos->sigma == *(float*)sigma.fill_info->value) {
                pthread_mutex_unlock(ncfile->lock);
                continue;
            }


            tpos->cloud = 0.0;
            tpos->yel_sub = 0.3;

            nc_get_vara(ncfile->file_handle, par.index, ind, count, val);
            tpos->sat_par = *(float *) val;
//            nc_get_vara_float(ncfile->file_handle, file_has_var(ncfile, "par"), ind1, siz1, &tpos->sat_par);
            if (tpos->sat_par == *(float*)par.fill_info->value) {
                pthread_mutex_unlock(ncfile->lock);
                continue;
            } else {
                pthread_mutex_unlock(ncfile->lock);
            }


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
//                usleep(1000);
            }

        }
        pthread_mutex_lock(ncfile->lock);
        nc_sync(ncfile->file_handle);
        pthread_mutex_unlock(ncfile->lock);

    }

//    printf("Submission loop finished, waiting until jobs finish to kill queue\n");

    int ic;
    pthread_mutex_lock(queue->lock);
    ic = queue->item_count;
    pthread_mutex_unlock(queue->lock);
    while (ic > 0) {

//        printf("Jobs remaining in queue: %d\n", queue->item_count);
        usleep(1000);
        pthread_mutex_lock(queue->lock);
        ic = queue->item_count;
        pthread_mutex_unlock(queue->lock);
//        asm ("nop");
    }

//    printf("Item count hit zero, doing one final sync...");

    pthread_mutex_lock(ncfile->lock);
//    printf("lock obtained......");
    nc_sync(ncfile->file_handle);
//    printf("ncfil synced......");
    pthread_mutex_unlock(ncfile->lock);
//    printf("mutex released......\n");
    pool->status = pool_stopping;

//    usleep(1000); // wait one second for threads to catch up!

//    printf("Freeing pool.....");
    free_thread_pool(pool);
//    printf("freed\n");

//    printf("Freeing queue.....\n");
    free_queue(queue);
//    printf("freed\n");

    printf("Going for one last sync...");
    pthread_mutex_lock(ncfile->lock);
    printf("lock acquired...");
    ncsync(ncfile->file_handle);
    printf("sync complete...");
    pthread_mutex_unlock(ncfile->lock);
    printf("lock released\n");


    free_ncfile(ncfile);
    free(args);
    return 0;

}

void f(void *params) {

    f_args_t f_args;
    f_args = *(f_args_t *)params;

    float result;

    result = calc_pixel_pp(f_args.dwcpn_params, f_args.req_data);

    if (!(isinf(result)) || (result <= 1000000.0) || result > 0.0) {


        // write chunk row to output file
        pthread_mutex_lock(f_args.inputfile->lock);
        nc_put_vara_float(f_args.inputfile->file_handle, file_has_var(f_args.inputfile, "pp"), f_args.ind, f_args.count, &result);
//        nc_sync(f_args.inputfile->file_handle);
        pthread_mutex_unlock(f_args.inputfile->lock);
    }

    free(f_args.req_data);
    free(f_args.ind);
    free(params);

}
