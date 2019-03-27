#ifndef TABLELIBSTATIC_H
#define TABLELIBSTATIC_H

#define MAX_NUMBER_OF_BLOCKS 1000
#define MAX_BLOCK_SIZE 1000

typedef struct table_static table_static;
struct table_static{
    char tab[MAX_NUMBER_OF_BLOCKS][MAX_BLOCK_SIZE];
    int len[MAX_NUMBER_OF_BLOCKS];
    int sum[MAX_NUMBER_OF_BLOCKS];
    int number_of_blocks;
    int block_size;
};

//prototypy funkcji wykonywanych na tablicy z alokacja statyczna pamieci
table_static create_static (int number_of_blocks, int block_size);
void delete_static (table_static * t);
void add_block_static (table_static * t, int index, char * block, int block_size);
void delete_block_static (table_static * t, int index);
int find_block_with_similar_sum_static (table_static * t, int index);

#endif