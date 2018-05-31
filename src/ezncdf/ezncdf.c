#include "ezncdf.h"

/*
 * Function:  prim_type_name
 * --------------------
 * taken from some netcdf example code that returns a "name" string for a
 * given nc_type
 *
 *  type: (nc_type) enum type from the netcdf C library
 *
 *  returns: string describing the type
 */
const char *
prim_type_name(nc_type type) {
    switch (type) {
        case NC_BYTE:
            return "byte";
        case NC_CHAR:
            return "char";
        case NC_SHORT:
            return "short";
        case NC_INT:
            return "int";
        case NC_FLOAT:
            return "float";
        case NC_DOUBLE:
            return "double";
        case NC_UBYTE:
            return "ubyte";
        case NC_USHORT:
            return "ushort";
        case NC_UINT:
            return "uint";
        case NC_INT64:
            return "int64";
        case NC_UINT64:
            return "uint64";
        case NC_STRING:
            return "string";
        case NC_VLEN:
            return "vlen";
        case NC_OPAQUE:
            return "opaque";
        case NC_COMPOUND:
            return "compound";
        default:
            printf("prim_type_name: bad type %d", type);
            return "bogus";
    }
}


/*
 * Function: format_string
 * --------------------
 * function to provide a string that describes the format of a netcdf file.
 * useful for printing/logging helpful messages
 *
 *  format (int): enum type from netcdf C library
 *
 *  returns: string describing nc file format
 */
const char *
format_string(int format) {
    switch (format) {
        case NC_FORMAT_CLASSIC:
            return "classic";
        case NC_FORMAT_64BIT:
            return "64-bit offset";
        case NC_FORMAT_NETCDF4:
            return "netCDF-4";
        case NC_FORMAT_NETCDF4_CLASSIC:
            return "netCDF-4 classic model";
        default:
            fprintf(stderr, "Unrecognised file format :(");
            return "unrecognised";
    }
}

/*
 * Function:  size_of_nc_type
 * --------------------
 * Lookup function for the element size of a given netcdf data type
 *
 *  var_type (nc_type): enum type from netcdf C library that describes the
 *                      type of the netcdf variable
 *
 *  returns: size_t of a single element of the given netcdf data type
 */
size_t
size_of_nc_type(nc_type var_type) {

    size_t size;

    switch (var_type) {
        case NC_INT:
            size = sizeof(int32_t);
            break;
        case NC_UINT:
            size = sizeof(uint32_t);
            break;
        case NC_UINT64:
            size = sizeof(uint64_t);
            break;
        case NC_INT64:
            size = sizeof(int64_t);
            break;
        case NC_SHORT:
            size = sizeof(int16_t);
            break;
        case NC_FLOAT:
            size = sizeof(float);
            break;
        case NC_DOUBLE:
            size = sizeof(double);
            break;
        case NC_BYTE:
            size = sizeof(int8_t);
            break;
        case NC_UBYTE:
            size = sizeof(uint8_t);
            break;
        case NC_CHAR:
            size = sizeof(int8_t);
            break;
        default:
            abort();
    }

    return size;
}

/*
 * Function:  gen_sin_info
 * --------------------
 * Calculates the number of elements, start_index of each longitude "row" and
 * latitude value of each longitude "row" and populate the struct.
 *
 *  num_rows (int): number of rows to split up the latitude values.
 *
 *  returns: sin_info_t struct containing all of the calculated information
 *           needed to describe the sinusoidal data
 */
sin_info_t *
gen_sin_info(int num_rows) {
    int row_start = 0;

    sin_info_t *sin_info = (sin_info_t *) malloc(sizeof(sin_info_t));

    sin_info->num_rows = num_rows;
    sin_info->num_bins = (int *) calloc(num_rows, sizeof(int));
    sin_info->start_index = (int *) calloc(num_rows, sizeof(int));
    sin_info->lat_bins = (double *) calloc(num_rows, sizeof(double));

    if ((sin_info->num_bins == NULL) || (sin_info->start_index == NULL) || (sin_info->lat_bins == NULL)) {
        fprintf(stderr, "Could not allocate memory for sin_info_t\n");
    }

    int row;
    for (row = 0; row < num_rows; ++row) {
        sin_info->lat_bins[row] = ((row + 0.5) * 180.0 / num_rows) - 90.0;
        //sin_info->num_bins[row] = floor_double_to_uint16((2.0f * num_rows * cos(degs_to_rads(sin_info->lat_bins[row]))) + 0.5f);
        sin_info->num_bins[row] = (int) (floor((2.0 * num_rows * cos(degs_to_rads(sin_info->lat_bins[row]))) + 0.5));
        sin_info->start_index[row] = row_start;
        row_start += sin_info->num_bins[row];
    }

    return sin_info;
}

/*
 * Function:  open_ncdf
 * --------------------
 * uses the native netCDF C library functions to fill in the information in
 * the struct representation of the netcdf file. recursively searches into
 * groups
 *
 *  filename (char*): path/filename of the input netcdf file
 *  openmode (int): enum passthrough to the nc_open function (as "omode").
 *                   NC_NOWRITE = read-only, buffering and caching
 *                   NC_WRITE = opens the dataset with read-write access.
 *  more info: http://www.unidata.ucar.edu/software/netcdf/netcdf-4/newdocs/netcdf-c/nc_005fopen.html
 *
 *  returns: pointer to nc_file_t struct that is populated with all of the
 *           information pertinent to describing this netcdf file completely
 */
