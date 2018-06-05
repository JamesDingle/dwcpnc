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
    args->scale_info = NULL;

    while ((opt = getopt(argc, argv, ":di:c:j:")) != -1) {
        switch (opt) {
            case 'd':
                args->debug = 1;
                break;
            case 'i':
                args->infile = (optarg);
                break;
            case 'c':
                args->scale_info = parse_chunk_arg(optarg);
                break;
            case 'j':
                args->nthreads = atoi(optarg);
                break;
            default:
                break;
        }
    }

    return args;

}

chunk_info_t *parse_chunk_arg(char *chunk_string) {

    assert(chunk_string != NULL);

    int comma_count, delim_count;

    comma_count = how_many_of_char_in_str(",", chunk_string);

    // allocate the struct to store these pairings
    chunk_info_t *chunk_info;
    chunk_info = (chunk_info_t *) malloc(sizeof(chunk_info_t));

    char *token;

    // if we have no delimiters, we must only be scaling 1 dimension, else error
    if (comma_count == 0) {

        delim_count = how_many_of_char_in_str("=", chunk_string);

        if (delim_count == 1) {
            chunk_def_t *dim_scale = (chunk_def_t *) malloc(sizeof(chunk_def_t));
            dim_scale->dim_name = strsep(&chunk_string, "=");
            dim_scale->chunk_length = atoi(strsep(&chunk_string, "="));

            chunk_info->count = 1;
            chunk_info->dim_chunks = (chunk_def_t *) malloc(sizeof(chunk_def_t));
            chunk_info->dim_chunks = dim_scale;
        } else {
            return NULL;
        }

    } else {
        chunk_info->count = comma_count + 1;
        chunk_info->dim_chunks = (chunk_def_t *) malloc(sizeof(chunk_def_t) * comma_count + 1);

        int i = 0;
        while ((token = strsep(&chunk_string, ",")) != NULL && (i <= comma_count)) {
            token = strip(token);
            delim_count = how_many_of_char_in_str("=", token);

            if (delim_count > 0) {
                chunk_info->dim_chunks[i].dim_name = strip(strsep(&token, "="));
                chunk_info->dim_chunks[i].chunk_length = atoi(strip(strsep(&token, "=")));
                ++i;
            } else {
                return NULL;
            }
        }
    }
    return chunk_info;
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