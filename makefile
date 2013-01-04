EXE=check_summ.e

all: $(EXE) 
$(EXE): main.c
	gcc main.c -lm -o $(EXE) 

clean:
	rm -f ./$(EXE)