OUT = "wyniki.txt"

all:
	gcc -Wall main.c -o main

	echo "Testy dla rekordow rozmiaru 4 byte'ow\n" >> $(OUT)

	./main generate testFile4 10000000 4 >> $(OUT)
	./main copy testFile4 copiedFileSys4 10000000 4 sys >> $(OUT)
	./main copy testFile4 copiedFileLib4 10000000 4 lib >> $(OUT)
	./main sort copiedFileSys4 5000 4 sys >> $(OUT)
	./main sort copiedFileLib4 5000 4 lib >> $(OUT)

	echo "\n\nTesty dla rekordow rozmiaru 512 byte'ow\n" >> $(OUT)

	./main generate testFile512 5000000 512 >> $(OUT)
	./main copy testFile512 copiedFileSys512 5000000 512 sys >> $(OUT)
	./main copy testFile512 copiedFileLib512 5000000 512 lib >> $(OUT)
	./main sort copiedFileSys512 5000 512 sys >> $(OUT)
	./main sort copiedFileLib512 5000 512 lib >> $(OUT)

	echo "\n\nTesty dla rekordow rozmiaru 4096 byte'ow\n" >> $(OUT)

	./main generate testFile4096 3000000 4096 >> $(OUT)
	./main copy testFile4096 copiedFileSys4096 3000000 4096 sys >> $(OUT)
	./main copy testFile4096 copiedFileLib4096 3000000 4096 lib >> $(OUT)
	./main sort copiedFileSys4096 5000 4096 sys >> $(OUT)
	./main sort copiedFileLib4096 5000 4096 lib >> $(OUT)

	echo "\n\nTesty dla rekordow rozmiaru 8192 byte'ow\n" >> $(OUT)

	./main generate testFile8192 1000000 8192 >> $(OUT)
	./main copy testFile8192 copiedFileSys8192 1000000 8192 sys >> $(OUT)
	./main copy testFile8192 copiedFileLib8192 1000000 8192 lib >> $(OUT)
	./main sort copiedFileSys8192 5000 8192 sys >> $(OUT)
	./main sort copiedFileLib8192 5000 8192 lib >> $(OUT)

clean:
#	rm wyniki.txt
	rm testFile4 copiedFileLib4 copiedFileSys4
	rm testFile512 copiedFileLib512 copiedFileSys512
	rm testFile4096 copiedFileLib4096 copiedFileSys4096	
	rm testFile8192 copiedFileLib8192 copiedFileSys8192