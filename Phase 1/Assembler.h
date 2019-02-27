/*********************************************************************************************
 * Eric Blasko
 * Taylor Helms
 *
 * Assemebler.cpp
 *
 * This program reads in assembley language and output machine code. ( in is .s file, out is
 * .o file). 
 *
 * ********************************************************************************************/

#ifndef ASSEMBLER
#define ASSEMBLER

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <stdexcept>

using namespace std;

class NullPointerException: public runtime_error{
public:
    NullPointerException(): runtime_error("Null Function Pointer!") {}
};

class Assembler {
    typedef int (Assembler::*FP)(istringstream &);
    map<string,FP> opcodes;

public:
    Assembler();
    int Assemble(fstream&, fstream&);
    int load(istringstream &);
    int loadi(istringstream &);
    int store(istringstream &);
    int add(istringstream &);
    int addi(istringstream &);
    int addc(istringstream &);
    int addci(istringstream &);
    int sub(istringstream &);
    int subi(istringstream &);
    int subc(istringstream &);
    int subci(istringstream &);
    int and_(istringstream &);
    int andi(istringstream &);
    int xor_(istringstream &);
    int xori(istringstream &);
    int compl_(istringstream &);
    int shl(istringstream &);
    int shla(istringstream &);
    int shr(istringstream &);
    int shra(istringstream &);
    int compr(istringstream &);
    int compri(istringstream &);
    int getstat(istringstream &);
    int putstat(istringstream &);
    int jump(istringstream &);
    int jumpl(istringstream &);
    int jumpe(istringstream &);
    int jumpg(istringstream &);
    int call(istringstream &);
    int return_(istringstream &);
    int read(istringstream &);
    int write(istringstream &);
    int halt(istringstream &);
    int noop(istringstream &);
};

#endif
