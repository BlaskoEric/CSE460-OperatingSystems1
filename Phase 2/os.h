/*****************************************************************************
 * Eric Blasko
 * Taylor Helms
 *
 * os.h
 *
 * 10/31/2018
 *
 * This is the header file for os.cpp. Contains the header information for
 * both os and PCB classes. PCB is friended with OS class. This gives OS
 * access to PCB member variables. 
 *
 ****************************************************************************/

#ifndef OS_H
#define OS_H

#include <fstream>
#include <list>
#include <queue>
#include <string>
#include <iomanip>
#include <stdio.h>
#include "VirtualMachine.h"
#include "Assembler.h"

using namespace std;

class OS;

class PCB {
    int pc;
	int r[4];
	int ir;
	int sr;
	int sp;
	int base;
	int limit;

	string prog;
	fstream in;
	fstream out;
	fstream stack;

	int turn_around_time;
	int cpu_time;
	int wait_time;
	int wait_time_begin;
	int io_time;
    int io_time_begin;
    int largestStack;

public:
    PCB(const string &p, const int &b, const int &l):
        prog(p), 
        base(b), 
        limit(l), 
        pc(b), 
        sr(2), 
        sp(256/*vn,msize*/), 
        cpu_time(0), 
        wait_time(0), 
        io_time(0), 
        wait_time_begin(0),
        largestStack(0) {}
friend
    class OS;
};

class OS {
    VirtualMachine vm;
    Assembler as;

    list<PCB *> jobs;
    queue<PCB *> readyQ;
    queue<PCB *> waitQ;
    PCB * running;
    const static int TIME_SLICE = 15;
    const static int CONTEXT_SWITCH_TIME = 5;

    fstream progs;
    int idle_time;
    int context_time;
    int sys_time;
public:
    OS();
    ~OS();
    void Run();
    void LoadState();
    void SaveState();
    void ContextSwitch();
    void CheckWaitQ();
    void EndProcess(string);
	void LoadProcess();
};

#endif
