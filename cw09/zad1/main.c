#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>

typedef struct buffer buffer;
struct buffer{
    char ** queue;
    int queue_size;

    int number_of_occupied_possitions;
    int put_index;
    int read_index;
};

FILE * file_to_read;
buffer buf;
pthread_mutex_t mutex_busy;
pthread_cond_t cond_not_empty;
pthread_cond_t cond_not_full;
int file_end_reached;
int line_length;
int line_length_factor;
int report_type;

void SIGINT_handler(int signo){
    printf("\nOtrzymano sygnal SIGINT, zamykanie\n");
    exit(0);
}

void exit_fun(){

    //wypisanie pustej linii dla czytelnosci
    printf("\n");

    //usuniecie mutexu
    if(pthread_mutex_destroy(&mutex_busy) != 0){
        perror("blad przy inicjalizacji zmiennej busy_writting");
        exit(1);
    }

    //usuniecie zmiennych warunkowych
    if(pthread_cond_destroy(&cond_not_empty) != 0){
        perror("blad przy inicjalizacji zmiennej busy_writting");
    }

    if(pthread_cond_destroy(&cond_not_full) != 0){
        perror("blad przy inicjalizacji zmiennej busy_writting");
    }

    //zamkniecie pliku do czytania
    if(fclose(file_to_read) != 0){
        perror("blad przy zamykaniu pliku do czytania");
    }

}

int compare_length(char * string){
    int length = strlen(string);
    if(string[length-1] == '\n')
        length--;

    if(length < line_length)
        return -1;
    else if(length == line_length)
        return 0;
    else
        return 1;
}

void producent_report(int index, char * line){
    //opcjonalne raportowanie pracy
    if(report_type == 1){
        printf("\033[0;32mProducent: \033[0m");
        if(file_end_reached == 1)
            printf("zauwazenie konca pliku\n");
        else
            printf("zapisanie linii do bufora na indeks %d -> %s", index, line);
        fflush(stdout);
    }

}

void * producent_fun(void * arg){

    while(1){
        //czekanie na mutex zajtosci kolejki
        pthread_mutex_lock(&mutex_busy);

        //czekanie az kolejka nie bedzie pelna
        while(buf.number_of_occupied_possitions == buf.queue_size){
            pthread_cond_wait(&cond_not_full, &mutex_busy);
        }

        //jesli caly plik zostal wczesniej przeczytany, to powiadamiamy ze kolejka nie jest pusta, zeby na pewno obudzic kosumentow
        if(file_end_reached == 1){
            pthread_cond_broadcast(&cond_not_empty);
            pthread_mutex_unlock(&mutex_busy);
            pthread_exit(0);
        }

        //przeczytanie linii z pliku i dopisanie do kolejki
        size_t n = 0;
        ssize_t bytes_read = getline(&buf.queue[buf.put_index], &n, file_to_read);
        if(bytes_read <= 0){
            file_end_reached = 1;
            buf.number_of_occupied_possitions--;
        }

        producent_report(buf.put_index, buf.queue[buf.put_index]);

        //aktualizacja danych bufora
        buf.number_of_occupied_possitions++;
        buf.put_index = (buf.put_index + 1) % buf.queue_size;

        //powiadomienie ze kolejka nie jest pusta i odblokowanie mutexa zajetosci kolejki
        pthread_cond_broadcast(&cond_not_empty);
        pthread_mutex_unlock(&mutex_busy);
    }
}

void consument_report(int index, char * line){
    //raportowanie pracy
    if(report_type == 0){
        if(compare_length(line) == line_length_factor){
            printf("indeks: %d -> %s", index, line);
        }
    }
    else{
        printf("\033[0;33mKonsument: \033[0modczytanie z indeksu %d, ", index);

        if(compare_length(line) == line_length_factor)
            printf("\033[1;32m");

        switch(compare_length(line)){
            case -1: printf("krotsza linia");
                        break;
            case 0: printf("rowna linia");
                        break;
            case 1: printf("dluzsza linia");
                        break;
        }

        printf("\033[0m -> %s", line);
    }
    fflush(stdout);
}

void * consument_fun(void * arg){

    while(1){
        //czekanie na mutex zajtosci kolejki
        pthread_mutex_lock(&mutex_busy);

        while(buf.number_of_occupied_possitions == 0){
            //sprawdzenie warunku koncowego, tzn kolejka jest pusta i przeczytalismy caly plik
            if(file_end_reached == 1){
                exit(0);
            }

            //czekanie az kolejka nie bedzie pusta
            pthread_cond_wait(&cond_not_empty, &mutex_busy);
        }

        //przeczytanie linii z bufora
        consument_report(buf.read_index, buf.queue[buf.read_index]);

        //usuniecie linii z pamieci
        free(buf.queue[buf.read_index]);
        buf.queue[buf.read_index] = NULL;

        //aktualizacja danych bufora
        buf.number_of_occupied_possitions--;
        buf.read_index = (buf.read_index + 1) % buf.queue_size;

        //powiadomienie o tym ze kolejka nie jest pelna i odblokowanie mutexa
        pthread_cond_broadcast(&cond_not_full);
        pthread_mutex_unlock(&mutex_busy);
    }

}

