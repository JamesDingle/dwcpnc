//
// Created by jad on 31/05/18.
//

#include "args.h"

args_t *parse_args(int argc, char **argv) {

    int opt;

    args_t *args;
    args = (args_t *) malloc(sizeof(args_t));

    args->debug = 0;
    args->overwrite = 0;
    args->infile = "";

    while ((opt = getopt(argc, argv, ":dwi:o:s:")) != -1) {
        switch (opt) {
            case 'd':
                args->debug = 1;
                break;
            case 'w':
                args->overwrite = 1;
                break;
            case 'i':
                args->infile = (optarg);
                break;
            case 's':
                args->scale_info = parse_scale_arg(optarg);
                break;
            default:
                break;
        }
    }

    return args;

}

scale_info_t *parse_scale_arg(char *scale_string) {

    assert(scale_string != NULL);

    int comma_count, delim_count;

    comma_count = how_many_of_char_in_str(",", scale_string);

    // allocate the struct to store these pairings
    scale_info_t *scale_info;
    scale_info = (scale_info_t *) malloc(sizeof(scale_info_t));

    char *token;

    // if we have no delimiters, we must only be scaling 1 dimension, else error
    if (comma_count == 0) {

        delim_count = how_many_of_char_in_str("=", scale_string);

        if (delim_count == 1) {
            dim_scale_t *dim_scale = (dim_scale_t *) malloc(sizeof(dim_scale_t));
            dim_scale->dim_name = strsep(&scale_string, "=");
            dim_scale->scale_factor = atoi(strsep(&scale_string, "="));

            scale_info->count = 1;
            scale_info->scaled_dims = (dim_scale_t *) malloc(sizeof(dim_scale_t));
            scale_info->scaled_dims = dim_scale;
        } else {
            return NULL;
        }

    } else {
        scale_info->count = comma_count + 1;
        scale_info->scaled_dims = (dim_scale_t *) malloc(sizeof(dim_scale_t) * comma_count + 1);

        int i = 0;
        while ((token = strsep(&scale_string, ",")) != NULL && (i <= comma_count)) {
            token = strip(token);
            delim_count = how_many_of_char_in_str("=", token);

            if (delim_count > 0) {
                scale_info->scaled_dims[i].dim_name = strip(strsep(&token, "="));
                scale_info->scaled_dims[i].scale_factor = atoi(strip(strsep(&token, "=")));
                ++i;
            } else {
                return NULL;
            }
        }
    }
    return scale_info;
}

int validate_args(args_t *args) {

    if (args->debug) {
        printf("Debug information enabled.\n");
        if (args->overwrite) {
            printf("Output file overwriting enabled.\n");
        }
    }

    if (args->infile == "") {
        fprintf(stderr, "No input file specified.\n");
        return 0;
    }

    if (access(args->infile, F_OK) == -1) {
        fprintf(stderr, "Specified input file (%s) does not exist.\n", args->infile);
        return 0;
    }

    printf("Args validated\n");
    return 1;

}

int how_many_of_char_in_str(const char *chr, char *str) {

    // first, count the occurrences of the delimiter ,
    int i, count;
    for (i = 0, count = 0; str[i]; i++) {
        count += (str[i] == *chr);
    }

    return count;
}

char *lstrip(char *str) {
    // determine input length
    size_t il;
    il = strlen(str);

    // count up to first valid character
    int first = 0;
    while (isspace(str[first]) && first < (int) il) {
        ++first;
    }

    // allocate from first valid character to end, then copy
    char *ret_str = malloc(il - (size_t) first);
    memcpy(ret_str, &str[first], il - (size_t) first);

    // return
    return ret_str;
}

char *rstrip(char *str) {
    // determine input length
    size_t il;
    il = strlen(str);

    // loop until we find a space, then keep going until
    // we either find a valid char or \0
    int i, j;
    for (i = 0; i < (int) il; ++i) {
        j = i;
        while (isspace(str[j])) {
            if (j == (int) il - 1) {
                // in which case, i is the start of the whitespace, j is the end (+1 for \0 terminator)
                char *ret_str = malloc((size_t) i + 1);
                memcpy(ret_str, str, (size_t) i);
                ret_str[i] = 0;
                return ret_str;
            } else {
                ++j;
            }
        }
    }
    return str;
}

char *strip(char *str) {
    return lstrip(rstrip(str));
}