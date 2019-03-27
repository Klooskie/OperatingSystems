#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>

int empty(char * str){
    if(str[0] == '\0')
        return 1;
    else
        return 0;
}

char * getname(char * prog){
    int i = 0, shift = 0;
    while(prog[i] != '\0'){
        if(prog[i] == '/')
            shift = i + 1;
        i++;
    }

    return prog + shift;
}

char * cut_white_spaces(char * buffer){
    int i = 0;
    while(buffer[i] == ' ') i++;
    buffer += i;
    i = 0;
    while(buffer[i] != '\0') i++;
    while(buffer[i-1] == ' ' || buffer[i-1] == '\n'){
        i--;
    }
    buffer[i] = '\0';
    return buffer;
}

void run_command(char * buffer){
    //stworzenie tablicy kolejnych argumentow
    char ** args = malloc(101 * sizeof(char *));
    char * prog = strtok(buffer, " \n\t");
    args[0] = getname(prog);

    //odczytanie kolejnych argumentow
    int i = 0;
    while(args[i] != NULL){
        i++;

        if(i == 101){
            printf("przekroczono maksymalna liczbe argumentow (nazwa_programu + 99 argumentow)");
            exit(1);
        }
        
        args[i] = strtok(NULL, " \n\t");
    }

    //uruchomienie programu
    int exec_status = execvp(prog, args); 
    if(exec_status == -1){
        perror("\x1b[31m Blad przy probie wykonnania polecenia\x1b[0m");
        exit(1);
    }
    
}

int main(int argc, char ** argv){
    
    //obsluga niepoprawnej ilosci arguemntow
    if(argc != 2){
        printf("Bledne argumenty, uzycie:\n./main nazwaPlikuWsadowego\n");
        exit(1);
    }

    //otwarcie pliku
    FILE * file = fopen(argv[1], "r");
    if(file == NULL){
        perror("blad przy otwieraniu pliku wsadowego");
        exit(1);
    }

    char * buffer = NULL;
    size_t buf_size;

    //odczytanie pierwszej linii
    int size = getline(&buffer, &buf_size, file);

    while(size != -1){
        //ominiecie ewentualnych pustych linii
        if(size == 1){
            size = getline(&buffer, &buf_size, file);
            continue;
        }

        buffer = cut_white_spaces(buffer);

        //stworzenie tablicy na poszczegolne komendy
        char ** commands = calloc(11, sizeof(char *));
        int number_of_commands = 0;

        //wypisanie instrukcji i zapisanie kopii
        char * bufbackup = malloc(size * sizeof(char));
        strcpy(bufbackup, buffer);
        printf("\x1B[32mInstrukcja: \"%s\"\x1b[0m\n", buffer);

        //podzielenie instrukcji na kolejne oddzielone pipe'ami
        commands[0] = strtok(buffer, "|");
        while(commands[number_of_commands] != NULL){
            number_of_commands++;
            //sprawdzenie przekroczenia limitu instrukcji w 1 wierszu
            if(number_of_commands == 11){
                printf("przekroczono liczbe mozliwych instrukcji oddzielonych '|' (10)\n");
                exit(1);
            }
            commands[number_of_commands] = strtok(NULL, "|");
        }

        int input = STDIN_FILENO;
        int output;

        //przejrzenie wszystkich komend danego wiersza
        for(int i = 0; i < number_of_commands; i++){
            //wczytanie komendy, odciecie bialych znakow na koncach, wypisanie jej
            buffer = cut_white_spaces(commands[i]);
            if(empty(buffer) == 1){
                printf("Zla skladnia wyrazenia \"%s\"\njedna z instrukcji jest pusta\n", bufbackup);
                exit(1);
            }
            printf("\x1B[36mAktualnie przetwarzane polecenie: %s\033[0m\n", buffer);

            //ustalenie odpowiednich outputow w zaleznosci od numeru komendy
            int fd[2];
            if(i != number_of_commands - 1){
                //dla nie ostatniej komendy tworze potok
                if(pipe(fd) != 0){
                    perror("blad przy tworzeniu potoku");
                    exit(1);
                }
                //ustawiam output tak, zeby komenda pisala do potoku
                output = fd[1];
            }
            else{
                //dla ostatniej komendy ustawiam output tak, zeby pisala na stdout
                output = STDOUT_FILENO;
            }

            //tworze nowy proces
            pid_t pid = fork();
            if(pid < 0){
                perror("blad przy tworzeniu potomka");
                exit(1);
            }
            else if(pid == 0){
                //ustawiam dziecku input i output
                if(input != STDIN_FILENO){
                    if(dup2(input, STDIN_FILENO) < 0){
                        perror("blad przy uzywaniu dup2 input");
                        exit(1);
                    }
                    close(input);
                }

                if(output != STDOUT_FILENO){
                    if(dup2(output, STDOUT_FILENO) < 0){
                        perror("blad przy uzywaniu dup2 output");
                        exit(1);
                    }
                    close(output);
                }
                
                //odpalam komende
                run_command(buffer);
                //execl("/bin/sh", "sh", "-c", buffer, NULL);
            }
            else{
                //zamkniecie zapisu do potoku
                close(fd[1]);
                
                //zapisanie nowego inputu
                input = fd[0];

                //oczekiwanie na zakonczenie procesu potomnego
                int status;
                wait(&status);

                //obsluga zakonczenia z innym statusem niz 0
                if(WIFSIGNALED(status)){
                    printf("\x1b[31m Proces z poleceniem \"%s\" zakonczyl sie ze statusem innym niz 0, zostal zabity sygnalem %d\n", bufbackup, WTERMSIG(status));
                    exit(1);
                }
                if(WIFEXITED(status) && WEXITSTATUS(status) != 0){
                    printf("\x1b[31m Proces z poleceniem \"%s\" zakonczyl sie ze statusem innym niz 0 (%d) \x1b[0m \n", bufbackup, WEXITSTATUS(status));
                    exit(1);
                }

            }

        }
        
        //wczytanie kolejnej linii z pliku wsadowego
        printf("\n");
        size = getline(&buffer, &buf_size, file);
    }
}