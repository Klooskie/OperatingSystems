#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char ** argv){
    if(argc != 2){
        printf("wrong number of args, usage:\n./consumer mem/time\n");
        exit(1);
    }

    if(strcmp(argv[1], "mem") == 0){

        //petla alokujaca po megabajcie w 1 iteracji
        long long i = 0;
        while(1){
            printf("zaalokowano juz ponad %lld MB\n", i);
            char * tab = malloc(1048576);
            tab[0] = 'a';
            i++;
        }

    }
    else if(strcmp(argv[1], "time") == 0){
        
        //petla nieskonczona majaca zjadac czas
        while(1);
    
    }
    else{
     
        printf("wrong arguments, usage:\n./consumer mem/time\n");
        exit(1);
    
    }
}