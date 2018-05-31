#ifndef EZNCDF_H
#define EZNCDF_H

#include <netcdf.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include "geo_math.h"

#define ERRCODE 2
#define ERR(e) {printf("Error: %s %s %d\n", nc_strerror(e), __FILE__, __LINE__); exit(ERRCODE);}

typedef struct {
    int num_rows;
    int *num_bins;
    int *start_index;
    double *lat_bins;
} sin_info_t;

typedef struct {
    char name[NC_MAX_NAME + 1];
    nc_type type;
    size_t len;
    void *value;
} ncatt_t;

typedef struct {
    int no_fill;
    size_t size;
    void *value;
} ncfill_t;

typedef struct {
    int index;
    int unlimited;
    char name[NC_MAX_NAME + 1];
    size_t size;
} ncdim_t;

typedef struct {
    int index;
    char name[NC_MAX_NAME + 1];
    nc_type type;
    ncfill_t *fill_info;
    int ndims;
    ncdim_t *dims;
    int numatts;
    ncatt_t *atts;
    int shuffle;
    int deflate;
    int deflate_level;
    int chunking;
    size_t *chunksizes;
    int f32checksum;
} ncvar_t;

typedef struct ncgroup_t {
    int grp_handle;
    char name[NC_MAX_NAME + 1];
    int ndims;
    ncdim_t *dims;
    int nvars;
    ncvar_t *vars;
    int nsubgroups;
    struct ncgroup_t *subgroups;
    int natts;
    ncatt_t *atts;
} ncgroup_t;

typedef struct {
    int file_handle;
    int ngroups;
    ncgroup_t *groups;
    int ndims;
    ncdim_t *dims;
    int nvars;
    ncvar_t *vars;
    int ngatts;
    ncatt_t *gatts;
    int nunlimdims;
    int ncformat;
} ncfile_t;

const char *prim_type_name(nc_type type);

const char *format_string(int format);

size_t size_of_nc_type(nc_type var_type);

sin_info_t *gen_sin_info(int num_rows);

// ############### GET INFO FROM NC FILE ####################
ncfile_t *open_ncdf(char *filename, int openmode);

ncfill_t *get_fill_info(int ncid, int varid);

ncdim_t *_get_dim_by_id(int ncid, int dimid);

int *_get_unlim_dims(int ncid, int nunlimdims);

ncvar_t *_get_var_by_id(int ncid, int varid);

ncgroup_t *_get_group_by_id(int ncid, int grpid);

ncatt_t *_get_att_by_id(int ncid, int varid, int attnum);

// ############### GET INFO FROM STRUCTS ####################

int file_has_att(ncfile_t *file, const char *name);

int file_has_dim(ncfile_t *file, const char *name);

int file_has_var(ncfile_t *file, const char *name);

int var_has_attr(ncvar_t *var, const char *name);

int var_has_dim(ncvar_t *var, const char *name);

ncatt_t *get_var_att_by_name(ncvar_t *var, const char *name);

ncdim_t *get_file_dim_by_name(ncfile_t *file, const char *name);

void print_ncdf_info(ncfile_t *ncfile);

void print_var_info(ncvar_t *var, int indentlvl);

void print_group_info(ncgroup_t *group, int indentlvl);

void print_att_info(ncatt_t *att, int identlvl);

// ################ OUTPUT FUNCTIONS #################

ncfile_t *init_empty_template();

void update_template_ids(ncfile_t *tpl);

int add_att_to_template(ncfile_t *tpl, ncatt_t *att);

int add_dim_to_template(ncfile_t *tpl, const char *name, size_t size);

int add_var_to_template(ncfile_t *tpl, ncvar_t *var, const char *sin_dim);

int add_coord_var_to_template(ncfile_t *tpl, ncvar_t *var, const char *sin_dim);

int write_template_to_file(ncfile_t *tpl, const char *filename, int write_mode);

int write_var_to_file(ncfile_t *file, int varid, void *arr);

int copy_var_to_file(ncfile_t *input_file, int input_var_id, ncfile_t *output_file, int output_var_id);

// ############### MEMORY CLEAN UP ##################
void free_var(ncvar_t *var);

void free_group(ncgroup_t *group);

void free_ncfile(ncfile_t *ncfile);

// ################# MISC FUNCTIONS ###################
int sin_rows_from_len(uint64_t len);

uint64_t sin_len_from_rows(int rows);

#endif
