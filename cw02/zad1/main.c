#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/times.h>
#include <sys/resource.h>
#include <time.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

void bad_usage(){
    printf("Zly format argumentow, uzycie:\ngenerate nazwaPliku liczbaRekordow dlugoscRekordow\nsort nazwaPliku liczbaRekordow dlugoscRekordow sys/lib\ncopy nazwaPliku1 nazwaPliku2 liczbaRekordow dlugoscRekordowBufora sys/lib\n");
}

void time_printer(long int real1, long int real2, struct rusage rusage1, struct rusage rusage2){
    struct timeval usr1, usr2, sys1, sys2;
    double time2, time1;

    usr1 = rusage1.ru_utime;
    usr2 = rusage2.ru_utime;
    sys1 = rusage1.ru_stime;
    sys2 = rusage2.ru_stime;


    printf(" real:\t%lf\n", ((double)(real2 - real1))/CLOCKS_PER_SEC);

    time2 = (double) sys2.tv_sec + ((double) sys2.tv_usec) / 1000000;
    time1 = (double) sys1.tv_sec + ((double) sys1.tv_usec) / 1000000;
    
    printf(" sys:\t%lf\n", time2 - time1);
    
    time2 = (double) usr2.tv_sec + ((double) usr2.tv_usec) / 1000000;
    time1 = (double) usr1.tv_sec + ((double) usr1.tv_usec) / 1000000;
    
    printf(" usr:\t%lf\n\n", time2 - time1);
}

void generate_record(int record_size, char * record){
    for(int i = 0; i < record_size - 1; i++)
        record[i] = (char)(rand() % 26 + 97); //litery
        //record[i] = (char)(rand() % 10 + 48); //cyfry
    record[record_size - 1] = '\n';
}

