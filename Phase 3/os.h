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
#include "PT.h"

using namespace std;

class OS;

class PCB {
    PageTable pageTable;
    int pc;
	int r[4];
	int ir;
	int sr;
	int sp;
	int base;
	int limit;

	string prog;
    fstream obj;
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
        largestStack(0),
        pageTable(32) {};
friend
    class OS;
};

class OS {
    VirtualMachine vm;
    Assembler as;

    const static int FRAMES = 32;
    const static int PAGESIZE = 8;
    const static int TIME_SLICE = 15;
    const static int CONTEXT_SWITCH_TIME = 5;
    const static int DEGREE_OF_MP = 5;

    list<PCB *> jobs;
    queue<string> newQ;
    queue<PCB *> readyQ;
    queue<PCB *> waitQ;
    PCB * running;
    vector<bool> emptyFrames; 
    vector<InvertedPageTable> invertedPT;

    string message; 
    fstream progs;
    int idle_time;
    int context_time;
    int sys_time;
    int pageFaults;
    bool fifo;
    bool lru;
    int victimPages;
public:
    OS(string);
    ~OS();
    void Run();
    void LoadState();
    void SaveState();
    void ContextSwitch();
    void CheckWaitQ();
    void EndProcess(string);
	void LoadProcess();
    void LongTermScheduler();
    void PageReplacement();
    int FindVictimFrame();
    void LoadFrame(int);
    int FindFreeFrame();
    void KillFrame(int);
};

#endif
