.PHONY: clean

# defalut
SocketThread: 
	gcc -std=c99 -pthread clientFile.c -o cf.out
	gcc -std=c99 `xml2-config --cflags` -pthread serverFile.c `xml2-config --libs` -o sf.out -lssl -lcrypto -D_GNU_SOURCE
clean:
	$(RM) *.out
