#include "Assembler.h"
#include "VirtualMachine.h"
#include <iostream>

int main(int argc, char *argv[])
{
    Assembler as;
    VirtualMachine vm;
    sys_time = 0;
    idle_time = 0;
    context_time = 0;

    //check if there was no argument
    if(argc == 1)
    {
        cout << "Error: Must supply an input file name.\n";
        exit(1);
    }
    
    //get file name from argument
    string inputFile = argv[1];

    //check if file has .s suffix. exit if not
    int pos = inputFile.find(".");
    if (pos > inputFile.length() || inputFile[pos+1] != 's') 
    {
        cout << "No .s suffix.\n";
        exit(2);
    }

    //write outfile with infile name and .o suffix
    string name = inputFile.substr(0,pos);
    string outputFile = name + ".o";
    
    //open stream for .s(input) and .o(output) files
    fstream assembly, objectCode;
    assembly.open(inputFile.c_str(), ios::in);
    objectCode.open(outputFile.c_str(), ios::out);
    
    //run assembler (stop and close files and exit if error returned))
    if(as.Assemble(assembly, objectCode))
    {
        cout << "Assembler Error\n";
        assembly.close();
        objectCode.close();
        exit(3);
    }

    //close .s and .o files
    assembly.close();
    objectCode.close();

    //open .o(input) and stream for .in(input) and .out(output)
    objectCode.open(outputFile.c_str(), ios::in);
    fstream in, out;
    in.open((name + ".in").c_str(), ios::in);
    out.open((name + ".out").c_str(), ios::out);
    
    //load memory to vm
	int limit = 0;
	while(!objectCode.eof())
	{
	    objectCode >> vm.getMemory(limit++);
	}
	
	vm.setLimit(limit);
	objectCode.close();

    //run virtual machine and print clock to .out file
    vm.run(in, out);
    out << "\nClock = " << vm.get_clock() << endl;

    //close files
    in.close();
    out.close();
    
    return 0;
}
