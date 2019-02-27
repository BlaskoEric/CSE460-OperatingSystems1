os: os.o VirtualMachine.o Assembler.o
	g++ -o os os.o VirtualMachine.o Assembler.o

os.o: os.cpp os.h
	g++ -c os.cpp

VirtualMachine.o: VirtualMachine.cpp VirtualMachine.h
	g++ -c VirtualMachine.cpp

Assembler.o: Assembler.cpp Assembler.h
	g++ -c Assembler.cpp

clean:
	rm *.o os
