#include <stdlib.h>
#include "tablelibdynamic.h"

//funkcja pomocnicza

int abs (int x){
    if(x < 0)
        return (-1) * x;
    return x;
}

//funkcje dla tablicy z dynamicznym lokowaniem pamieci

table_dynamic * create_dynamic (int number_of_blocks, int block_size){
    table_dynamic * t = malloc(sizeof(table_dynamic));
    t -> number_of_blocks = number_of_blocks;
    t -> block_size = block_size;
    t -> tab = calloc(number_of_blocks, sizeof(char*));
    t -> len = calloc(number_of_blocks, sizeof(int));
    t -> sum = calloc(number_of_blocks, sizeof(int));
    return t;
}

void delete_dynamic (table_dynamic * t){
    free(t -> sum);
    free(t -> len);
    for(int i = 0; i < t -> number_of_blocks; i++)
        if(t -> tab[i] != NULL)
            free(t -> tab[i]);
    free(t -> tab); 
    free(t);
}

void add_block_dynamic (table_dynamic * t, int index, char * block, int block_size){
    if(t -> number_of_blocks <= index) return;
    if(t -> block_size < block_size) return;
    if(t -> tab[index] != NULL) free(t -> tab[index]);

    t -> tab[index] = calloc(block_size, sizeof(char));
    t -> len[index] = block_size;
    int sum_of_characters = 0;
    for(int i = 0; i < block_size; i++){
        t -> tab[index][i] = block[i];
        sum_of_characters += block[i];
    }
    t -> sum[index] = sum_of_characters;
}

void delete_block_dynamic (table_dynamic * t, int index){
    if(t -> number_of_blocks <= index) return;
    if(t -> tab[index] == NULL) return;

    free(t -> tab[index]);
    t -> tab[index] = NULL;
    t -> len[index] = 0;
    t -> sum[index] = 0; 
}

int find_block_with_similar_sum_dynamic (table_dynamic * t, int index){
    if (index >= t -> number_of_blocks) return -1;

    int sum_of_characters = t -> sum[index];
    int ix = 0;
    int smallest_difference = 2000000000;
    for(int i = 0; i < t -> number_of_blocks; i++)
        if(i != index && abs(t -> sum[i] - sum_of_characters) < smallest_difference){
            ix = i;
            smallest_difference = abs(t -> sum[i] - sum_of_characters);
        }    
    return ix;
}