ncfile_t *
open_ncdf(char *filename, int openmode) {

    ncfile_t *ncfile = (ncfile_t *) malloc(sizeof(ncfile_t));

    int retval, ncid;
    if ((retval = nc_open(filename, openmode, &ncid))) {
        ERR(retval);
    }

    // Disabled the reading of number of unlimited dimensions here (nunlimdims)
    // as for files with multiple unlimited dimensions, it still shows just 1
    // replaced below as nc_inq_unlimdims
    int ndims, nvars, ngatts;
    if ((retval = nc_inq(ncid, &ndims, &nvars, &ngatts, NULL))) {
        ERR(retval);
    }

    int nunlimdims;
    if ((nc_inq_unlimdims(ncid, &nunlimdims, NULL))) {
        ERR(retval);
    }

    int ngroups;
    if ((retval = nc_inq_grps(ncid, &ngroups, NULL))) {
        ERR(retval);
    }

    int ncformat;
    if ((retval = nc_inq_format(ncid, &ncformat))) {
        ERR(retval);
    }

    ncfile->file_handle = ncid;
    ncfile->ngroups = ngroups;
    ncfile->nvars = nvars;
    ncfile->ndims = ndims;
    ncfile->ngatts = ngatts;
    ncfile->nunlimdims = nunlimdims;
    ncfile->ncformat = ncformat;

    // DIMENSION INFORMATION HANDLING

    ncdim_t *dims = (ncdim_t *) malloc(sizeof(ncdim_t) * ndims);
    int i;
    for (i = 0; i < ndims; ++i) {
        dims[i] = *_get_dim_by_id(ncid, i);
    }
    ncfile->dims = dims;

    // MARK RECORD DIMENSIONS AS UNLIMITED

    int *unlimdimsp;
    unlimdimsp = _get_unlim_dims(ncid, nunlimdims);

    if (nunlimdims != -1) {
        for (i = 0; i < nunlimdims; ++i) {
            ncfile->dims[unlimdimsp[i]].unlimited = 1;
        }
    }

    // GROUP INFORMATION HANDLING
    ncgroup_t *groups = (ncgroup_t *) malloc(sizeof(ncgroup_t) * ngroups);

    int *group_handles;
    group_handles = (int *) malloc(sizeof(int) * ngroups);

    if ((retval = nc_inq_grps(ncid, NULL, group_handles))) {
        ERR(retval);
    }

    for (i = 0; i < ngroups; ++i) {
        groups[i] = *_get_group_by_id(ncid, group_handles[i]);
    }
    ncfile->groups = groups;

    // VARIABLE INFORMATION HANDLING
    ncvar_t *vars = (ncvar_t *) malloc(sizeof(ncvar_t) * nvars);

    for (i = 0; i < nvars; ++i) {
        vars[i] = *_get_var_by_id(ncid, i);
    }

    ncfile->vars = vars;

    // GLOBAL ATTRIBUTE HANDLING
    ncatt_t *gatts = (ncatt_t *) malloc(sizeof(ncatt_t) * ngatts);
    for (i = 0; i < ngatts; ++i) {
        gatts[i] = *_get_att_by_id(ncid, NC_GLOBAL, i);
    }

    ncfile->gatts = gatts;

    free(group_handles);
    return ncfile;
}

/*
 * Function:  get_fill_info
 * --------------------
 * gets information about the fill value of a specified variable in an open
 * netcdf file handle.
 *
 *  ncid (int): file handle of an open netcdf file
 *  varid (int): id of the variable you wish to look up
 *
 *  returns: pointer to nc_fill_t struct that contains the information relevant
 *           to reading/using the fill value. (size, type, value)
 */
ncfill_t *
get_fill_info(int ncid, int varid) {

    int retval;
    nc_type var_type;
    void *fill_value;
    int no_fill = 0;
    size_t size = 0;

    ncfill_t *fill_info = (ncfill_t *) malloc(sizeof(ncfill_t));

    if ((retval = nc_inq_vartype(ncid, varid, &var_type))) {
        ERR(retval);
    }

    size = size_of_nc_type(var_type);
    fill_value = malloc(size);

    // NOTE: nc_inq_var_fill does NOT work as it will return the default fillvalue
    // for the given type
    if ((retval = nc_get_att(ncid, varid, _FillValue, fill_value))) {
        fill_info->no_fill = 1;
        fill_info->size = 0;
        fill_info->value = NULL;
        free(fill_value); // since we're not storing the pointer here we need to de-allocate
        return fill_info;
    }

    fill_info->no_fill = no_fill;
    fill_info->size = size;
    fill_info->value = fill_value;

    return fill_info;
}

/*
 * Function:  _get_unlimdim_by_id
 * --------------------
 * internal function to read information about a specific unlimited dimension
 * from an open netcdf file handle
 *
 *  ncid (int):file handle of a currently opened netcdf file
 *  dimid (int): id of the unlimited dimension to look up
 *
 *  returns: pointer to ncdim_t struct that will store id, name, size which
 *           is everything needed to access/manipulate this dimension later
 */
int *
_get_unlim_dims(int ncid, int nunlimdims) {

    int *unlimdimidsp;
    unlimdimidsp = (int *) malloc(sizeof(int) * nunlimdims);


    int retval;
    if ((retval = nc_inq_unlimdims(ncid, NULL, unlimdimidsp))) {
        ERR(retval);
    }

    return unlimdimidsp;
}

/*
 * Function:  _get_dim_by_id
 * --------------------
 * internal function to read information about a specific dimension from an
 * open netcdf file handle
 *
 *  ncid (int):file handle of a currently opened netcdf file
 *  dimid (int): id of the dimension to look up
 *
 *  returns: pointer to ncdim_t struct that will store id, name, size which
 *           is everything needed to access/manipulate this dimension later
 */
ncdim_t *
_get_dim_by_id(int ncid, int dimid) {

    ncdim_t *dim = (ncdim_t *) malloc(sizeof(ncdim_t));

    dim->index = dimid;
    dim->unlimited = 0; // set by default, to be modified later

    int retval;
    if ((retval = nc_inq_dim(ncid, dimid, dim->name, &dim->size))) {
        ERR(retval);
    }

    return dim;
}

