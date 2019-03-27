CC = gcc
CFLAGS = -Wall
INPUT = baboon.pgm
OUTPUT = Times.txt

all:
	make compile
	make test

compile:
	$(CC) $(CFLAGS) main.c -o main -lm -pthread
	$(CC) $(CFLAGS) filter_generator.c -o filter_generator

test:
	echo "Pomiary czasow operacji filtrowania obrazu dla roznych ilosci watkow i roznych filtrow" >> $(OUTPUT)
	
	echo "\nFiltr pierwszy, 30 na 30\n" >> $(OUTPUT)
	
	./filter_generator 30 fil1 >> $(OUTPUT)
	./main 1 $(INPUT) fil1 out1 >> $(OUTPUT)
	./main 2 $(INPUT) fil1 out1 >> $(OUTPUT)
	./main 4 $(INPUT) fil1 out1 >> $(OUTPUT)
	./main 8 $(INPUT) fil1 out1 >> $(OUTPUT)

	echo "\nFiltr drugi, 40 na 40\n" >> $(OUTPUT)
	
	./filter_generator 40 fil2 >> $(OUTPUT)
	./main 1 $(INPUT) fil2 out2 >> $(OUTPUT)
	./main 2 $(INPUT) fil2 out2 >> $(OUTPUT)
	./main 4 $(INPUT) fil2 out2 >> $(OUTPUT)
	./main 8 $(INPUT) fil2 out2 >> $(OUTPUT)

	echo "\nFiltr trzeci, 50 na 50\n" >> $(OUTPUT)
	
	./filter_generator 50 fil3 >> $(OUTPUT)
	./main 1 $(INPUT) fil3 out3 >> $(OUTPUT)
	./main 2 $(INPUT) fil3 out3 >> $(OUTPUT)
	./main 4 $(INPUT) fil3 out3 >> $(OUTPUT)
	./main 8 $(INPUT) fil3 out3 >> $(OUTPUT)

clean:
	rm main filter_generator

cleanall:
	rm -f main filter_generator fil1 fil2 fil3 out1 out2 out3 Times.txt
