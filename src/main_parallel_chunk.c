//
// Created by jad on 01/06/18.
//

#include "main_parallel_chunk.h"
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
        printf("Automatic thread count choice....");
        printf("\tThis system has %d procs configured and %d procss available\n",get_nprocs_conf(), get_nprocs());
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

    req_data_chunk_t *tpos;
//    tpos = (req_data_t*)malloc(sizeof(req_data_t));

    size_t ind1[1];
    size_t siz1[1] = {1};

    size_t *ind;
    size_t count[] = {1, 1};

    void *val = malloc(4);

    int res;
    f_args_t *f_args;
    task_t *task;


    ca_array_t test;
    test.ndims = chlor_a.ndims;

    test.chunk_arrays = (chunk_array_t**)malloc(sizeof(chunk_array_t*) * test.ndims);

    int d;

    for (d = 0; d < test.ndims; ++d) {
        test.chunk_arrays[d] = gen_chunks((int)chlor_a.dims[d].size, 10);
    }


    vbn_t *vbn;

    vbn = init_vbn(test.ndims);

    int i,j;
    for (i = 0; i < test.ndims; ++i) {
        vbn->digit_max[i] = test.chunk_arrays[i]->count - 1;
    }

    i = 0;
    do {
//        printf("%03d: ", i);
//        for (j = 0; j < vbn->digit_count; ++j) {
////            print_chunk(*test.chunk_arrays[j]->chunks[vbn->digit_value[j]]);
////            printf(", ");
//        }
        print_vbn(vbn);

//        printf("\n");
        ++i;
    } while(increment_vbn(vbn));

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

//    printf("Going for one last sync...");
    pthread_mutex_lock(ncfile->lock);
//    printf("lock acquired...");
    ncsync(ncfile->file_handle);
//    printf("sync complete...");
    pthread_mutex_unlock(ncfile->lock);
//    printf("lock released\n");


    free_ncfile(ncfile);
    free(args);
    return 0;

}

req_data_chunk_t *read_input_chunk(ncfile_t *ncfile, vbn_t *vbn, ca_array_t *ca_array) {

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
