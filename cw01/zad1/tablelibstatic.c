#include <stdlib.h>
#include "tablelibstatic.h"

//funkcja pomocnicza

int abss (int x){
    if(x < 0)
        return (-1) * x;
    return x;
}

//funkcje dla tablicy ze statycznym lokowaniem pamieci

table_static create_static (int number_of_blocks, int block_size){
    table_static t;
    t.number_of_blocks = number_of_blocks;
    t.block_size = block_size;
    for(int i = 0; i < number_of_blocks; i++){
        t.len[i] = 0;
        t.sum[i] = 0;
    }
    return t; 
}

//funkcja nie usuwa tablicy i nie zwalnia pamieci, jedynie ustawia odpowiednie pola struktury na 0
void delete_static (table_static * t){ 
    for(int i = 0; i < t -> number_of_blocks; i++){
        for(int j = 0; j < t -> len[i]; j++)
            t -> tab[i][j] = 0;
        t -> len[i] = 0;
        t -> sum[i] = 0;
    }
    t -> number_of_blocks = 0;
    t -> block_size = 0;
}

void add_block_static (table_static * t, int index, char * block, int block_size){
    if(index >= t -> number_of_blocks) return;
    if(block_size > t -> block_size) return;

    int sum_of_characters = 0;
    for(int i = 0; i < block_size; i++){
        t -> tab[index][i] = block[i];
        sum_of_characters += (int)block[i];
    }
    t -> len[index] = block_size;
    t -> sum[index] = sum_of_characters;
}

void delete_block_static (table_static * t, int index){
    if(index >= t -> number_of_blocks) return;

    int block_size = t -> len[index];
    for(int i = 0; i < block_size; i++)
        t -> tab[index][i] = 0;
    t -> len[index] = 0;
    t -> sum[index] = 0;
}

int find_block_with_similar_sum_static (table_static * t, int index){
    if(index >= t -> number_of_blocks) return -1;

    int sum_of_characters = t -> sum[index];
    int ix = 0;
    int smallest_difference = 2000000000;
    for(int i = 0; i < t -> number_of_blocks; i++)
        if(i != index && smallest_difference > abss(t -> sum[i] - sum_of_characters)){
            ix = i;
            smallest_difference = abss(t -> sum[i] - sum_of_characters);
        }
    return ix;
}