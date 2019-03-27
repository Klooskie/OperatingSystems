#ifndef TABLELIBDYNAMIC_H
#define TABLELIBDYNAMIC_H

typedef struct table_dynamic table_dynamic;
struct table_dynamic{
    char ** tab;
    int * len;
    int * sum;
    int number_of_blocks;
    int block_size;
};

//prototypy funkcji wykonywanych na tablicy z alokacja dynamiczna pamieci
table_dynamic * create_dynamic (int number_of_blocks, int block_size);
void delete_dynamic (table_dynamic * t);
void add_block_dynamic (table_dynamic * t, int index, char * block, int block_size);
void delete_block_dynamic (table_dynamic * t, int index);
int find_block_with_similar_sum_dynamic (table_dynamic * t, int index);

#endif