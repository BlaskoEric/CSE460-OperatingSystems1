/*************************************************************************
*Eric Blasco
*Taylor Helms
*
*VirtualMachine.h
*
*Header file for VirtualMachine.cpp
**************************************************************************/



#ifndef VIRTUALMACHINE_H
#define VIRTUALMACHINE_H

#include <fstream>
#include <vector>
#include <map>
#include <fstream>

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
    bool stop;

public:
	VirtualMachine();
	void run(fstream&,fstream&);
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

