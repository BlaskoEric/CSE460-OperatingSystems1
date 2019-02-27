/**************************************************************************
 * Eric Blasko
 * Taylor Helsm
 *
 * os.cpp
 *
 * 10/31/2018
 *
 * This program imitates a functoning Operating System. OS loaded '.s'
 * files and sends them to Assembler to complie to '.o' files. OS creates
 * a list of PCB's for each process and loads its object code into vm's 
 * memory. All process are loaded into the readyQ initially. OS then loads
 * vm with the first process in the readyQ. When the VM is interupted, OS
 * saves the state of the running process, reschedules all process, moves
 * the last running process to its proper queue based on return status or 
 * terminates if nessacery, then loads the next process from the readyQ into
 * the VM. If readyQ is empty and waitQ is full, OS idle's until another 
 * process if available
 *************************************************************************/
  
#include "os.h"
#include <iostream>

using namespace std;

//default constructor. loads programs on start
OS::OS(string alg)
{
    if(alg == "-fifo")
    {
        fifo = true;
        lru = false;
    }
    else
    {
        fifo = false;
        lru = true;
    }

    idle_time = 0;
    context_time = 0;
    sys_time = 0;
    pageFaults = 0;
    invertedPT.resize(FRAMES);    
    for(int i = 0; i < FRAMES; i++)
        emptyFrames.push_back(false);
    victimPages = 0;
}

//gathers all '.s' files into a program file and compiles them to '.o' files. 
//Then creates a PCB for each process and loads it into the readyQ. Program
//is deleted after loading is complete
void OS::LoadProcess()
{
    system("ls *.s > program");
    fstream filesIn;
    filesIn.open("program",ios::in);

    if(!filesIn)
    {
        cerr << "Error\n";
        exit(1);
    }

    string prog;
    while(filesIn >> prog)
    {
        newQ.push(prog);
    }

    filesIn.close();
    system("rm program");
    LongTermScheduler();
}

void OS::LongTermScheduler()
{
    
    while((readyQ.size() + waitQ.size()) < DEGREE_OF_MP && !newQ.empty())
    {
        string prog = newQ.front();
        newQ.pop();

        cout << "Loading " << prog << "..." << endl;
 
        int pos = prog.find(".");
        string name = prog.substr(0,pos);
        string outputFile = name + ".o";
	string inFile = name + ".in";
	string outFile = name + ".out";
	string st = name + ".st";
        
        fstream assemblyCode, objectCode;
        assemblyCode.open(prog.c_str(),ios::in);
        if(!assemblyCode)
        {
            cerr << "Error loading" << prog << endl;
            continue;
        }

        objectCode.open(outputFile.c_str(),ios::out);
        if(!objectCode)
        {
            cerr << "Error loading" << prog << endl;
            continue;
        }

        if(as.Assemble(assemblyCode, objectCode))
        {
            cout << "Assembler Error in " << prog << endl;
            assemblyCode.close();
            objectCode.close();
            continue;
        }
       
        assemblyCode.close();
        objectCode.close();

        objectCode.open(outputFile.c_str(),ios::in);
        if(!objectCode)
        {
            cerr << "Error loading" << prog << endl;
            continue;
        }

        int base = 0; 
        int limit = 0;
        string temp;

    	while(!objectCode.eof())
	    {
	        objectCode >> temp;
            limit++;
	    }

        objectCode.close();
	   
        PCB * p = new PCB(name, base, limit - 1);
	    
        p->obj.open(outputFile.c_str(),ios::in | ios::out);
        if(!p->obj)
        {
           cerr << "Error loading" << prog << endl;
            continue;
        }    
    
        p->in.open(inFile.c_str(),ios::in);
        if(!p->in)
        {
            cerr << "Error loading " << prog << endl;
            continue;
        }
    	p->out.open(outFile.c_str(),ios::out);

        if(!p->out)
        {
            cerr << "Error loading" << prog << endl;
            continue;
        }

        system(string("touch " + name + ".st").c_str());
	p->stack.open(st.c_str(),ios::in | ios::out);
        if(!p->stack)
        {
            cerr << "Error opening stack " << prog << endl;
            continue;
        }
	
	cout << "Load Successful\n\n";

        readyQ.push(p);
        jobs.push_back(p);

        running = p;
        running->sr |= 0x0400;
        PageReplacement(); 

        objectCode.close(); 
    }
    running = NULL;
}

