#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/times.h>
#include <sys/resource.h>
#include <time.h>
#include "tablelibstatic.h"
#include "tablelibdynamic.h"

void time_printer(long int real1, long int real2, struct rusage rusage1, struct rusage rusage2){
    struct timeval usr1, usr2, sys1, sys2;
    double time2, time1;

    usr1 = rusage1.ru_utime;
    usr2 = rusage2.ru_utime;
    sys1 = rusage1.ru_stime;
    sys2 = rusage2.ru_stime;


    printf("real: %lf   ", ((double)(real2 - real1))/CLOCKS_PER_SEC);

    time2 = (double) sys2.tv_sec + ((double) sys2.tv_usec) / 1000000;
    time1 = (double) sys1.tv_sec + ((double) sys1.tv_usec) / 1000000;
    
    printf("sys: %lf   ", time2 - time1);
    
    time2 = (double) usr2.tv_sec + ((double) usr2.tv_usec) / 1000000;
    time1 = (double) usr1.tv_sec + ((double) usr1.tv_usec) / 1000000;
    
    printf("usr: %lf\n", time2 - time1);
}

char * generate_block(int block_size){
    char * block = malloc(block_size * sizeof(char));
    for(int i = 0; i < block_size; i++){
        int c = rand() % 26 + 97;
        block[i] = (char)c;
    }
    return block;
}