void generate_file(char * file_name, int number_of_records, int record_size){
    int new_file = creat(file_name, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
    if(new_file < 0){
        perror("blad podczas tworzenia pliku");
        exit(2);
    }
    char * record = malloc(record_size * sizeof(char));
    for(int i = 0; i < number_of_records; i++){
        generate_record(record_size, record);
        int w = write(new_file, record, record_size);
        if(w != record_size){
            perror("blad podczas zapisu do pliku");
            exit(2);
        }
    }

    free(record);

    int c = close(new_file);
    if(c < 0){
        perror("blad podczas zamykania pliku");
        exit(3);
    }
}

void sys_sort_file(char * file_name, int number_of_records, int record_size){
    int file = open(file_name, O_RDWR);
    if(file < 0){
        perror("blad podczas otwierania pliku do sortowania");
        exit(2);
    }

    char * tmp = malloc(record_size * sizeof(char));
    char * record = malloc(record_size * sizeof(char));
    
    for(int i = 1; i < number_of_records; i++){
        // record = t[i]
        long long int offset = lseek(file, i * record_size, SEEK_SET);
        if(offset < 0){
            perror("blad podczas przesuwania offsetu pliku");
            exit(2);
        }

        int len = read(file, record, record_size);
        if(len != record_size){
            perror("blad podczas odczytu z pliku");
            exit(2);
        }

        int j = i;
        while(j > 0){
            // tmp = t[j-1]
            long long int tmp_offset = lseek(file, (j - 1) * record_size, SEEK_SET);
            if(tmp_offset < 0){
                perror("blad podczas przesuwania offsetu pliku");
                exit(2);
            }

            int tmp_len = read(file, tmp, record_size);
            if(tmp_len != record_size){
                perror("blad podczas odczytu z pliku");
                exit(2);
            }

            // if tmp > record (t[j-1] > record)
            if(tmp[0] > record[0]){
                
                // t[j] = tmp (t[j] = t[j-1])
                long long int swap_offset = lseek(file, j * record_size, SEEK_SET);
                if(swap_offset < 0){
                    perror("blad podczas przesuwania offsetu pliku");
                    exit(2);
                }
                int w = write(file, tmp, record_size);
                if(w != record_size){
                    perror("blad podczas zapisu do pliku");
                    exit(2);
                }

                j--; 

            }
            else{
                break;
            }
        }
        // t[j] = record (t[j] = t[i])
        offset = lseek(file, j * record_size, SEEK_SET);
        if(offset < 0){
            perror("blad podczas przesuwania offsetu pliku");
            exit(2);
        }
        int w = write(file, record, record_size);
        if(w != record_size){
            perror("blad podczas zapisu do pliku");
            exit(2);
        }
    }

    int c = close(file);
    if(c < 0){
        perror("blad podczas zamykania pliku");
        exit(3);
    }

    free(tmp);
    free(record);
}

void lib_sort_file(char * file_name, int number_of_records, int record_size){
    FILE * file = fopen(file_name, "r+");
    if(file == NULL){
        perror("blad podczas otwierania pliku do sortowania");
        exit(2);
    }

    char * record = malloc(record_size * sizeof(char));
    char * tmp = malloc(record_size * sizeof(char));

    for(int i = 1; i < number_of_records; i++){
        // record = t[i]
        int offset = fseek(file, i * record_size, 0);
        if(offset != 0){
            perror("blad podczas przesuwania offsetu pliku");
            exit(2);
        }

        int len = fread(record, sizeof(char), record_size, file);
        if(len != record_size){
            perror("blad podczas odczytu z pliku");
            exit(2);
        }

        int j = i;
        while(j > 0){
            // tmp = t[j-1]
            int tmp_offset = fseek(file, (j - 1) * record_size, 0);
            if(tmp_offset != 0){
                perror("blad podczas przesuwania offsetu pliku");
                exit(2);
            }

            int tmp_len = fread(tmp, sizeof(char), record_size, file);
            if(tmp_len != record_size){
                perror("blad podczas odczytu z pliku");
                exit(2);
            }

            // if tmp > record (t[j-1] > record)
            if(tmp[0] > record[0]){
                
                // t[j] = tmp (t[j] = t[j-1])
                int swap_offset = fseek(file, j * record_size, 0);
                if(swap_offset != 0){
                    perror("blad podczas przesuwania offsetu pliku");
                    exit(2);
                }

                int w = fwrite(tmp, sizeof(char), record_size, file);
                if(w != record_size){
                    perror("blad podczas zapisu do pliku");
                    exit(2);
                }

                j--; 

            }
            else{
                break;
            }
        }
        // t[j] = record (t[j] = t[i])
        offset = fseek(file, j * record_size, 0);
        if(offset != 0){
            perror("blad podczas przesuwania offsetu pliku");
            exit(2);
        }

        int w = fwrite(record, sizeof(char), record_size, file);
        if(w != record_size){
            perror("blad podczas zapisu do pliku");
            exit(2);
        }
    }

    int c = fclose(file);
    if(c != 0){
        perror("blad podczas zamykania pliku");
        exit(3);
    }

    free(tmp);
    free(record);
}

void sys_copy_file(char * file1, char * file2, int number_of_records, int record_size){
    int f1 = open(file1, O_RDONLY);
    if(f1 < 0){
        perror("blad podczas otwierania pliku kopiowanego");
        exit(2);
    }
    
    int f2 = creat(file2, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
    if(f2 < 0){
        perror("blad podczas tworzenia pliku docelowego");
        exit(2);
    }
    
    char * buf = malloc(record_size * sizeof(char));

    for(int i = 0; i < number_of_records; i++){
        int len = read(f1, buf, record_size);
        if(len < 0){
            perror("blad przy odczycie z pliku");
            exit(2);
        }
        if(len == 0){
            printf("skopiowano caly plik, wystarczylo skopiowac %d rekordow\n", i);
            break;
        }
        int w = write(f2, buf, len);
        if(w != len){
            perror("blad podczas zapisu do pliku docelowego");
            exit(2);
        }
    }

    int c = close(f1);
    if(c < 0){
        perror("blad podczas zamykania pliku");
        exit(3);
    }
    c = close(f2);
    if(c < 0){
        perror("blad podczas zamykania pliku");
        exit(3);
    }

    free(buf);
}

void lib_copy_file(char * file1, char * file2, int number_of_records, int record_size){
    
    FILE  * f1 = fopen(file1, "r");
    if(f1 == NULL){
        perror("blad podczas otwierania pliku kopiowanego");
        exit(2);
    }

    FILE * f2 = fopen(file2, "w");
    if(f2 == NULL){
        perror("blad podczas tworzenia pliku docelowego");
        exit(2);
    }

    char * buf = malloc(record_size * sizeof(char));
    
    for(int i = 0; i < number_of_records; i++){

        int len = fread(buf, sizeof(char), record_size, f1);
        if(len < 0){
            perror("blad przy odczycie z pliku");
            exit(2);
        }
        if(len == 0){
            printf("skopiowano caly plik, wystarczylo skopiowac %d rekordow\n", i);
            break;
        }

        int w = fwrite(buf, sizeof(char), len, f2);
        if(w < 0 || w != len){
            perror("blad podczas zapisu do pliku docelowego");
            exit(2);
        }
    }

    int c = fclose(f1);
    if(c != 0){
        perror("blad podczas zamykania pliku");
        exit(3);
    }
    c = fclose(f2);
    if(c != 0){
        perror("blad podczas zamykania pliku");
        exit(3);
    }

    free(buf);
}

int main(int argc, char ** argv){

    int arg = 1;

    if(arg < argc){
        if(strcmp(argv[arg], "generate") == 0){
            
            if(argc != 5){
                bad_usage();
                exit(1);
            }

            clock_t real1, real2;
            struct rusage rusage1, rusage2;
            real1 = clock();
            getrusage(RUSAGE_SELF, &rusage1);

            generate_file(argv[arg+1], atoi(argv[arg+2]), atoi(argv[arg+3]));

            real2 = clock();
            getrusage(RUSAGE_SELF, &rusage2);
            printf("Generowanie pliku \"%s\" o %d rekordach rozmiaru %d byte'ow z uzyciem funkcji systemowych\n", argv[arg+1], atoi(argv[arg+2]), atoi(argv[arg+3]));
            time_printer(real1, real2, rusage1, rusage2);

        }
        else if(strcmp(argv[arg], "sort") == 0){
            
            if(argc != 6){
                bad_usage();
                exit(1);
            }

            clock_t real1, real2;
            struct rusage rusage1, rusage2;

            char * sys_or_lib = argv[arg+4];
            if(strcmp(sys_or_lib, "sys") == 0){

                real1 = clock();
                getrusage(RUSAGE_SELF, &rusage1);

                sys_sort_file(argv[arg+1], atoi(argv[arg+2]), atoi(argv[arg+3]));

                real2 = clock();
                getrusage(RUSAGE_SELF, &rusage2);
                printf("Sortowanie pliku \"%s\" o %d rekordach rozmiaru %d byte'ow z uzyciem funkcji SYSTEMOWYCH\n", argv[arg+1], atoi(argv[arg+2]), atoi(argv[arg+3]));
            }
            else if(strcmp(sys_or_lib, "lib")==0){

                real1 = clock();
                getrusage(RUSAGE_SELF, &rusage1);

                lib_sort_file(argv[arg+1], atoi(argv[arg+2]), atoi(argv[arg+3]));

                real2 = clock();
                getrusage(RUSAGE_SELF, &rusage2);
                printf("Sortowanie pliku \"%s\" o %d rekordach rozmiaru %d byte'ow z uzyciem funkcji BIBLIOTECZNYCH\n", argv[arg+1], atoi(argv[arg+2]), atoi(argv[arg+3]));
            }
            else{
                bad_usage();
                exit(1);
            }

            time_printer(real1, real2, rusage1, rusage2);   

        }
        else if(strcmp(argv[arg], "copy") == 0){


            if(argc != 7){
                bad_usage();
                exit(1);
            }

            clock_t real1, real2;
            struct rusage rusage1, rusage2;

            char * sys_or_lib = argv[arg+5];
            if(strcmp(sys_or_lib, "sys") == 0){

                real1 = clock();
                getrusage(RUSAGE_SELF, &rusage1);

                sys_copy_file(argv[arg+1], argv[arg+2], atoi(argv[arg+3]), atoi(argv[arg+4]));

                real2 = clock();
                getrusage(RUSAGE_SELF, &rusage2);
                printf("Kopiowanie %d rekordow o rozmiarze %d byte'ow z pliku %s do pliku %s z uzyciem funkcji SYSTEMOWYCH\n", atoi(argv[arg+3]), atoi(argv[arg+4]), argv[arg+1], argv[arg+2]);

            }
            else if(strcmp(sys_or_lib, "lib")==0){

                real1 = clock();
                getrusage(RUSAGE_SELF, &rusage1);

                lib_copy_file(argv[arg+1], argv[arg+2], atoi(argv[arg+3]), atoi(argv[arg+4]));
            
                real2 = clock();
                getrusage(RUSAGE_SELF, &rusage2);
                printf("Kopiowanie %d rekordow o rozmiarze %d byte'ow z pliku %s do pliku %s z uzyciem funkcji BIBLIOTECZNYCH\n", atoi(argv[arg+3]), atoi(argv[arg+4]), argv[arg+1], argv[arg+2]);
            
            }
            else{
                bad_usage();
                exit(1);
            }

            time_printer(real1, real2, rusage1, rusage2);

        }
        else{
            bad_usage();
            exit(1);
        }
    }
}