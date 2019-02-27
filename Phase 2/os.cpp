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
OS::OS()
{
    idle_time = 0;
    context_time = 0;
    sys_time = 0;

    LoadProcess();
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
        cerr << "Error" << endl;
        exit(1);
    }

    string prog;
    int limit = 0;

    while(filesIn >> prog)
    {
        cout << "Loading " << prog << "..." << endl;
        fstream assemblyCode, objectCode;
        assemblyCode.open(prog.c_str(),ios::in);
        if(!assemblyCode)
        {
            cerr << "Error loading" << prog << endl;
            continue;
        }
   
        int pos = prog.find(".");
        string name = prog.substr(0,pos);
        string outputFile = name + ".o";
	    string inFile = name + ".in";
	    string outFile = name + ".out";
	    string st = name + ".st";
        
        objectCode.open(outputFile.c_str(),ios::out);
        if(!objectCode)
        {
            cerr << "Error loading" << prog << endl;
            continue;
        }

        as.Assemble(assemblyCode, objectCode);
       
        assemblyCode.close();
        objectCode.close();

        objectCode.open(outputFile.c_str(),ios::in);
        if(!objectCode)
        {
            cerr << "Error loading" << prog << endl;
            continue;
        }
         
        int base = limit;
    	while(!objectCode.eof())
	    {
	        objectCode >> vm.getMemory(limit++);
	    }
	   
        PCB * p = new PCB(name, base, limit - 1);
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

	    p->stack.open(st.c_str(),ios::in | ios::out);

        readyQ.push(p);
        jobs.push_back(p);

        objectCode.close(); 
    } 
    system("rm program");
}

//Deconstructor. Prints system information to each processes output file. After, each process is done
//its '.st' file is removed, all files are closed and PCB * is deleted
OS::~OS()
{
    cout << endl;
    double userClocks = 0;
    double processes = 0;
    for (auto const& i : jobs)
    {
        userClocks += i->cpu_time;
        processes++;
    }

    for (auto const& i : jobs) {

        double systemTime = ((double)(context_time) + (double)(idle_time)) / 1000.0;
        double cpuTime = (double)(vm.clock) / 1000.0;
        double cpuUtil = (double)(vm.clock - idle_time) / (double)(vm.clock) * 100;
        double userCpu = userClocks / (double)(vm.clock + context_time) * 100;
        double throughput = (double)(processes) / ((double)(sys_time) / 1000.0);


        i->out << "\n\tSystem Information\n";
        i->out << "System Time: \t\t" << fixed <<setprecision(3) << systemTime << " Seconds\n";
        i->out << "CPU Time: \t\t"  << cpuTime << " Seconds\n";
        i->out << "CPU Utilization: \t" << cpuUtil << "%\n";
        i->out << "User CPU Utilization: \t" << userCpu << "%\n";
        i->out << "Throughput: \t\t" << throughput << " Per Seconds\n"; 
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
           	vm.run(TIME_SLICE);
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
	running->wait_time += (sys_time - running->wait_time_begin);
    
    if(running->sp < 256)
    {
        running->stack.open(string(running->prog + ".st").c_str(),ios::in);
        int temp = running->sp;
        while(temp < 256)
            running->stack >> vm.mem[temp++];

        running->stack.close();
    }
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
    sys_time += (vm.clock - vm.session);

    if(running->sp < 256)
    {
        running->stack.open(string(running->prog + ".st").c_str(),ios::out);
        int stackSize = 0;
        int temp = running->sp;
        while(temp < 256)
        {
            running->stack << vm.mem[temp++] << endl;
            stackSize++;
        }

        if(running->largestStack < stackSize)
            running->largestStack = stackSize;
        
        running->stack.close();
    }   
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
    
    if((running->sr & 0x00E0) == 192 || (running->sr & 0x00E0) == 224)
    {
        if((running->sr & 0x00E0) == 192)                   //read
        {
            if((running->sr & 0x0300) == 0)
                running->in >> running->r[0];
            else if((running->sr & 0x0300) == 256)
                running->in >> running->r[1];
            else if((running->sr & 0x0300) == 512)
                running->in >> running->r[2];
            else if((running->sr & 0x0300) == 768)
               running->in >> running->r[3];
        }
        else                                                //write
        {
            if((running->sr & 0x0300) == 0)
                running->out << running->r[0] << endl;
            else if((running->sr & 0x0300) == 256)
                running->out << running->r[1] << endl;
            else if((running->sr & 0x0300) == 512)
                running->out << running->r[2] << endl;
            else if((running->sr & 0x0300) == 768)
               running->out <<  running->r[3] << endl;

        }
        
        waitQ.push(running);
        running->io_time_begin = sys_time;
    }
    else if((running->sr & 0x00E0) == 32)                   //halt
    {
        string message = "Successfully Completed";
        EndProcess(message);
    }
    else if((running->sr & 0x00E0) == 64)                   //out of bounds
    {
        string message = "Out of Bounds";
        EndProcess(message);
    }
    else if((running->sr & 0x00E0) == 96)                   //stack overflow
    {
        string message = "Stack Overflow Detected";
        EndProcess(message);
    }
    else if((running->sr & 0x00E0) == 128)                  //stack underflow
    {
        string message = "Stack Underflow Detected";
        EndProcess(message);
    }
    else if((running->sr & 0x00E0) == 160)                  //Ivalid Opcode
    {
        string message = "Invalid Opcodes";
        EndProcess(message);
    }
    else if((running->sr & 0x00E0) == 0)                    //Timeslice
    {
        running->wait_time_begin = sys_time;
        readyQ.push(running);
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

//if a process has ended, print its status to its output file
void OS::EndProcess(string message)
{
        double time = running->cpu_time + running->wait_time + running->io_time;

        running->out << "\n\tProcess Information\n";
        running->out << message << endl;
        running->out << "CPU Time: \t\t" << running->cpu_time << " Ticks\n";
        running->out << "Waiting Time: \t\t" << running->wait_time << " Ticks\n";
        running->out << "I/O Time: \t\t" << running->io_time << " Ticks\n";
        running->out << "Turnaround Time: \t" << fixed << setprecision(2) << (time/1000) << " Seconds\n";
        running->out << "Largest Stack: \t\t" << running->largestStack << endl;

        cout << running->prog << " Completed\n";
        running = NULL;
}

//create instance of os and run it
int main(int argc, char *argv[])
{
    OS os;
    cout << endl << "Running...\n" << endl;
    os.Run();
}



