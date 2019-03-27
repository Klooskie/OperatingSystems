#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <pthread.h>
#include <sys/time.h>


int ** unfiltered_picture;
int ** filtered_picture;
int width;
int height;
int max_pixel_value;

double ** filter;
int filter_size;

typedef struct thread_responsibility thread_responsibility;
struct thread_responsibility{
    int thread_number;
    int start;
    int limit;
};

int max(int x, int y){
    return x > y ? x : y;
}

int min(int x, int y){
    return x < y ? x : y;
}

void parse_input_file(char * input_file_path){
    //otwarcie obrazu wejsciowego
    FILE * input_file = fopen(input_file_path, "r");
    if(input_file == NULL){
        perror("blad przy otwieraniu obrazu wejsciowego");
        exit(1);
    }

    size_t buffer_size = 100000;
    char * buffer = malloc(buffer_size);

    //ominiecie 1 linii ("P2") i ew lini komentarza
    getline(&buffer, &buffer_size, input_file);
    getline(&buffer, &buffer_size, input_file);
    if(buffer[0] == '#')
        getline(&buffer, &buffer_size, input_file);
    
    //zapamietanie danych z 2 linii
    width = atoi(strtok(buffer, " \n\t"));
    height = atoi(strtok(NULL, " \n\t"));

    //zapamietanie danej z 3 linii
    getline(&buffer, &buffer_size, input_file);
    max_pixel_value = atoi(strtok(buffer, " \n\t"));
    
    //alokacja tablicy
    unfiltered_picture = malloc(height * sizeof(int *));
    for(int i = 0; i < height; i++)
        unfiltered_picture[i] = malloc(width * sizeof(int));
    
    //przepisanie tablicy zawierajacej dane obrazu
    int index = 0;
    ssize_t line_length = getline(&buffer, &buffer_size, input_file);
    while(line_length > 0){
        
        char * pixel_value = strtok(buffer, " \n\t\r");        
        while(pixel_value != NULL){
            unfiltered_picture[index / width][index % width] = atoi(pixel_value);
            index++;
            
            pixel_value = strtok(NULL, " \n\t\r");
        }

        line_length = getline(&buffer, &buffer_size, input_file);
    }

    //zwolnienie bufora i zamkniecie pliku
    // free(buffer);
    if(fclose(input_file) != 0){
        perror("blad przy zamykaniu pliku z obrazem zrodlowym");
        exit(1);
    }
}

void parse_filter_file(char * filter_file_path){
    //otwarcie filtra
    FILE * filter_file = fopen(filter_file_path, "r");
    if(filter_file == NULL){
        perror("blad przy otwieraniu pliku z filtrem");
        exit(1);
    }

    size_t buffer_size = 100000;
    char * buffer = malloc(buffer_size);

    //odczytanie danej z 1 linii
    getline(&buffer, &buffer_size, filter_file);
    filter_size = atoi(strtok(buffer, " \n\t"));

    //alokacja tablicy
    filter = malloc(filter_size * sizeof(double *));
    for(int i = 0; i < filter_size; i++)
        filter[i] = malloc(filter_size * sizeof(double));

    //przepisanie filtra
    int index = 0;
    ssize_t line_length = getline(&buffer, &buffer_size, filter_file);
    while(line_length > 0){
        
        char * filter_value = strtok(buffer, " \n\t\r");        
        while(filter_value != NULL){
            filter[index / filter_size][index % filter_size] = atof(filter_value);
            index++;
            
            filter_value = strtok(NULL, " \n\t\r");
        }

        line_length = getline(&buffer, &buffer_size, filter_file);
    }

    //zwolnienie bufora i zamkniecie pliku
    // free(buffer);
    if(fclose(filter_file) != 0){
        perror("blad przy zamykaniu pliku z filtrem");
        exit(1);
    }
}

void save_calculated_picture(char * output_file_path){
    //utworzenie pliku wynikowego
    FILE * output_file = fopen(output_file_path, "w");
    if(output_file == NULL){
        perror("blad przy tworzeniu pliku wynikowego");
        exit(1);
    }

    size_t buffer_size = 100000;
    char * buffer = malloc(buffer_size);

    //dopisanie do pliku naglowka
    sprintf(buffer, "P2\n%d %d\n%d\n", width, height, max_pixel_value);
    fwrite(buffer, 1, strlen(buffer), output_file);

    //dopisywanie kolejnych linii do pliku
    for(int i = 0; i < height; i++){
        buffer[0] = '\0';
        for(int j = 0; j < width; j++){
            char pixel_value[4];
            sprintf(pixel_value, "%d ", filtered_picture[i][j]);
            strcat(buffer, pixel_value);
        }
        buffer[strlen(buffer) - 1] = '\n';
        fwrite(buffer, 1, strlen(buffer), output_file);
    }    

    //zwolnienie bufora i zamkniecie pliku
    // free(buffer);
    if(fclose(output_file) != 0){
        perror("blad przy zamykaniu pliku z filtrem");
        exit(1);
    }
}

