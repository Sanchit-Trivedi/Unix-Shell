run : 
	gcc -w shell.c -o shell
	./shell
clean:
	rm -rf ./shell