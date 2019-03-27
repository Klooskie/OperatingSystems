#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

int main(int argc, char ** argv){
    //sprawdzenie liczby argumentow wejsciowych
    if(argc != 3){
        printf("zla liczba argumentow, uzycie:\n");
        printf("./filter_generator rozmiarFiltra nazwaPlikuWynikowego\n");
        exit(1);
    }

    int size = atoi(argv[1]);
    double filter[size * size];

    //przeliczenie wartosci na poszczegolnych miejscach
    srand(time(NULL));
    int sum = 0;
    for(int i = 0; i < size * size; i++){
        filter[i] = (double)(rand() % 100);
        sum += filter[i];
    }
    for(int i = 0; i < size * size; i++){
        filter[i] /= sum;
    }

    //stworzenie pliku wynikowego
    FILE * filter_file = fopen(argv[2], "w");
    if(filter_file == NULL){
        perror("blad przy tworzeniu pliku wynikowego");
        exit(1);
    }

    //dopisanie do pliku naglowka
    char buffer[100000];
    sprintf(buffer, "%d\n", size);
    fwrite(buffer, 1, strlen(buffer), filter_file);

    //dopisywanie kolejnych linii do pliku
    for(int i = 0; i < size; i++){
        buffer[0] = '\0';
        for(int j = 0; j < size; j++){
            char number[20];
            sprintf(number, "%lf ", filter[i * size + j]);
            strcat(buffer, number);
        }
        buffer[strlen(buffer)] = '\n';

        fwrite(buffer, 1, strlen(buffer), filter_file);

        // printf("%s", buffer);
    }

    if(fclose(filter_file) != 0){
        perror("blad przy zamykaniu pliku z filtrem");
        exit(1);
    }

}