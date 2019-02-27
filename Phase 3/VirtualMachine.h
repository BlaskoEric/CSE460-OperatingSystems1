/*************************************************************************
* Eric Blasco
* Taylor Helms
*
* VirtualMachine.h
*
* 10/31/2018
*
* Header file for VirtualMachine.cpp. Contains variables that immitate 
* the hardware found in a computer system. VMjumpTable contains function
* pointers for all execution functions. Run is the main function which
* performs the fetch, decode and execute cycle untile a interupt occurs.
*
**************************************************************************/

#ifndef VIRTUALMACHINE_H
#define VIRTUALMACHINE_H

#include <fstream>
#include <vector>
#include <map>
#include <fstream>
#include "PT.h"

using namespace std;

class VirtualMachine {
private:
	typedef void (VirtualMachine::*FP)(int&,int&,int&,int&);
	map<int, FP> VMjumpTable;
	vector <int> r;
	vector <int> mem;
	int msize;
	int rsize;
	int pc;
	int ir;
	int sr;
	int sp;
	int base;
	int limit;
	int clock;    
    int session;
    bool stop;
    PageTable tlb;
    vector<int> frameRegisters;
	int hits;

public:
	VirtualMachine();
	void run(int,fstream&,fstream&,bool);
    void setcarry(int);
    bool getcarry();
    void load(int&,int&,int&,int&);
	void store(int&,int&,int&,int&);
	void add(int&,int&,int&,int&);
	void addc(int&,int&,int&,int&);
	void sub(int&,int&,int&,int&);
	void subc(int&,int&,int&,int&);
	void and_(int&,int&,int&,int&);
	void xor_(int&,int&,int&,int&);
	void compl_(int&,int&,int&,int&);
	void shl(int&,int&,int&,int&);
	void shla(int&,int&,int&,int&);
	void shr(int&,int&,int&,int&);
	void shra(int&,int&,int&,int&);
	void compr(int&,int&,int&,int&);
	void getstat(int&,int&,int&,int&);
	void putstat(int&,int&,int&,int&);
	void jump(int&,int&,int&,int&);
	void jumpl(int&,int&,int&,int&);
	void jumpe(int&,int&,int&,int&);
	void jumpg(int&,int&,int&,int&);
	void call(int&,int&,int&,int&);
	void return_(int&,int&,int&,int&);
	void read(int&,int&,int&,int&,fstream&);
	void write(int&,int&,int&,int&,fstream&);
	void halt(int&,int&,int&,int&);
	void noop(int&,int&,int&,int&);
	int get_clock();
    int& getMemory(int);
    void setLimit(int);
    bool GetOutOfBounds(int);
    bool isOutOfBounds();
    bool isStackOverflow();
    bool isStackUnderflow();
	void CheckOverflow(int,int,int);
	void SignExtend(int);
    bool IsInMem(int);
    friend class OS;
};

#endif 