void set_exit_fun(){
    //ustawienie funkcji wyjscia
    if(atexit(exit_fun) != 0){
        perror("blad przy ustawianiu funkcji wyjscia");
        exit(1);
    }
}

void set_signal_handler(){
    //ustawienie obslugi sygnalu SIGINT
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_handler = SIGINT_handler;
    if(sigaction(SIGINT, &sa, NULL) < 0){
        perror("blad przy uzywaniu sigaction");
        exit(1);
    }
}

void set_line_length_factor(char * length_parameter){
    //ustalenie zmiennej zaleznosci dlugosci wypisywanych linii
    if(length_parameter[0] == '<')
        line_length_factor = -1;
    else if(length_parameter[0] == '=')
        line_length_factor = 0;
    else if(length_parameter[0] == '>')
        line_length_factor = 1;
    else{
        printf("bledny atrybut linii do wypisywania\n");
        exit(1);
    }
}

void set_report_type(char * print_type){
    //ustalenie typu raportowania
    if(strcmp(print_type, "simple") == 0){
        report_type = 0;
    }
    else if(strcmp(print_type, "reportall") == 0){
        report_type = 1;
    }
    else{
        printf("bledny tryb wypisywania\n");
        exit(1);
    }
}

void setup_buffer(int N){
    //zainicjalizowanie poczatkowej zawartosci kolejki
    buf.queue = calloc(N, sizeof(char *));
    buf.queue_size = N;
    buf.number_of_occupied_possitions = 0;
    buf.put_index = 0;
    buf.read_index = 0;
}

void open_file_to_read(char * file_to_read_name){
    //otwarcie pliku do czytania i ustawienie flagi przeczytania pliku
    file_to_read = fopen(file_to_read_name, "r");
    if(file_to_read == NULL){
        perror("blad przy otwarciu pliku konfiguracyjnego");
        exit(1);
    }
    file_end_reached = 0;
}

void init_multithread_communication(){
    //zainicjalizowanie mutexa zajetosci kolejki
    if(pthread_mutex_init(&mutex_busy, NULL) != 0){
        perror("blad przy inicjalizacji zmiennej busy_writting");
        exit(1);
    }

    //zainicjalizowanie zmiennych warunkowych
    if(pthread_cond_init(&cond_not_empty, NULL) != 0){
        perror("blad przy inicjalizacji zmiennej busy_writting");
        exit(1);
    }

    if(pthread_cond_init(&cond_not_full, NULL) != 0){
        perror("blad przy inicjalizacji zmiennej busy_writting");
        exit(1);
    }
}

void create_threads(int P, int K){
    //stworzenie tablic tid'ow producentow i konsumentow
    pthread_t producents[P];
    pthread_t consuments[K];

    //zablokowanie mutexa zajetosci bufora
    pthread_mutex_lock(&mutex_busy);

    //stworzenie watkow producentow
    for(int i = 0; i < P; i++){
        if(pthread_create(&producents[i], NULL, producent_fun, NULL) != 0){
            perror("blad przy tworzeniu watku producenta");
            exit(1);
        }
    }

    //stworzenie watkow konsumentow
    for(int i = 0; i < K; i++){
        if(pthread_create(&consuments[i], NULL, consument_fun, NULL) != 0){
            perror("blad przy tworzeniu watku konsumenta");
            exit(1);
        }
    }

    //odblokowanie mutexa zajetosci bufora
    pthread_mutex_unlock(&mutex_busy);
}

void end_after_maxtime(int maxtime){
    //opcjonalne zakonczenie programu po nk sekundach
    if(maxtime != 0){
        sleep(maxtime);
        printf("koniec czasu\n");
        exit(0);
    }
}

void analyze_config_file(char * config_file_name){
    //otwarcie pliku z konfiguracja
    FILE * config_file = fopen(config_file_name, "r");
    if(config_file == NULL){
        perror("blad przy otwarciu pliku konfiguracyjnego");
        exit(1);
    }

    //zainicjalizowanie i wczytanie zmiennych z pliku
    int P, K, N, maxtime;
    char file_to_read_name[100], length_parameter[1], print_type[10];
    fscanf(config_file, "%d %d %d %s %d %s %s %d", &P, &K, &N, file_to_read_name, &line_length, length_parameter, print_type, &maxtime);

    //zamkniecie pliku konfiguracyjnego
    if(fclose(config_file) != 0){
        perror("blad przy zamykaniu pliku konfiguracyjnego");
        exit(1);
    }

    set_line_length_factor(length_parameter);
    set_report_type(print_type);
    setup_buffer(N);
    open_file_to_read(file_to_read_name);
    init_multithread_communication();
    create_threads(P, K);
    end_after_maxtime(maxtime);
}

int main(int argc, char ** argv){
    //obsluzenie niepoprawnej liczby argumentow
    if(argc != 2){
        printf("niepoprawna liczba argumentow, uzycie\n./main plikKonfiguracyjny\n");
        exit(1);
    }

    set_exit_fun();
    set_signal_handler();
    analyze_config_file(argv[1]);

    pause();
}