//Deconstructor. Prints system information to each processes output file. After, each process is done
//its '.st' file is removed, all files are closed and PCB * is deleted
OS::~OS()
{
    double userClocks = 0;
    double processes = 0;
    for (auto const& i : jobs)
    {
        userClocks += i->cpu_time;
        processes++;
    }

        double systemTime = ((double)(context_time) + (double)(idle_time)) / 1000.0;
        double cpuTime = (double)(vm.clock) / 1000.0;
        double cpuUtil = (double)(vm.clock - idle_time) / (double)(vm.clock) * 100;
        double userCpu = userClocks / (double)(vm.clock + context_time) * 100;
        double throughput = (double)(processes) / ((double)(sys_time) / 1000.0);
	double paging  = (double)(pageFaults) + (double)(vm.hits);
	double hitRatio = ((double)(vm.hits) / paging) * 100;

    for (auto const& i : jobs) 
	{

        i->out << "\n\tSystem Information\n";
        i->out << "System Time: \t\t" << fixed <<setprecision(3) << systemTime << " Seconds\n";
        i->out << "CPU Time: \t\t"  << cpuTime << " Seconds\n";
        i->out << "CPU Utilization: \t" << cpuUtil << "%\n";
        i->out << "User CPU Utilization: \t" << userCpu << "%\n";
        i->out << "Throughput: \t\t" << throughput << " Per Seconds\n"; 
	    
        if(fifo == true)
            i->out << "\n\tAlgorithm type: FIFO\n";
        else
            i->out << "\n\tAlgorithm type: LRU\n";

        i->out << "Page Faults: \t\t" << pageFaults << endl;
        i->out << "Page Hits: \t\t" << vm.hits << endl;
	    i->out << "Hit Ratio: \t\t" << hitRatio << endl;
        i->out << "Number of Victims \t" << victimPages << endl << endl; 
        i->out.close();
        i->in.close();
        i->stack.close(); 
        remove(string(i->prog + ".st").c_str());
        cout << "Closing " << i->prog << "..." << endl;
	    delete i;
	}
    
}

//Runs as long as a process is in ready or wait queue's. Loads PCB to VM, runs VM, then
//performs context switch on return. if all process are in waitQ, system will idle
void OS::Run()
{
	while(!readyQ.empty() || !waitQ.empty() || running != NULL)
	{
		if(!readyQ.empty() || running != NULL)
		{
			if(running == NULL)
			{
				running = readyQ.front();
				readyQ.pop();
			}
			LoadState();
			if(fifo == true)
        		vm.run(TIME_SLICE, running->in, running->out, false);
			else
		        vm.run(TIME_SLICE, running->in, running->out, true);
			ContextSwitch();
		}
		else
		{
            sys_time++;
            idle_time++;
			CheckWaitQ();
		}
    }
}

//loads current running process data into vm. If it has a stack, load
//it into memeory from stack file
void OS::LoadState()
{
    for(int i = 0; i < 4; i++)
        vm.r[i] = running->r[i];

    vm.pc = running->pc;
    vm.ir = running->ir;
    vm.sr = running->sr;
    vm.sp = running->sp;
    vm.base = running->base;
    vm.limit = running->limit;
    vm.tlb= running->pageTable;
	running->wait_time += (sys_time - running->wait_time_begin);
    
    running->stack.open(string(running->prog + ".st").c_str(),ios::in);
    for(int i = vm.sp; i < vm.msize and not running->stack.fail();i++)
	{
		if(invertedPT[i/8].pid != "")
		{
            KillFrame(i/8);
	        emptyFrames[i/8] = false;
            invertedPT[i/8].pid = "";
            
    	}	
	    
        running->stack >> vm.mem[i];
    
	}
	running->stack.close();

}