/*
 * Function: _get_var_by_id
 * --------------------
 * internal function to read information about a specific variable from an
 * open netcdf file handle
 *
 *  ncid (int): file handle of a currently opened netcdf file
 *  varid (int): id of the variable to look up
 *
 *  returns: pointer to ncvar_t struct that will store all information needed
 *           to read/interpret this variable in other functions
 */
ncvar_t *
_get_var_by_id(int ncid, int varid) {

    int i; // count var
    ncvar_t *var = (ncvar_t *) malloc(sizeof(ncvar_t));
    var->index = varid;

    int retval, dimids[NC_MAX_VAR_DIMS];
    if ((retval = nc_inq_var(ncid, varid, var->name, &var->type, &var->ndims,
                             dimids, &var->numatts))) {
        ERR(retval);
    }

    var->dims = (ncdim_t *) malloc(sizeof(ncdim_t) * var->ndims);
    for (i = 0; i < var->ndims; ++i) {
        var->dims[i] = *_get_dim_by_id(ncid, dimids[i]);
    }

    if ((retval = nc_inq_var_deflate(ncid, varid, &var->shuffle, &var->deflate,
                                     &var->deflate_level))) {
        ERR(retval);
    }

    var->chunksizes = (size_t *) malloc(var->ndims * sizeof(size_t));
    if ((retval = nc_inq_var_chunking(ncid, varid, &var->chunking, var->chunksizes))) {
        ERR(retval);
    }

    if ((retval = nc_inq_var_fletcher32(ncid, varid, &var->f32checksum))) {
        ERR(retval);
    }

    var->fill_info = get_fill_info(ncid, varid);

    ncatt_t *atts;
    atts = (ncatt_t *) malloc(sizeof(ncatt_t) * var->numatts);

    for (i = 0; i < var->numatts; ++i) {
        atts[i] = *_get_att_by_id(ncid, varid, i);
    }
    var->atts = atts;

    return var;
}

/*
 * Function:  _get_group_by_id
 * --------------------
 * reads information about a specified group from an open netcdf filehandle
 *
 *  ncid (int): file handle for an open netcdf file
 *  grpid (int): id number of the group to get
 *
 *  returns: pointer to ncgroup_t struct containing information that describes
 *           the contents and layout of this group.
 */
ncgroup_t *
_get_group_by_id(int ncid, int grpid) {

    ncgroup_t *group;
    group = (ncgroup_t *) malloc(sizeof(ncgroup_t));

    group->grp_handle = grpid;

    int retval;
    if ((retval = nc_inq_grpname(grpid, group->name))) {
        ERR(retval);
    }

    // LOOK UP DIMENSIONS USED IN THIS GROUP
    int grpndims;
    if ((retval = nc_inq_ndims(grpid, &grpndims))) {
        ERR(retval);
    }
    group->ndims = grpndims;

    group->dims = (ncdim_t *) malloc(sizeof(ncdim_t) * grpndims);

    int *grpdimids;
    grpdimids = (int *) malloc(sizeof(int) * grpndims);

    if ((retval = nc_inq_dimids(grpid, NULL, grpdimids, 0))) {
        ERR(retval);
    }

    int i;
    for (i = 0; i < grpndims; ++i) {
        group->dims[i] = *_get_dim_by_id(grpid, grpdimids[i]);
    }

    // LOOKUP VARIABLES CONTAINED IN THIS GROUP
    int grpnvars;
    if ((retval = nc_inq_varids(grpid, &grpnvars, NULL))) {
        ERR(retval);
    }
    group->nvars = grpnvars;

    group->vars = (ncvar_t *) malloc(sizeof(ncvar_t) * grpnvars);

    int *grpvarids;
    grpvarids = (int *) malloc(sizeof(int) * grpnvars);

    if ((retval = nc_inq_varids(grpid, NULL, grpvarids))) {
        ERR(retval);
    }

    for (i = 0; i < grpnvars; ++i) {
        group->vars[i] = *_get_var_by_id(grpid, grpvarids[i]);
    }

    // LOOK UP SUBGROUPS CONTAINED IN THIS GROUP
    int grpnsubgroups;
    if ((retval = nc_inq_grps(grpid, &grpnsubgroups, NULL))) {
        ERR(retval);
    }
    group->nsubgroups = grpnsubgroups;

    group->subgroups = (ncgroup_t *) malloc(sizeof(ncgroup_t) * grpnsubgroups);

    int *subgrpids;
    subgrpids = (int *) malloc(sizeof(int) * grpnsubgroups);

    if ((retval = nc_inq_grps(grpid, NULL, subgrpids))) {
        ERR(retval);
    }

    for (i = 0; i < grpnsubgroups; ++i) {
        group->subgroups[i] = *_get_group_by_id(grpid, subgrpids[i]);
    }

    // LOOK UP ATTRIBUTES CONTAINED IN THIS GROUp

    int natts;
    if ((retval = nc_inq_natts(grpid, &natts))) {
        ERR(retval);
    }

    group->atts = (ncatt_t *) malloc(sizeof(ncatt_t) * natts);

    for (i = 0; i < natts; ++i) {
        group->atts[i] = *_get_att_by_id(grpid, NC_GLOBAL, i);
    }

    return group;
}

/*
 * Function:  _get_att_by_id
 * --------------------
 * reads information about a specific attribute from a specific variable in an
 * open netcdf file handle
 *
 *  ncid (int): file handle of an open netcdf file
 *  varid (int): index of the variable that contains the attribute
 *  attnum (int): index of the attribute we want
 *
 *  returns: pointer to ncatt_t struct that fully describes the attribute
 */
