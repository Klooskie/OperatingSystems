#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <ftw.h>

int comparator;
int day;
int month;
int year;

void bad_usage(){
    printf("Niepoprawne argumenty wejsciowe, uzycie:\n./main  sciezkaDoKatalogu  mlodsze/starsze/rowne  dzien  miesiac(int)  rok  nftw/manual\n");
}

int date_comparator(struct tm * date){
    //mlodsze 0
    //starsze 1
    //rowne 2
    if(date -> tm_year == year){
        if(date -> tm_mon == month){
            if(date -> tm_mday == day)
                return 2;
            else if(date -> tm_mday > day)
                return 1;
            else
                return 0;
        }
        else if(date -> tm_mon > month)
            return 1;
        else
            return 0;
    }
    else if(date -> tm_year > year)
        return 1;
    else
        return 0;
            
}

void print_file(const char * path, const struct stat * stats){
    // wypisanie sciezki pliku
    printf("%s\t", path);

    // wypisanie rozmiaru pliku
    printf("%ld\t", stats -> st_size);

    // wypisanie uprawnien do pliku
    // wlasciciel
    if(stats -> st_mode & S_IRUSR) printf("r"); else printf("-");
    if(stats -> st_mode & S_IWUSR) printf("w"); else printf("-");
    if(stats -> st_mode & S_IXUSR) printf("x"); else printf("-");
    // wlasciciel grupowy
    if(stats -> st_mode & S_IRGRP) printf("r"); else printf("-");
    if(stats -> st_mode & S_IWGRP) printf("w"); else printf("-");
    if(stats -> st_mode & S_IXGRP) printf("x"); else printf("-");
    // pozostali
    if(stats -> st_mode & S_IROTH) printf("r"); else printf("-");
    if(stats -> st_mode & S_IWOTH) printf("w"); else printf("-");
    if(stats -> st_mode & S_IXOTH) printf("x\t"); else printf("-\t");

    // wypisanie daty ostatniej modyfikacji
    printf("%s", ctime(&(stats -> st_mtime)));
}

void manual(char * path){

    // otwarcie katalogu
    DIR * dir = opendir(path);
    if(dir == NULL){
        perror("blad przy otwieraniu katalogu");
        exit(2);
    }

    // wczytywanie kolejnych plikow katalogu (wsystkich) 
    struct dirent * file = readdir(dir);
    struct stat * stats = malloc(sizeof(struct stat));
    while(file != NULL){
        
        // ominiecie katalogow . i ..
        if(strcmp(file -> d_name, "..") == 0 || strcmp(file -> d_name, ".") == 0){
            file = readdir(dir);
            continue;
        }

        // stworzenie sciezki bezwzglednej danego pliku
        char * new_path = calloc(2000, sizeof(char));
        strcat(new_path, path);
        strcat(new_path, "/");
        strcat(new_path, file -> d_name);

        // odczytanie statystyk pliku
        int s_check = lstat(new_path, stats);
        if(s_check < 0){
            perror("blad przy zczytywaniu statystyk pliku");
            exit(2);
        }

        // rekurencyjna obsluga katalogow
        if(S_ISDIR(stats -> st_mode)){ 
            manual(new_path);
            file = readdir(dir);
            continue;
        }

        // obsluga (ominiecie) innych plikow niz katalogi i pliki zwykle
        if(!S_ISREG(stats -> st_mode)){
            file = readdir(dir);
            continue;
        }

        //sprawdzanie warunku z data
        struct tm * date = gmtime(&(stats -> st_mtime));
        if(date_comparator(date) != comparator){
            file = readdir(dir);
            continue;
        }

        //wypisanie pliku z danymi
        print_file(new_path, stats);
        
        free(new_path);
        file = readdir(dir);
    }

    free(stats);

    int c = closedir(dir);
    if(c != 0){
        perror("blad przy zamykaniu folderu");
        exit(2);
    }
}

int nftw_printer(const char * path, const struct stat * stats, int type, struct FTW * ftw){

    if(type == FTW_F){
        //sprawdzenie warunku z data
        struct tm * date = gmtime(&(stats -> st_mtime));
        if(date_comparator(date) == comparator){
            print_file(path, stats);    
        }
    }

    return 0;
}

int main(int argc, char ** argv){

    if(argc !=7){
        bad_usage();
        exit(1);
    }

    char * path = calloc(10000, sizeof(char));

    // ustalanie sciezki bezwzglednej
    if(argv[1][0] != '/'){
        path = getcwd(path, 2000);
        if(path == NULL){
            perror("blad przy ustalaniu sciezki bezwzglednej");
            exit(2);
        }
        strcat(path, "/");
        strcat(path, argv[1]);
    }
    else{
        strcat(path, argv[1]);
    }

    // ustawianie wartosci zaleznej od znaku < > = tak, aby pasowala to wartosci zwracanych przez date_comparator
    if(strcmp(argv[2], "mlodsze") == 0)
        comparator = 0;
    else if(strcmp(argv[2], "starsze") == 0)
        comparator = 1;
    else if(strcmp(argv[2], "rowne") == 0)
        comparator = 2;
    else{
        printf("XD\n");
        bad_usage();
        exit(1);
    }

    day = atoi(argv[3]);
    if(day < 1 || day > 31){
        bad_usage();
        exit(1);
    }

    month = atoi(argv[4]) - 1;
    if(month < 0 || month > 11){
        bad_usage();
        exit(1);
    }

    year = atoi(argv[5]) - 1900;
    if(year < 0){
        bad_usage();
        exit(1);
    }

    if(strcmp(argv[6], "nftw") == 0){
        int n = nftw(path, nftw_printer, 20, FTW_PHYS);
        if(n != 0){
            perror("blad przy wykonywaniu funkcji nftw");
            exit(2);
        }
    }
    else if(strcmp(argv[6], "manual") == 0){
        manual(path);
    }
    else{
        bad_usage();
        exit(1);
    }
}