//saves vm data into the returned process. If it had a stack, it is 
//saved to stack file
void OS::SaveState()
{
    for(int i = 0; i < 4; i++)
        running->r[i] = vm.r[i];

    running->pc = vm.pc;
    running->ir = vm.ir;
    running->sr = vm.sr;
    running->sp = vm.sp;
    running->base = vm.base;
    running->limit = vm.limit;
    running->cpu_time += (vm.clock - vm.session);
    running->pageTable = vm.tlb;
    sys_time += (vm.clock - vm.session);

    int stackSize = 0;
    running->stack.open(string(running->prog + ".st").c_str(), ios::out);
    for(int i = vm.sp; i < vm.msize; i++)
    {
        running->stack << vm.mem[i] << endl;
        stackSize++;
    }
    running->stack.close();
        if(running->largestStack < stackSize)
            running->largestStack = stackSize;

}

//Saves state of returned process. Then checks if any process finished
//in the waitQ. Then looks at VM return status to tell if a read, write,
//timeslice, halt or error occured. Will ether end the process or put
//it into the proper queue 
void OS::ContextSwitch()
{
    context_time += CONTEXT_SWITCH_TIME;
    sys_time += CONTEXT_SWITCH_TIME;
    SaveState();

    CheckWaitQ();
   
    int sr = ((running->sr >> 5) & 0x027); 
    switch(sr)
    { 
        case 0: //Timeslice
            running->wait_time_begin = sys_time;
            readyQ.push(running);
            break;
        case 1: //halt
            message = "Successfully Completed";
            EndProcess(message);
            break;
        case 2: //out of bounds
            message = "Out of Bounds";
            EndProcess(message);
            break;
        case 3: //stack overflow
            message = "Stack Overflow Detected";
            EndProcess(message);
            break;
        case 4: //stack underflow
            message = "Stack Underflow Detected";
            EndProcess(message);
            break;
        case 5: //invalid opcode
            message = "Invalid Opcodes";
            EndProcess(message);
            break;
        case 6: //read
            waitQ.push(running);
            running->io_time_begin = sys_time;
            break;
        case 7: //write
            waitQ.push(running);
            running->io_time_begin = sys_time;
            break;
        case 32: //page fault
            pageFaults++;
            PageReplacement();
            waitQ.push(running);
            running->io_time_begin = sys_time;
            break;
        default:
            cout << "Unknown error encountered" << endl;
	break;
    } 
    
    if(!readyQ.empty())                                     //Process in readyQ
    {
        running = readyQ.front();
        readyQ.pop();
    }
    else                                                    //readyQ empty
        running = NULL; 
}

//checks if I/O operation has completed. if it has, increase its io time,
//push to readyQ and start its wait time
void OS::CheckWaitQ()
{
    if(!waitQ.empty())
    {
        while(!waitQ.empty())
        {
            PCB * waiting = waitQ.front();
            if((sys_time - waiting->io_time_begin) >= 27)
            {
                waiting->io_time += 27;
                waitQ.pop();
                readyQ.push(waiting);
                waiting->wait_time_begin = sys_time;
            }
            else
                return;
        }
    }
}

//finds a free frame. If one is found it is loaded immediately, else
//a frame must be freed before loading. FIFO and LRU only used if
//no frames available
void OS::PageReplacement()
{
    int freeFrame = FindFreeFrame();
    if(freeFrame >= 0)
    {
	    LoadFrame(freeFrame);
    }

    else if(freeFrame < -1) 
    {
        
	    freeFrame = FindVictimFrame();
	    LoadFrame(freeFrame);
    }
}

//checks if a empty frame is available and returns it if found.
//if not found returns -1
int OS::FindFreeFrame()
{
    for(int i = 0; i < emptyFrames.size(); i++)
    {
        if(emptyFrames[i] == false)
		    return i;
    }
    return -1;
}