ncatt_t *
_get_att_by_id(int ncid, int varid, int attnum) {

    ncatt_t *att;
    att = (ncatt_t *) malloc(sizeof(ncatt_t));

    int retval;
    if ((retval = nc_inq_attname(ncid, varid, attnum, att->name))) {
        ERR(retval);
    }

    if ((retval = nc_inq_atttype(ncid, varid, att->name, &att->type))) {
        ERR(retval);
    }

    if ((retval = nc_inq_attlen(ncid, varid, att->name, &att->len))) {
        ERR(retval);
    }

    if (att->type == NC_CHAR) {
        char *tmp;
        tmp = (char *) malloc(att->len + 1);
        if ((retval = nc_get_att_text(ncid, varid, att->name, tmp))) {
            ERR(retval);
        }
        tmp[att->len] = '\0';
        att->value = tmp;
    } else {
        att->value = malloc(size_of_nc_type(att->type) * att->len);
        if ((retval = nc_get_att(ncid, varid, att->name, att->value))) {
            ERR(retval);
        }
    }

    return att;
}

/*
 * Function:  file_has_att
 * --------------------
 * checks global attributes in an ncfile_t struct to see if it contains a
 * specific attribute
 *
 *  file (ncfile_t*): populated ncfile_t struct (e.g. from open_ncdf())
 *  name (const char*): name of the attribute to look up
 *
 *  returns: int describing either the id of the attribute in the file or -1 if
 *           there is no such attribute.
 */