int main(int argc, char ** argv){

    //biblioteka ze statycznym alokowaniem pamieci
    void * handle = dlopen("./libtablelibstatic.so", RTLD_LAZY);
    if(!handle){
        printf("no handle for static");
        return 1;
    }

    table_static (*create_static)(int, int);
    create_static = dlsym(handle, "create_static");

    void (*add_block_static)(table_static*, int, char*, int);
    add_block_static = dlsym(handle, "add_block_static");

    void (*delete_block_static)(table_static*, int);
    delete_block_static = dlsym(handle, "delete_block_static");

    int (*find_block_with_similar_sum_static)(table_static*, int);
    find_block_with_similar_sum_static = dlsym(handle, "find_block_with_similar_sum_static");
    
    //biblioteka z dynamicznym alokowaniem pamieci
    void * han = dlopen("./libtablelibdynamic.so", RTLD_LAZY);
    if(!han){
        printf("no handle for dynamic");
        return 1;
    }

    table_dynamic* (*create_dynamic)(int, int);
    create_dynamic = dlsym(han, "create_dynamic");

    void (*add_block_dynamic)(table_dynamic*, int, char*, int);
    add_block_dynamic = dlsym(han, "add_block_dynamic");

    void (*delete_block_dynamic)(table_dynamic*, int);
    delete_block_dynamic = dlsym(han, "delete_block_dynamic");

    int (*find_block_with_similar_sum_dynamic)(table_dynamic*, int);
    find_block_with_similar_sum_dynamic = dlsym(han, "find_block_with_similar_sum_dynamic");

    //poczatek programu
    if(argc < 4){
        printf("Niepoprawna liczba argumentow, uzycie:\n./main liczbaBlokow rozmiarBloku \"s\" lub \"d\" operacje\n");
        return 0;
    }

    int number_of_blocks = atoi(argv[1]);
    int block_size = atoi(argv[2]);
    int arg = 4;

    if(strcmp(argv[3], "s") == 0){
        printf("\nstatyczna alokacja pamieci:\n");

        table_static t = create_static(number_of_blocks, block_size);

        while(arg < argc){
            if(strcmp(argv[arg], "create") == 0){
                printf("-tworzenie tablicy, statyczna alokacja\n");

                clock_t real1, real2;
                struct rusage rusage1, rusage2;
                real1 = clock();
                getrusage(RUSAGE_SELF, &rusage1);

                create_static(atoi(argv[arg+1]), atoi(argv[arg+2]));
                arg += 3;

                getrusage(RUSAGE_SELF, &rusage2);
                real2 = clock();
                time_printer(real1, real2, rusage1, rusage2);
            }
            else if(strcmp(argv[arg], "search") == 0){
                printf("-wyszukiwanie bloku o zblizonej sumie znakow, statyczna alokacja\n");

                clock_t real1, real2;
                struct rusage rusage1, rusage2;
                real1 = clock();
                getrusage(RUSAGE_SELF, &rusage1);

                find_block_with_similar_sum_static(&t, atoi(argv[arg+1]));
                arg += 2;

                getrusage(RUSAGE_SELF, &rusage2);
                real2 = clock();
                time_printer(real1, real2, rusage1, rusage2);
            }         
            else if(strcmp(argv[arg], "remove") == 0){
                printf("-usuwanie %d blokow, statyczna alokacja\n", atoi(argv[arg+1]));

                clock_t real1, real2;
                struct rusage rusage1, rusage2;
                real1 = clock();
                getrusage(RUSAGE_SELF, &rusage1);

                int ilosc = atoi(argv[arg+1]);
                for(int i = 0; i < ilosc; i++)
                    delete_block_static(&t, i%number_of_blocks);
                arg += 2;

                getrusage(RUSAGE_SELF, &rusage2);
                real2 = clock();
                time_printer(real1, real2, rusage1, rusage2);
            }   
            else if(strcmp(argv[arg], "add") == 0){
                printf("-dodawanie %d blokow, statyczna alokacja\n", atoi(argv[arg+1]));

                clock_t real1, real2;
                struct rusage rusage1, rusage2;
                real1 = clock();
                getrusage(RUSAGE_SELF, &rusage1);

                int ilosc = atoi(argv[arg+1]);
                char * block = generate_block(block_size);
                for(int i = 0; i < ilosc; i++)
                    add_block_static(&t, i%number_of_blocks, block, block_size);
                arg += 2;

                getrusage(RUSAGE_SELF, &rusage2);
                real2 = clock();
                time_printer(real1, real2, rusage1, rusage2);                
            }   
            else if(strcmp(argv[arg], "remove_and_add") == 0){
                printf("-usuwanie i dodawanie na zmiane %d blokow, statyczna alokacja\n", atoi(argv[arg+1]));

                clock_t real1, real2;
                struct rusage rusage1, rusage2;
                real1 = clock();
                getrusage(RUSAGE_SELF, &rusage1);

                int ilosc = atoi(argv[arg+1]);
                char * block = generate_block(block_size);
                for(int i = 0; i < ilosc; i++){
                    delete_block_static(&t, i%number_of_blocks);
                    add_block_static(&t, i%number_of_blocks, block, block_size);
                }
                arg += 2;

                getrusage(RUSAGE_SELF, &rusage2);
                real2 = clock();
                time_printer(real1, real2, rusage1, rusage2);
            }   
            else{
                printf("niepoprawny format polecen, uzycie:\n");
                printf("create liczbaBlokow rozmiarBloku;\n");
                printf("search indeksBloku;\n");
                printf("remove iloscBlokow;\n");
                printf("add iloscBlokow;\n");
                printf("remove_and_add iloscBlokow;\n");
                return 0;
            }
        }
    }
    else if(strcmp(argv[3], "d") == 0){
        printf("\ndynamiczna alokacja pamieci\n");

        table_dynamic * t = create_dynamic(number_of_blocks, block_size);
        
        while(arg < argc){
            if(strcmp(argv[arg], "create") == 0){
                printf("-tworzenie tablicy, dynamiczna alokacja\n");

                clock_t real1, real2;
                struct rusage rusage1, rusage2;
                real1 = clock();
                getrusage(RUSAGE_SELF, &rusage1);

                create_dynamic(atoi(argv[arg+1]), atoi(argv[arg+2]));
                arg += 3;

                getrusage(RUSAGE_SELF, &rusage2);
                real2 = clock();
                time_printer(real1, real2, rusage1, rusage2);
            }
            else if(strcmp(argv[arg], "search") == 0){
                printf("-wyszukiwanie bloku o zblizonej sumie znakow, dynamiczna alokacja\n");

                clock_t real1, real2;
                struct rusage rusage1, rusage2;
                real1 = clock();
                getrusage(RUSAGE_SELF, &rusage1);

                find_block_with_similar_sum_dynamic(t, atoi(argv[arg+1]));
                arg += 2;

                getrusage(RUSAGE_SELF, &rusage2);
                real2 = clock();
                time_printer(real1, real2, rusage1, rusage2);
            }        
            else if(strcmp(argv[arg], "remove") == 0){
                printf("-usuwanie %d blokow, dynamiczna alokacja\n", atoi(argv[arg+1]));
 
                clock_t real1, real2;
                struct rusage rusage1, rusage2;
                real1 = clock();
                getrusage(RUSAGE_SELF, &rusage1);
 
                int ilosc = atoi(argv[arg+1]);
                for(int i = 0; i < ilosc; i++)
                    delete_block_dynamic(t, i%number_of_blocks);
                arg += 2;
 
                getrusage(RUSAGE_SELF, &rusage2);
                real2 = clock();
                time_printer(real1, real2, rusage1, rusage2);
            }  
            else if(strcmp(argv[arg], "add") == 0){
                printf("-dodawanie %d blokow, dynamiczna alokacja\n", atoi(argv[arg+1]));

                clock_t real1, real2;
                struct rusage rusage1, rusage2;
                real1 = clock();
                getrusage(RUSAGE_SELF, &rusage1);

                int ilosc = atoi(argv[arg+1]);
                char * block = generate_block(block_size);
                for(int i = 0; i < ilosc; i++)
                    add_block_dynamic(t, i%number_of_blocks, block, block_size);
                arg += 2;

                getrusage(RUSAGE_SELF, &rusage2);
                real2 = clock();
                time_printer(real1, real2, rusage1, rusage2);               
            }  
            else if(strcmp(argv[arg], "remove_and_add") == 0){
                printf("-usuwanie i dodawanie na zmiane %d blokow, dynamiczna alokacja\n", atoi(argv[arg+1]));

                clock_t real1, real2;
                struct rusage rusage1, rusage2;
                real1 = clock();
                getrusage(RUSAGE_SELF, &rusage1);

                int ilosc = atoi(argv[arg+1]);
                char * block = generate_block(block_size);
                for(int i = 0; i < ilosc; i++){
                    delete_block_dynamic(t, i%number_of_blocks);
                    add_block_dynamic(t, i%number_of_blocks, block, block_size);
                }
                arg += 2;

                getrusage(RUSAGE_SELF, &rusage2);
                real2 = clock();
                time_printer(real1, real2, rusage1, rusage2);
            }  
            else{
                printf("niepoprawny format polecen, uzycie:\n");
                printf("create liczbaBlokow rozmiarBloku;\n");
                printf("search indeksBloku;\n");
                printf("remove iloscBlokow;\n");
                printf("add iloscBlokow;\n");
                printf("remove_and_add iloscBlokow;\n");
                return 0;
            }
        }        
    }
    else
        printf("niepoprawny format argumentow, uzycie:\n./main liczbaBlokow rozmiarBloku \"s\" lub \"d\" operacje\n");
}