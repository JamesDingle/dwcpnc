#include "array_product.h"
#include "vbn.h"

//int main(int argc, char *argv[]) {
//
//    int i, j;
//
//    ca_array_t test;
//    test.ndims = 3;
//
//    test.chunk_arrays = (chunk_array_t**)malloc(sizeof(chunk_array_t*) * test.ndims);
//
//
//    test.chunk_arrays[0] = gen_chunks(10,3);
//    test.chunk_arrays[1] = gen_chunks(10,3);
//    test.chunk_arrays[2] = gen_chunks(10,3);
////    test.chunk_arrays[3] = gen_chunks(2,1);
////    test.chunk_arrays[2] = gen_chunks(13,2);
//
//
//    vbn_t *vbn;
//
//    vbn = init_vbn(test.ndims);
//
//    for (i = 0; i < test.ndims; ++i) {
//        vbn->digit_max[i] = test.chunk_arrays[i]->count - 1;
//    }
//
//    int indices[4] = {9999,1,1,1};
//    int extents[4] = {10000,1000,100,10};
//    int index = nd_to_1d(indices, extents, 4);
//    printf("1d index: %d\n", index);
//
//    i = 0;
//    do {
//        printf("%03d: ", i);
//        for (j = 0; j < vbn->digit_count; ++j) {
//            print_chunk(*test.chunk_arrays[j]->chunks[vbn->digit_value[j]]);
//            printf(", ");
////            print_vbn(vbn);
//        }
//
//        printf("\n");
//        ++i;
//    } while(increment_vbn(vbn));
//////
//////    free(ca1);
//////    free(ca2);
//////    free(ca3);
//
//    return 0;
//
//}

//chunk_array_t *
//gen_chunks(int total_length, int chunk_length) {
//
////    printf("Total Length: %d\n", total_length);
////    printf("Chunk Length: %d\n", chunk_length);
//
//    int whole_chunks;
//    whole_chunks = total_length / chunk_length;
////    printf("Whole chunks: %d\n", whole_chunks);
//
//    int partial_chunks;
//    partial_chunks = total_length % chunk_length ? 1 : 0;
////    printf("Partial chunks: %d\n", partial_chunks);
//
//    chunk_array_t *chunk_array;
//    chunk_array = (chunk_array_t*)malloc(sizeof(chunk_array_t));
//
//    chunk_array->count = whole_chunks + partial_chunks;
//
//    chunk_array->chunks = (chunk_t**)malloc(sizeof(chunk_t*) * (chunk_array->count));
//
//    int i;
//    for (i=0; i < whole_chunks; ++i) {
//        chunk_t *chunk = (chunk_t*)malloc(sizeof(chunk_t));
//        chunk->start = i * chunk_length;
//        chunk->end = i * chunk_length + chunk_length - 1;
//        chunk_array->chunks[i] = chunk;
////        print_chunk(*chunk_array->chunks[i]);
//    }
//
//    if (partial_chunks == 1) {
//        i = whole_chunks;
//        chunk_t *chunk = (chunk_t*)malloc(sizeof(chunk_t));
//        chunk->start = i * chunk_length;
//        chunk->end = total_length - 1;
//        chunk_array->chunks[i] = chunk;
////        print_chunk(*chunk_array->chunks[i]);
//    }
//
//    return chunk_array;
//
//}
//
//int add_chunk_to_slice(chunk_t *chunk, slice_t *slice) {
//
//    return 1;
//}
//
//void print_chunk(chunk_t chunk) {
//    printf("(%d → %d)", chunk.start, chunk.end);
//}
//
//void print_ca_array_from_vbn(ca_array_t* ca_array, vbn_t *vbn) {
//    int j;
//    for (j = 0; j < vbn->digit_count; ++j) {
//        print_chunk(*ca_array->chunk_arrays[j]->chunks[vbn->digit_value[j]]);
//        printf(", ");
//    }
//}
//
//int nd_to_1d(int* indices, int* extents, int count) {
//    int index = 0;
//
//    int i,j;
//    int tmp = 0;
//
//    for (i = 0; i < count; ++i) {
//        tmp = indices[i];
//
//        if (count - i > 0) {
//            for (j = i+1; j < count; ++j) {
//                tmp *= extents[j];
//            }
//            index += tmp;
//        } else {
//            index += tmp;
//        }
//    }
//
//    return index;
//}