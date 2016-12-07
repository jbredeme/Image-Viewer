all:
	cl /MD /I. *.lib ezview.c
	
clean:
	rm *.exe