//Loads physical memory with entries. The inverted page table records
//the program name(pid), page number and timestamp (if FIFO). The pcb
//records frame number in its page table and sets it to valid. 
void OS::LoadFrame(int frameNumber)
{
            emptyFrames[frameNumber] = true;
            int page = running->sr >> 11;
            
            invertedPT[frameNumber].pid = running->prog;
            invertedPT[frameNumber].page = page;

            if(fifo == true)
                invertedPT[frameNumber].timeStamp = vm.clock;
            else
                vm.frameRegisters[frameNumber] = vm.clock;

            running->pageTable.page_table[page].frame = frameNumber;
            running->pageTable.page_table[page].valid = true;
            running->obj.seekg(page*6*8, ios::beg);
            
            for(int j = 0; j < PAGESIZE; j++)
            {
                running->obj >> vm.mem[frameNumber*8 + j];
            }
            running->obj.clear();
}

//if FIFO, then search inverted page table for lowest timestamp and 
//request the frame to be killed.
//if LRU, check VM's frame registers for the lowest time stampe and
//request the frame to be killed.
//return frame number to be filled
int OS::FindVictimFrame()
{
    int frameNumber = 0;
    if(fifo == true)    //FIFO
	{
		int victimFrame = invertedPT[0].timeStamp;
		for(int i = 0; i < invertedPT.size();i++)
		{
		    if(victimFrame > invertedPT[i].timeStamp)
		    {
		        victimFrame = invertedPT[i].timeStamp;
		        frameNumber = i;
		    }
		}
	}
	else                //LRU
	{
		int victimFrame = vm.frameRegisters[0];
		int frameNumber = 0;
		for(int i = 0; i < vm.frameRegisters.size();i++)
		{
		    if(vm.frameRegisters[i] < victimFrame)
		    {
		        victimFrame = vm.frameRegisters[i];
		        frameNumber = i;
		    }
		}

	}
    
    KillFrame(frameNumber);

	return frameNumber;
}

//Gets pid and page from inverted table of frame that is to be removed.
//Searches pcb's for a matching pid(prog) and checks if that page has
//been modified. If it has, read memeory back to file before clearing
//frame. Lastly set pcb's page table location to not valid;
void OS::KillFrame(int frameNumber)
{
    victimPages++;
    string pid = invertedPT[frameNumber].pid;
    int page = invertedPT[frameNumber].page;
    for(auto * x : jobs)
    {
        if(x->prog == pid)
        { 
            if(x->pageTable.page_table[page].modified == true)
            {
                x->obj.seekp(page * 6 * 8, ios::beg);
                for(int j = 0; j < PAGESIZE; j++)
                {
                    x->obj << vm.mem[(frameNumber * 8) + j] << endl;
                }
                x->obj.clear();
            }
		
            x->pageTable.page_table[page].valid = false;
            invertedPT[frameNumber].pid = "";
            invertedPT[frameNumber].page = 0;
        }
    }
 
    emptyFrames[frameNumber] = false;
}

//if a process has ended, clear inverted page table if any page 
//table entries are in memory. Then print its status to its output file
void OS::EndProcess(string message)
{
        for(int i = 0; i < invertedPT.size();i++)
        {
            if(invertedPT[i].pid == running->prog)
            {
                emptyFrames[i] = false;
                invertedPT[i].pid = "";
                invertedPT[i].page = 0;
            }
        }

        double time = running->cpu_time + running->wait_time + running->io_time;

        running->out << "\n\tProcess Information\n";
        running->out << "Program name: \t\t" << running->prog << endl;
        running->out << message << endl;
        running->out << "CPU Time: \t\t" << running->cpu_time << " Ticks\n";
        running->out << "Waiting Time: \t\t" << running->wait_time << " Ticks\n";
        running->out << "I/O Time: \t\t" << running->io_time << " Ticks\n";
        running->out << "Turnaround Time: \t" << fixed << setprecision(2) << (time/1000) << " Seconds\n";
        running->out << "Largest Stack: \t\t" << running->largestStack << endl;

        cout << running->prog << " Completed\n\n";
        running = NULL;
        LongTermScheduler();
}

//create instance of os and run it
int main(int argc, char *argv[])
{   
    if(argc != 2)
    {
        cerr << "must have 2 arguments\n";
        exit(1);
    }

    OS os(argv[1]);

    os.LoadProcess();
	
    cout << "Running...\n\n";

    os.Run();

    return 0;
}