int
file_has_att(ncfile_t *file, const char *name) {
    int i;
    for (i = 0; i < file->ngatts; ++i) {
        if (strcmp(file->gatts[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

/*
 * Function:  file_has_dim
 * --------------------
 * checks dimensions in an ncfile_t struct by name to see if it contains a
 * specific dimension
 *
 *  file (ncfile_t*): populated ncfile_t struct (e.g. from open_ncdf())
 *  name (const char*): name of the dimension to look up
 *
 *  returns: int describing either the id of the dimension in the file or -1 if
 *           there is no such dimension.
 */
int
file_has_dim(ncfile_t *file, const char *name) {
    int i;
    for (i = 0; i < file->ndims; ++i) {
        if (strcmp(file->dims[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

/*
 * Function:  file_has_var
 * --------------------
 * checks variables in an ncfile_t struct by name to see if it contains a
 * specific variable
 *
 *  file (ncfile_t*): populated ncfile_t struct (e.g. from open_ncdf())
 *  name (const char*): name of the variable to look up
 *
 *  returns: int describing either the id of the variable in the file or -1 if
 *           there is no such variable.
 */
int
file_has_var(ncfile_t *file, const char *name) {
    int i;
    for (i = 0; i < file->nvars; ++i) {
        if (strcmp(file->vars[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}


/*
 * Function:  var_has_attr
 * --------------------
 * checks attributes in an ncvar_t struct by name to see if it contains a
 * specific attribute
 *
 *  var (ncvar_t*): populated ncvar_t struct
 *  name (const char*): name of the attribute to look up
 *
 *  returns: int describing either the id of the attribute in the file or -1 if
 *           there is no such attribute.
 */
int
var_has_attr(ncvar_t *var, const char *name) {
    int i;
    for (i = 0; i < var->numatts; ++i) {
        if (strcmp(var->atts[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

/*
 * Function:  var_has_dim
 * --------------------
 * checks dimensions in an ncvar_t struct by name to see if it contains a
 * specific dimension
 *
 *  var (ncvar_t*): populated ncvar_t struct
 *  name (const char*): name of the dimension to look up
 *
 *  returns: int describing either the id of the dimension in the file or -1 if
 *           there is no such dimension.
 */
int
var_has_dim(ncvar_t *var, const char *name) {
    int i;
    for (i = 0; i < var->ndims; ++i) {
        if (strcmp(var->dims[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

/*
 * Function:  get_var_att_by_name
 * --------------------
 * get an attribute from a variable by name
 *
 *  var (ncvar_t*): populated ncvar_t struct
 *  name (const char*): name of the variable to get
 *
 *  returns: pointer to an ncatt_t struct if the attribute is found, else NULL.
 */
ncatt_t *
get_var_att_by_name(ncvar_t *var, const char *name) {
    int i;
    i = var_has_attr(var, name);
    if (i > -1) {
        return &var->atts[i];
    }
    return NULL;
}

/*
 * Function:  get_file_dim_by_name
 * --------------------
 * get an dimension from a file by name
 *
 *  file (ncfile_t*): populated ncfile_t struct
 *  name (const char*): name of the dimension to get
 *
 *  returns: pointer to an ncdim_t struct if the dimension is found, else NULL.
 */
ncdim_t *
get_file_dim_by_name(ncfile_t *file, const char *name) {
    int i;
    for (i = 0; i < file->ndims; ++i) {
        if (strcmp(file->dims[i].name, name) == 0) {
            return &file->dims[i];
        }
    }
    return NULL;
}

/*
 * Function: print_ncdf_info
 * --------------------
 * prints file info in a (somewhat) nicely formatted and structured way.
 * similar to ncdump -h
 *
 *  ncfile (ncfile_t*): populated ncfile_t struct
 *
 *  returns: void
 */
void
print_ncdf_info(ncfile_t *ncfile) {
    int i;

    printf("Dimensions:\n");
    for (i = 0; i < ncfile->ndims; ++i) {
        if (ncfile->dims[i].unlimited) {
            printf("\t%d %s: (size: %lu) (Record dimension)\n", ncfile->dims[i].index, ncfile->dims[i].name,
                   ncfile->dims[i].size);
        } else {
            printf("\t%d %s: (size: %lu)\n", ncfile->dims[i].index, ncfile->dims[i].name, ncfile->dims[i].size);
        }
    }

    if (ncfile->nvars > 0) {
        printf("Global Variables: (%d)\n", ncfile->nvars);
        for (i = 0; i < ncfile->nvars; ++i) {
            print_var_info(&ncfile->vars[i], 1);
        }
    }

    if (ncfile->ngroups > 0) {
        printf("Groups: (%d)\n", ncfile->ngroups);
        for (i = 0; i < ncfile->ngroups; ++i) {
            print_group_info(&ncfile->groups[i], 1);
        }
    }

    if (ncfile->ngatts > 0) {
        printf("Global attributes:\n");
        for (i = 0; i < ncfile->ngatts; ++i) {
            print_att_info(&ncfile->gatts[i], 1);
        }
    }
} // end function

/*
 * Function: print_var_info
 * --------------------
 * prints variable info in a (somewhat) nicely formatted and structured way.
 *
 *  var (ncvar_t*): populated ncvar_t struct
 *  indentlvl (int): number of indents (4 spaces) as a base for this block of text
 *
 *  returns: void
 */
void print_var_info(ncvar_t *var, int indentlvl) {
    int i;

    printf("%*s", indentlvl * 4, " ");
    printf("%s %s", prim_type_name(var->type), var->name);

    // print dimensionality in parentheses
    if (var->ndims == 1) {
        printf("(%s):\n", var->dims[0].name);
    } else {
        for (i = 0; i < var->ndims; ++i) {
            if (i == 0) {
                printf("(%s", var->dims[i].name);
            } else if (i == (var->ndims - 1)) {
                printf(", %s):\n", var->dims[i].name);
            } else {
                printf(", %s", var->dims[i].name);
            } // end if
        } // end loop
    } // end if

    //print variable specific attributes
    if (var->numatts > 0) {
        for (i = 0; i < var->numatts; ++i) {
            printf("%*s", (indentlvl + 1) * 4, " ");
            printf("%s: ", var->atts[i].name);
            switch (var->atts[i].type) {
                case NC_INT:
                    printf("%d\n", *(int32_t *) var->atts[i].value);
                    break;
                case NC_UINT:
                    printf("%d\n", *(uint32_t *) var->atts[i].value);
                    break;
                case NC_UINT64:
                    printf("%" PRIu64 "\n", *(uint64_t *) var->atts[i].value);
                    break;
                case NC_INT64:
                    printf("%" PRId64 "\n", *(int64_t *) var->atts[i].value);
                    break;
                case NC_SHORT:
                    printf("%d\n", *(int16_t *) var->atts[i].value);
                    break;
                case NC_FLOAT:
                    printf("%f\n", *(float *) var->atts[i].value);
                    break;
                case NC_DOUBLE:
                    printf("%e\n", *(double *) var->atts[i].value);
                    break;
                case NC_BYTE:
                    printf("0x%02X\n", *(int8_t *) var->atts[i].value);
                    break;
                case NC_UBYTE:
                    printf("0x%02X\n", *(uint8_t *) var->atts[i].value);
                    break;
                case NC_CHAR:
                    printf("%s\n", (char *) var->atts[i].value);
                    break;
                default:
                    abort();
            } // end switch statement
        } // end attribute loop
    } // end if

    // print storage description attributes

    if (var->deflate) {
        printf("%*s", (indentlvl + 1) * 4, " ");
        printf("shuffle: %d\n", var->shuffle);
        printf("%*s", (indentlvl + 1) * 4, " ");
        printf("complevel: %d\n", var->deflate_level);
    }

    if (var->chunking == NC_CHUNKED) {
        printf("%*s", (indentlvl + 1) * 4, " ");
        printf("Chunksizes: ");
        if (var->ndims == 1) {
            printf("(%zu)\n", var->chunksizes[0]);
        } else {
            for (i = 0; i < var->ndims; ++i) {
                if (i == 0) {
                    printf("(%zu", var->chunksizes[i]);
                } else if (i == var->ndims - 1) {
                    printf(", %zu)\n", var->chunksizes[i]);
                } else {
                    printf(", %zu", var->chunksizes[i]);
                } // end if
            } // dim loop
        } // end if
    }
}

/*
 * Function: print_group_info
 * --------------------
 * prints group info in a (somewhat) nicely formatted and structured way.
 *
 *  var (ncgroup_t*): populated ncgroup_t struct
 *  indentlvl (int): base indent level (n * 4 spaces)
 *
 *  returns: void
 */
void
print_group_info(ncgroup_t *group, int indentlvl) {
    int i;

    printf("%*s", indentlvl * 4, " ");
    printf("%s: (vars: %d) (atts: %d)\n", group->name, group->nvars, group->natts);

    if (group->nvars > 0) {
        for (i = 0; i < group->nvars; ++i) {
            print_var_info(&group->vars[i], indentlvl + 1);
        }
    }

    if (group->nsubgroups > 0) {
        for (i = 0; i < group->nsubgroups; ++i) {
            print_group_info(group->subgroups + i, indentlvl + 1);
        }
    }
}

/*
 * Function: print_att_info
 * --------------------
 * prints attribute names/values in a (somewhat) nicely formatted and structured way.
 *
 *  var (ncatt_t*): populated ncatt_t struct
 *  indentlvl (int): base indent level (n * 4 spaces)
 *
 *  returns: void
 */
void
print_att_info(ncatt_t *att, int indentlvl) {
    printf("%*s", (indentlvl + 1) * 4, " ");
    printf("%s: ", att->name);
    switch (att->type) {
        case NC_INT:
            printf("%d\n", *(int32_t *) att->value);
            break;
        case NC_UINT:
            printf("%d\n", *(uint32_t *) att->value);
            break;
        case NC_UINT64:
            printf("%" PRIu64 "\n", *(uint64_t *) att->value);
            break;
        case NC_INT64:
            printf("%" PRId64 "\n", *(int64_t *) att->value);
            break;
        case NC_SHORT:
            printf("%d\n", *(int16_t *) att->value);
            break;
        case NC_FLOAT:
            printf("%f\n", *(float *) att->value);
            break;
        case NC_DOUBLE:
            printf("%e\n", *(double *) att->value);
            break;
        case NC_BYTE:
            printf("0x%02X\n", *(int8_t *) att->value);
            break;
        case NC_UBYTE:
            printf("0x%02X\n", *(uint8_t *) att->value);
            break;
        case NC_CHAR:
            printf("%s\n", (char *) att->value);
            break;
        default:
            abort();
    } // end switch statement

}

/*
 * Function: init_empty_template
 * --------------------
 * initialises empty ncfile_t struct for use as a virtual file template
 *
 *  returns: pointer to a zero/null initialised ncfile_t struct
 */
ncfile_t *init_empty_template() {

    ncfile_t *tpl = (ncfile_t *) malloc(sizeof(ncfile_t));

    tpl->file_handle = 0;
    tpl->ngroups = 0;
    tpl->groups = NULL;
    tpl->ndims = 0;
    tpl->dims = NULL;
    tpl->nvars = 0;
    tpl->vars = NULL;
    tpl->ngatts = 0;
    tpl->gatts = NULL;
    tpl->nunlimdims = 0;
    tpl->ncformat = 0;

    return tpl;
}

/*
 * Function: update_template_ids
 * --------------------
 * recursively updates the ids of the dimensions contained in ncvar_t structs
 * to reflect changes in the overall file structure
 *
 *  tpl (ncfile_t *): ncfile_t template to update
 *
 *  returns:
 */
void
update_template_ids(ncfile_t *tpl) {
    int i, j, idx;

    for (i = 0; i < tpl->nvars; ++i) {
        for (j = 0; j < tpl->vars[i].ndims; ++j) {
            idx = file_has_dim(tpl, tpl->vars[i].dims[j].name);
            if (tpl->vars[i].dims[j].index != idx) {
                tpl->vars[i].dims[j].index = idx;
            }
        }
    }
}

int add_att_to_template(ncfile_t *tpl, ncatt_t *att) {

    int i;
    if (file_has_att(tpl, att->name) > 0) {
        return 0;
    }

    ncatt_t tmp;

    strncpy(tmp.name, att->name, NC_MAX_NAME + 1);
    tmp.type = att->type;
    tmp.len = att->len;
    tmp.value = malloc(size_of_nc_type(tmp.type) * tmp.len);
    tmp.value = att->value;

    tpl->ngatts += 1;
    ncatt_t *new_atts = (ncatt_t *) malloc(sizeof(ncatt_t) * tpl->ngatts);

    //copy over existing dimensions
    for (i = 0; i < tpl->ngatts - 1; ++i) {
        new_atts[i] = tpl->gatts[i];
    }

    new_atts[tpl->ngatts - 1] = tmp;
    free(tpl->gatts);
    tpl->gatts = new_atts;
    return 1;
}

int add_dim_to_template(ncfile_t *tpl, const char *name, size_t size) {

    int i;
    // check if dim is already in template
    if (file_has_dim(tpl, name) > 0) {
        return 0;
    }

    // if not, add the dim (simples)
    ncdim_t tmp;
    tmp.index = -999;
    strncpy(tmp.name, name, NC_MAX_NAME + 1);
    tmp.size = size;

    tpl->ndims += 1;
    ncdim_t *new_dims = (ncdim_t *) malloc(sizeof(ncdim_t) * tpl->ndims);

    // copy over existing dimensions
    for (i = 0; i < tpl->ndims - 1; ++i) {
        new_dims[i] = tpl->dims[i];
    }

    new_dims[tpl->ndims - 1] = tmp;
    free(tpl->dims);
    tpl->dims = new_dims;

    return 1;
}

int
add_var_to_template(ncfile_t *tpl, ncvar_t *var, const char *sin_dim) {
    int i;
    // check if var is already in tempalte
    if (file_has_var(tpl, var->name) > 0) {
        return 0;
    }

    ncvar_t tmp;

    // copy over everything that isn't affected by changes in dimension
    tmp.index = var->index;
    strncpy(tmp.name, var->name, NC_MAX_NAME + 1);
    tmp.type = var->type;
    tmp.shuffle = var->shuffle;
    tmp.deflate = var->deflate;
    tmp.deflate_level = var->deflate_level;
    tmp.f32checksum = var->f32checksum; // double check exactly what this is a checksum of. may need recalc

    tmp.numatts = var->numatts;
    tmp.atts = (ncatt_t *) malloc(sizeof(ncatt_t) * tmp.numatts);
    tmp.atts = var->atts;

    tmp.fill_info = (ncfill_t *) malloc(sizeof(ncfill_t));
    tmp.fill_info->no_fill = var->fill_info->no_fill;
    tmp.fill_info->size = var->fill_info->size;
    tmp.fill_info->value = var->fill_info->value;

    // dimensions
    if (var_has_dim(var, sin_dim) > 0) {
        tmp.ndims = var->ndims + 1;
        tmp.dims = (ncdim_t *) malloc(sizeof(ncdim_t) * tmp.ndims);
        for (i = 0; i < var->ndims - 1; ++i) {
            if (strcmp(var->dims[i].name, sin_dim) == 0) {
                printf("Feck... %s isn't then last dimension when it should be\n", sin_dim);
                return -1;
            } else {
                tmp.dims[i] = *get_file_dim_by_name(tpl, var->dims[i].name);
            }
        }

        tmp.dims[var->ndims - 1] = *get_file_dim_by_name(tpl, "lat");
        tmp.dims[var->ndims] = *get_file_dim_by_name(tpl, "lon");
    } else {
        tmp.ndims = var->ndims;
        tmp.dims = (ncdim_t *) malloc(sizeof(ncdim_t) * tmp.ndims);
        for (i = 0; i < var->ndims; ++i) {
            tmp.dims[i] = *get_file_dim_by_name(tpl, var->dims[i].name);
        }
    }
    // chunksizes
    // hardcoded to 256 along lat/lon atm as this should remain consistent across resolutions
    if (var->chunking == NC_CONTIGUOUS) {
        // check that the variable even uses chunking
        tmp.chunking = NC_CONTIGUOUS;
        tmp.chunksizes = NULL;
    } else {
        // allocate memory for chunksizes, number of dims will have already been decided above
        tmp.chunking = var->chunking;
        tmp.chunksizes = (size_t *) malloc(sizeof(size_t) * tmp.ndims);

        // if the variable uses chunking and is being remapped from sin->geo, then update chunksizes
        // to reflect changes to the dimensionality i.e. (time, bin_index) -> (time, lat, lon)
        if (var_has_dim(var, sin_dim) > 0) {
            for (i = 0; i < var->ndims - 1; ++i) {
                tmp.chunksizes[i] = var->chunksizes[i];
            }
            tmp.chunksizes[var->ndims - 1] = 256;
            tmp.chunksizes[var->ndims] = 256;
        } else {
            // otherwise just perform a straight copy of the chunksizes.
            for (i = 0; i < var->ndims; ++i) {
                tmp.chunksizes[i] = var->chunksizes[i];
            }
        }
    }

    // re-allocate variable array with n+1 variables
    tpl->nvars += 1;
    ncvar_t *new_vars = (ncvar_t *) malloc(sizeof(ncvar_t) * tpl->nvars);

    // copy all of the existing variables across first
    for (i = 0; i < tpl->nvars - 1; ++i) {
        new_vars[i] = tpl->vars[i];
    }
    // then copy the new variable in the final position
    new_vars[tpl->nvars - 1] = tmp;

    free(tpl->vars);
    tpl->vars = new_vars;

    return 1;

}

int
add_coord_var_to_template(ncfile_t *tpl, ncvar_t *var, const char *sin_dim) {

    int i;
    // check if var is already in template
    if (file_has_var(tpl, var->name) > 0) {
        return 0;
    }

    ncvar_t tmp;

    // copy over everything that isn't affected by changes in dimension
    tmp.index = var->index;
    strncpy(tmp.name, var->name, NC_MAX_NAME + 1);
    tmp.type = var->type;
    tmp.shuffle = var->shuffle;
    tmp.deflate = var->deflate;
    tmp.deflate_level = var->deflate_level;
    tmp.f32checksum = var->f32checksum; // double check exactly what this is a checksum of. may need recalc

    tmp.numatts = var->numatts;
    tmp.atts = (ncatt_t *) malloc(sizeof(ncatt_t) * tmp.numatts);
    for (i = 0; i < var->numatts; ++i) {
        tmp.atts[i] = var->atts[i];
    }

    tmp.fill_info = (ncfill_t *) malloc(sizeof(ncfill_t));
    tmp.fill_info->no_fill = var->fill_info->no_fill;
    tmp.fill_info->size = var->fill_info->size;
    tmp.fill_info->value = var->fill_info->value;


    tpl->nvars += 1;
    ncvar_t *new_vars = (ncvar_t *) malloc(sizeof(ncvar_t) * tpl->nvars);

    // dimensions
    // all coordinate variables in a geo file should have 1 dimension of the same name
    if (var->ndims > 1) {
        printf("Houston we have too many variables in coordinate var: %s\n", var->name);
        return 0;
    } else {
        tmp.ndims = var->ndims;
        tmp.dims = (ncdim_t *) malloc(sizeof(ncdim_t));
        if (strcmp(var->dims[0].name, sin_dim) == 0) {
            tmp.dims[0] = *get_file_dim_by_name(tpl, var->name);
        }
    }

    // chunksizes
    // for the coordinate variables (since they're only 1 dimension) the chunksize has been set
    // to the same size as the coordinate
    tmp.chunking = NC_CHUNKED;
    tmp.chunksizes = (size_t *) malloc(sizeof(size_t));
    tmp.chunksizes[0] = tmp.dims[0].size;

    // copy all existsing variables across first
    for (i = 0; i < tpl->nvars - 1; ++i) {
        new_vars[i] = tpl->vars[i];
    }

    // then copy the new variable in the final position
    new_vars[tpl->nvars - 1] = tmp;
    free(tpl->vars);
    tpl->vars = new_vars;

    return 1;
}

int
write_template_to_file(ncfile_t *tpl, const char *filename, int write_mode) {
    int i, j;
    int retval;
    if ((retval = nc_set_default_format(tpl->ncformat, NULL))) {ERR(retval); }
    if ((retval = nc_create(filename, write_mode, &tpl->file_handle))) {ERR(retval); }

    // WRITE DIMENSIONS TO FILE
    ncdim_t tmpd;
    for (i = 0; i < tpl->ndims; ++i) {
        tmpd = tpl->dims[i];
        if ((retval = nc_def_dim(tpl->file_handle, tmpd.name, tmpd.size, &tmpd.index))) {ERR(retval); }
        tpl->dims[i] = tmpd;
    }

    //update template variables with new dimension ids
    update_template_ids(tpl);

    //printf("Writing template to file... info:\n");
    //print_ncdf_info(tpl);

    // WRITE VARIABLES TO FILE
    ncvar_t tmpv;
    ncatt_t tmpva;
    for (i = 0; i < tpl->nvars; ++i) {
        tmpv = tpl->vars[i];
        if (tmpv.ndims > 0) {
            int dimids[tmpv.ndims];
            for (j = 0; j < tmpv.ndims; ++j) {
                dimids[j] = tmpv.dims[j].index;
            }
            if ((retval = nc_def_var(tpl->file_handle, tmpv.name, tmpv.type, tmpv.ndims, dimids, &tmpv.index))) {
                printf("name: %s\n", tmpv.name);
                printf("type: %s\n", prim_type_name(tmpv.type));
                for (j = 0; j < tmpv.ndims; ++j) {
                    printf("dim %d: %s\n", j, tmpv.dims[j].name);
                }
                ERR(retval);
            }
        } else {
            if ((retval = nc_def_var(tpl->file_handle, tmpv.name, tmpv.type, tmpv.ndims, NULL, &tmpv.index))) {
                ERR(retval);
            }
        }

        // handle variable attributes
        for (j = 0; j < tpl->vars[i].numatts; ++j) {
            tmpva = tpl->vars[i].atts[j];
            if ((retval = nc_put_att(tpl->file_handle, tmpv.index, tmpva.name, tmpva.type, tmpva.len,
                                     tmpva.value))) {ERR(retval); }
        }

        // handle fill value
        if ((retval = nc_def_var_fill(tpl->file_handle, tmpv.index, tmpv.fill_info->no_fill,
                                      tmpv.fill_info->value))) {ERR(retval); }

        // handle chunking
        if ((retval = nc_def_var_chunking(tpl->file_handle, tmpv.index, tmpv.chunking, tmpv.chunksizes))) {
            ERR(retval);
        }

        // handle compression
        if ((retval = nc_def_var_deflate(tpl->file_handle, tmpv.index, tmpv.shuffle, tmpv.deflate,
                                         tmpv.deflate_level))) {ERR(retval); }

        // handle fletcher32
        if ((retval = nc_def_var_fletcher32(tpl->file_handle, tmpv.index, tmpv.f32checksum))) {ERR(retval); }

        tpl->vars[i] = tmpv;
    }

    // WRITE GROUPS TO FILE
    // TODO(James): do this. (make write group template to file method and run it recursively)

    // WRITE GLOBAL ATTS TO FILE
    ncatt_t tmpa;
    for (i = 0; i < tpl->ngatts; ++i) {
        tmpa = tpl->gatts[i];
        if ((retval = nc_put_att(tpl->file_handle, NC_GLOBAL, tmpa.name, tmpa.type, tmpa.len, tmpa.value))) {
            ERR(retval);
        }
    }

    if ((retval = nc_enddef(tpl->file_handle))) {ERR(retval); }

    return 1;
}

int write_var_to_file(ncfile_t *file, int varid, void *arr) {
    int retval;
    if ((retval = nc_put_var(file->file_handle, varid, arr))) {ERR(retval); }
    return 1;
}

int copy_var_to_file(ncfile_t *input_file, int input_var_id, ncfile_t *output_file, int output_var_id) {
    //TODO(James): Modify this code to be chunked based on the chunksizes set in the input file
    ncvar_t invar = input_file->vars[input_var_id];
    int i;
    int num_elements = 0;
    for (i = 0; i < invar.ndims; ++i) {
        num_elements += invar.dims[i].size;
    }

    void *inp;
    inp = malloc(size_of_nc_type(invar.type) * num_elements);

    int retval;
    if ((retval = nc_get_var(input_file->file_handle, input_var_id, inp))) {ERR(retval); }

    if ((retval = nc_put_var(output_file->file_handle, output_var_id, inp))) {ERR(retval); }

    free(inp);

    return 1;
}

void
free_group(ncgroup_t *group) {

    int i;
    for (i = 0; i < group->nsubgroups; ++i) {
        free_group(&group->subgroups[i]);
    }
    free(group->subgroups);


    for (i = 0; i < group->natts; ++i) {
        free(group->atts[i].value);
    }

    free(group->dims);
    free(group->vars);
    free(group->atts);
}

void
free_var(ncvar_t *var) {
    int i;

    free(var->fill_info->value);
    free(var->fill_info);

    free(var->dims);

    free(var->chunksizes);

    for (i = 0; i < var->numatts; ++i) {
        free(var->atts[i].value);
    }

    free(var->atts);
}

void
free_ncfile(ncfile_t *ncfile) {

    int i;

    if (ncfile->nvars > 0) {
        for (i = 0; i < ncfile->nvars; ++i) {
            free_var(&ncfile->vars[i]);
        }
    }

    if (ncfile->ngroups > 0) {
        for (i = 0; i < ncfile->ngroups; ++i) {
            free_group(&ncfile->groups[i]);
        }
    }

    free(ncfile->vars);
    free(ncfile->dims);
    free(ncfile->groups);

}

int
sin_rows_from_len(uint64_t len) {
    switch (len) {
        case 371272:
            return 540;
        case 1485108:
            return 1080;
        case 5940422:
            return 2160;
        case 23761676:
            return 4320;
        case 95046858:
            return 8640;
        case 380187130:
            return 17280;
        case 1520749130:
            return 34560;
        case 6082996640:
            return 69120;
        case 24331986784:
            return 138240;
        case 97327946460:
            return 276480;
        default:
            return 0;
    }
}

uint64_t
sin_len_from_rows(int len) {
    switch (len) {
        case 540:
            return 371272;
        case 1080:
            return 1485108;
        case 2160:
            return 5940422;
        case 4320:
            return 23761676;
        case 8640:
            return 95046858;
        case 17280:
            return 380187130;
        case 34560:
            return 1520749130;
        case 69120:
            return 6082996640;
        case 138240:
            return 24331986784;
        case 276480:
            return 97327946460;
        default:
            return 0;
    }
}