void calculate_pixel_color(int x, int y){
    //algorytm podany w tresci zadania z drobna modyfikacja, aby nie wychodzic poza tablice
    double color = 0;
    int k = (int)ceil(((double)filter_size) / 2.0);

    for(int i = 0; i < filter_size; i++){
        for(int j = 0; j < filter_size; j++){
            
            int row = min(height - 1, max(0, x - k + i));
            int column = min(width - 1, max(0, y - k + j));
            
            color += ((double)unfiltered_picture[row][column]) * filter[i][j];            
        }
    }

    if(color < 0)
        filtered_picture[x][y] = 0;
    else if(color > max_pixel_value)
        filtered_picture[x][y] = max_pixel_value;
    else
        filtered_picture[x][y] = (int)round(color);
}

void * calculate_filtered_picture(void * arg){
    //zrzutowanie arg na zmienna typu responsibility
    thread_responsibility responsibility = *(thread_responsibility *)arg;

    //przeliczenie odpowienich pixeli
    for(int i = responsibility.start; i < responsibility.limit; i++){
        calculate_pixel_color(i / width, i % width);
    }

    pthread_exit(0);
}

int main(int argc, char ** argv){
    //sprawdzenie liczby argumentow wejsciowych
    if(argc != 5){
        printf("zla liczba argumentow, uzycie:\n");
        printf("./main liczbaWatkow obrazWejsciowy definicjaFiltru plikWynikowy\n");
        exit(1);
    }

    //przypisanie argumentow programu do zmiennych
    int number_of_threads = atoi(argv[1]);

    //sparsowanie pliku z obrazem wejsciowym
    parse_input_file(argv[2]);

    //sparsowanie pliku z filtrem
    parse_filter_file(argv[3]);

    //alokacja tablicy na przefiltrowany obraz
    filtered_picture = malloc(height * sizeof(int *));
    for(int i = 0; i < height; i++)
        filtered_picture[i] = malloc(width * sizeof(int));

    //przelioczenie zakresow pixeli dla poszczegolnych watkow
    thread_responsibility * responsibility = malloc(number_of_threads * sizeof(thread_responsibility));
    int number_of_pixels = width * height;
    int one_thread_ammount_of_pixels = number_of_pixels / number_of_threads;

    for(int i = 0; i < number_of_threads; i++){
        responsibility[i].thread_number = i;
        responsibility[i].start = i * one_thread_ammount_of_pixels;

        if(i == number_of_threads - 1)
            responsibility[i].limit = number_of_pixels;
        else
            responsibility[i].limit = (i + 1) * one_thread_ammount_of_pixels;
    }

    //stworzenie tablicy tid'ow i zapisanie czasu poczatkowego
    pthread_t threads[number_of_threads];
    struct timeval time_start, time_end;
    gettimeofday(&time_start, NULL);

    //stworzenie watkow i przekazanie im argumentu z zakresem odpowiedzialnosci
    for(int i = 0; i < number_of_threads; i++){
        if(pthread_create(&threads[i], NULL, calculate_filtered_picture, &responsibility[i]) != 0){
            perror("blad przy tworzeniu watku");
            exit(1);
        }
    }

    //oczekiwanie az wszystkie watki zakoncza swoja prace
    for(int i = 0; i < number_of_threads; i++){
        if(pthread_join(threads[i], NULL) != 0){
            perror("blad przy uzywaniu pthread_join");
            exit(1);
        }
    }

    //zapisanie czasu koncowego i wypisanie roznicy
    gettimeofday(&time_end, NULL);
    printf("liczba watkow: %d, czas: %lf\n", number_of_threads, ((double)time_end.tv_sec + ((double)time_end.tv_usec)/1000000) - ((double)time_start.tv_sec + ((double)time_start.tv_usec)/1000000));

    //zapisanie przefiltrowanego obrazu do pliku
    save_calculated_picture(argv[4]);
}