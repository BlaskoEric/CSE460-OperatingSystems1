/**************************************************************************
*
* Eric Blasco
* Taylor Helms
*
* VirtualMachine.cpp
*
* 10/31/2018
*
* The VirtualMachine currently consist of a Virtual CPU and memeory. It 
* performs the fetch, decode and execute cycle until a interupt is encountered.
* All object code is loaded by the OS directly into memory. The OS also 
* manages which process is loaded into the Virtual Machine. VM will loop the
* Run fucntion until a interupt via timeslice, read, write, or other error 
* is encountered.
**************************************************************************/

#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <string>
#include <cstdlib>

#include "VirtualMachine.h"

using namespace std;

//default constructor. Initializes function pointers for VMjumpTable
VirtualMachine::VirtualMachine()
    :msize(256), rsize(4), clock(0)
{
	mem.reserve(msize);
	mem.resize(msize);
    r.reserve(rsize);
    r.resize(rsize);
    tlb = PageTable(32); 
	frameRegisters.resize(32);
	hits = 0;
    VMjumpTable[0] = &VirtualMachine::load;
	VMjumpTable[1] = &VirtualMachine::store;
	VMjumpTable[2] = &VirtualMachine::add;
	VMjumpTable[3] = &VirtualMachine::addc;
	VMjumpTable[4] = &VirtualMachine::sub;
	VMjumpTable[5] = &VirtualMachine::subc;
	VMjumpTable[6] = &VirtualMachine::and_;
	VMjumpTable[7] = &VirtualMachine::xor_;
	VMjumpTable[8] = &VirtualMachine::compl_;
	VMjumpTable[9] = &VirtualMachine::shl;
	VMjumpTable[10] = &VirtualMachine::shla;
	VMjumpTable[11] = &VirtualMachine::shr;
	VMjumpTable[12] = &VirtualMachine::shra;
	VMjumpTable[13] = &VirtualMachine::compr;
	VMjumpTable[14] = &VirtualMachine::getstat;
	VMjumpTable[15] = &VirtualMachine::putstat;
	VMjumpTable[16] = &VirtualMachine::jump;
	VMjumpTable[17] = &VirtualMachine::jumpl;
	VMjumpTable[18] = &VirtualMachine::jumpe;
	VMjumpTable[19] = &VirtualMachine::jumpg;
	VMjumpTable[20] = &VirtualMachine::call;
	VMjumpTable[21] = &VirtualMachine::return_;
	VMjumpTable[24] = &VirtualMachine::halt;
	VMjumpTable[25] = &VirtualMachine::noop;
}

//Performs loop of fetch, decode and execute. After each execution, there
//will be a check if a interupt has occured.
void VirtualMachine::run(int TIME_SLICE,fstream & in, fstream & out, bool lru)
{
    session = clock;

    int opcode, rd, i, rs, val;

    //infinite loop
    while(true)
    {
        sr = sr & 0x001F;                       //reseting sr
 
        if(IsInMem(pc) == false)                //checking if in physical
            return;
       
        clock += 4;                             //accessing tlb
       
         int phy = tlb.log_to_phys(pc);         //getting physical addr
        
        if(lru == true)                         //changing timestamp
        {
            int frame = phy / 8;
            frameRegisters[frame] = clock;
        }

	    ir = mem[phy];                        
        pc++;                        
        opcode = (ir & 0xf800) >> 11;
        rd = (ir & 0x600) >> 9;
        i = (ir & 0x100) >> 8;
        rs = (ir & 0xc0) >> 6;
        val = (ir & 0xff);  
 
        if (pc > limit or pc < base)            //check for out of bounds
        {
            sr |= 0x0040;
            return;
        }

        if (opcode >= 26)                       //invalid opcode
        {
            sr |= 0x00A0;
            return;
        }

        if (opcode == 22)                       //opcode == read
        {
            read(rd,i,rs,val,in);
            return;
        }
        else if (opcode == 23)                  //opcode == write
        { 
            write(rd,i,rs,val,out);
            return;
        }
        else                                    //all other opcodes
            (this->*VMjumpTable[opcode])(rd,i,rs,val);


        if((clock - session) >= TIME_SLICE)     //timeslice
        {
	    tlb.page_table[pc/8].modified = true;
            return; 
        }

        if((sr & 0x00E0) == 32)                 //halt
        {   
            return;
        }

        //returns if any are true
        if(isOutOfBounds() || isStackOverflow() || isStackUnderflow())
            return;

        //if store found logical address not in physical memory
        if(((sr >> 5) & 0x27) == 32)
            return;

	hits++;
    }

}

//checks if logical memory address is in physical memmory. If it is not
//set sr so that os knows which page to load in
bool VirtualMachine::IsInMem(int addr)
{
    int phy = tlb.log_to_phys(addr);
        if(phy == -1)
        {
            int page = addr / 8;
            sr |= page << 11;
            sr |= 0x0400;
        
            return false;            
        }
    else return true;
}

//returns bool if out of bounds accured
bool VirtualMachine::isOutOfBounds()
{
    if((sr & 0x00E0) == 64)
        return true;
    else
        return false;
}

//returns bool if stack overflow accured
bool VirtualMachine::isStackOverflow()
{
    if((sr & 0x00E0) == 96)
    {
        return true;
    }
    else
        return false;
}

//returns bool if stack underflow accured
bool VirtualMachine::isStackUnderflow()
{
    if((sr & 0x00E0) == 128)
    {
        return true;
    }
    else
        return false; 
}

//Check for carry, set sr if true
void VirtualMachine::setcarry(int rd) 
{
    if(r[rd] & 0x10000)               
        sr |= 1;                      
    else                              
        sr &= 0x001e;                 
}

//returns bool if carry accured
bool VirtualMachine::getcarry()
{
    if(sr & 1)
        return 1;              
    else    
        return 0;              
}

//Check Overflow
void VirtualMachine::CheckOverflow(int val1,int val2,int rd)
{
	if(val1 == val2 and val1 != (r[rd] & 0x8000) >> 15)
		sr |= 0x10;
	else
		sr &= 0xf;
}

//Sign extend
void VirtualMachine::SignExtend(int rd)
{
	if(r[rd] & 0x8000)
		r[rd] |= 0xffff0000;
	else
		r[rd] &= 0xffff;
}

//if load (val = addr), if loadi (val = constant)
//loads memory address or constant into rd
void VirtualMachine::load(int& rd, int& i, int& rs, int& val)
{
    if(i == 0)                                  //load
    {   
        if(IsInMem(val) == false)
        {
            pc--;
            return;
        }    
        clock += 1;
        int phys = tlb.log_to_phys(val);
        r[rd] = mem[phys];          
    }
    else                                        //loadi
    {
        if(val & 0x80)                    
            val |= 0xffffff00;            
        clock += 1;
        r[rd] = val;                      
    }
}

//mem[addr] = r[rd]
//loads value of r[rd] int memory address
void VirtualMachine::store(int& rd, int& i, int& rs, int& addr)
{
    if(IsInMem(addr) == false)
    {
        pc--;
        return;
    }

    clock += 4;
    int phys = tlb.log_to_phys(addr);
    tlb.page_table[addr/8].modified = true;
    mem[phys] = r[rd];             
}

//if add (r[rd] + r[rs]), if addi (r[rd] + constant)
//checks for carry, overflow, and sign extends if needed
void VirtualMachine::add(int& rd, int& i, int& rs, int& constant)
{
    clock += 1;
    int sign1 = (r[rd] & 0x8000) >> 15;
    int sign2;
    if(i == 0)                                  //add
    {
        sign2 = (r[rs] & 0x8000) >> 15;
        r[rd] = r[rd] + r[rs];                        
    }
    else                                        //addi
    {
        if(constant & 0x80)                     
            constant |= 0xffffff00;             
 
        sign2 = (constant & 0x8000) >> 15;
        r[rd] = r[rd] + constant;  
    }   
    setcarry(rd);
    CheckOverflow(sign1,sign2,rd);
    SignExtend(rd);
}

//if addc (r[rd] + r[rs] + carry), if addci (r[rd] + constant + carry)
//only adds carry if present. check for carry and sign extend if
//needed
void VirtualMachine::addc(int& rd, int& i, int& rs, int& constant)
{
    clock += 1;
    if(i == 0)                                  //addc
    {
	r[rd] = r[rd] + r[rs] + (sr & 1);
    }
    else                                        //addci
    {
        if(constant & 0x80)     
            constant |= 0xffffff00;
 	r[rd] = r[rd] + constant + (sr & 1);
    }

    setcarry(rd);
    SignExtend(rd);
}

//if sub (r[rd] - r[rs]), if subi (r[rd] - constant)
//checks for carry, overflow, and sign extends if needed
void VirtualMachine::sub(int& rd, int& i, int& rs, int& constant)
{
    clock += 1;

    int sign1 = (r[rd] & 0x8000) >> 15;
    int sign2;
    if(i == 0)                                  //sub 
    {
        sign2 = (r[rs] & 0x8000) >> 15;
        r[rd] = r[rd] - r[rs];
    }
    else                                        //subi
    {
        if(constant & 0x80)     
            constant |= 0xffffff00;
 
        sign2 = (constant & 0x8000) >> 15;
        r[rd] = r[rd] - constant;
    }

    setcarry(rd);
    CheckOverflow(sign1,sign2,rd);
    SignExtend(rd);
}

//if subc (r[rd] - r[rs] - carry), if subci (r[rd] - constant - carry)
//only subtracts carry if present. check for carry and sign extend if
//needed
void VirtualMachine::subc(int& rd, int& i, int& rs, int& constant)
{
    clock += 1;
    if(i == 0)                                  //subc
    {
        r[rd] = r[rd] - r[rs] - (sr & 1);  
    }
    else                                        //subci
    {
         if(constant & 0x80)       
            constant |= 0xffffff00;
       
        r[rd] = r[rd] - constant - (sr & 1);
    }

    setcarry(rd);
    SignExtend(rd);
}

//if and{r[rd] & r[rs]), if andi(r[rd] & constant)
//sign extend if needed
void VirtualMachine::and_(int& rd, int& i, int& rs, int& constant)
{
    clock += 1;
    if(i == 0)                                  //and
    {
        r[rd] = r[rd] & r[rs];
    }
    else                                        //andi
    {   
        if(constant & 0x80)
            constant |= 0xffffff00;
 
        r[rd] = r[rd] & constant;
    }
 
    SignExtend(rd);
}

//if xor(r[rd] xor r[rs]), if xori(r[rd] xor constant)
//sign extend if needed
void VirtualMachine::xor_(int& rd, int& i, int& rs, int& constant)
{
    clock += 1;
    if (i == 0)                                 //xor
    {
        r[rd] = r[rd] ^ r[rs];
    }
    else                                        //xori
    {
        if(constant & 0x80) 
            constant |= 0xffffff00;
 
        r[rd] = r[rd] ^ constant;
    }

    SignExtend(rd);
}

//r[rd] = !r[rd]. Takes complement of r[rd]. Sign extend if needed
void VirtualMachine::compl_(int& rd, int& i, int& rs, int& val)
{
    clock += 1;
    r[rd] = ~r[rd];                        

    SignExtend(rd);

}

//r[rd] = r[rd] << 1, shift in bit = 0, set carry. Sign extend
//if needed
void VirtualMachine::shl(int& rd, int& i, int& rs, int& val)
{
    clock += 1;
    r[rd] = r[rd] << 1;
    setcarry(rd);
    SignExtend(rd);
}

//shl arithmetic.Sign extend if needed
void VirtualMachine::shla(int& rd, int& i, int& rs, int& val)
{
    clock += 1;
    r[rd] <<= 1;
    if(r[rd] & 0x10000)
    {
        sr |= 1;
        r[rd] |= 0x8000;
    }
    else
    {
        sr &= 0x1e;
        r[rd] &= 0x7fff;
    }
    SignExtend(rd);
}

//r[rd] = r[rd] >> 1, shift in bit == 0. Set carry
//sign extend if needed
void VirtualMachine::shr(int& rd, int& i, int& rs, int& val)
{
    clock += 1;
    r[rd] &= 0xffff;
    if(r[rd] & 1)                     
        sr = sr | 1;                  
    else 
        sr &= 0x1e;
    r[rd] = r[rd] >> 1;               
}

//shr arithmetic.
void VirtualMachine::shra(int& rd, int& i, int& rs, int& val)
{
    clock += 1;
    if(r[rd] & 1)                      
        sr = sr | 1;
    else
        sr &= 0x1e;
    r[rd] = r[rd] >> 1;            
}

//compare...both compr and compri
void VirtualMachine::compr(int& rd, int& i, int& rs, int& constant)
{
    clock += 1;    

    if(i == 0)                                  //compr
    {
        if(r[rd] < r[rs])                       //if less
        {
            sr = sr | 8;                        //set less in sr 01000
            sr = sr & 0x0019;                   //and sr with 11001. clears equal/greater
        }
        else if(r[rd] == r[rs])                 //if equal
        {
            sr = sr | 4;                        //set equal in sr 00100
            sr = sr & 0x0015;                   //and sr with 10101. clears less/greater
        }
        else                                    //if greater
        {
            sr = sr | 2;                        //set greater in sr 00010
            sr = sr & 0x0013;                   //and sr with 10011
        }
    }
    else                                        //compri
    {
        if(constant & 0x80)      
            constant |= 0xffffff00;

        if(r[rd] < constant)                    //if less
        {
            sr = sr | 8;                        //set less in sr 01000
            sr = sr & 0x0019;                   //and sr with 11001. clears equal/greater
        }
        else if(r[rd] == constant)              //if equal
        {
            sr = sr | 4;                        //set equal in sr 00100
            sr = sr & 0x0015;                   //and sr with 10101. clears less/greater
        }
        else                                    //if greater
        {
            sr = sr | 2;                        //set greater in sr 00010
            sr = sr & 0x0013;                   //and sr with 10011
        }
    }
}

//r[rd] = SR. load r[rd] with status register
void VirtualMachine::getstat(int& rd, int& i, int& rs, int& val)
{
    clock += 1;
    r[rd] = sr;
}

//SR = r[rd]. load status register with r[rd]
void VirtualMachine::putstat(int& rd, int& i, int& rs, int& val)
{
    clock += 1;
    sr = r[rd];                              
}

//Check if addr + base is valid, if it is pc == addr.
//else return
void VirtualMachine::jump(int& rd, int& i, int& rs, int& addr)
{
    clock += 1;
        if(IsInMem(addr) == false)
        {
            pc--;
            return;
        }    
    pc = (addr); 
}

//if less in sr is true, check if addr + base is valid. If it
//is pc == addr else return
void VirtualMachine::jumpl(int& rd, int& i, int& rs, int& addr)
{
    clock += 1;
    if(sr & 8)             
    {
        if(IsInMem(addr) == false)
        {
            pc--;
            return;
        }    
        pc = (addr); 
    }                    
}

//if equal in sr is true, check if addr + base is valid. If it
//is pc == addr else return
void VirtualMachine::jumpe(int& rd, int& i, int& rs, int& addr)
{
    clock += 1;
    if(sr & 4)
    {
        if(IsInMem(addr) == false)
        {
            pc--;
            return;
        }    
        pc = (addr);
    }
}

//if greater in sr is true, check if addr + base is valid. If it
//is pc == addr else return
void VirtualMachine::jumpg(int& rd, int& i, int& rs, int& addr)
{
    clock += 1;
    if(sr & 2)
    {
        if(IsInMem(addr) == false)
        {
            pc--;
            return;
        }    
        pc = (addr);  
    }
}

//if addr + base is valid and sp is less that limit + 6,
//push vm status to stack and set pc to addr
void VirtualMachine::call(int& rd, int& i, int& rs, int& addr)
{
    clock += 1;

    if(sp < limit + 6)
    {
        sr |= 0x0060;
        return;
    }

    mem[--sp] = pc;                         
    
    for(int i = 0; i < 4; i++)              
        mem[--sp] = r[i];
    mem[--sp] = sr;                         
    pc = (addr);                     
}

//if stack pointer is less that memory size - 6, pop stack
//to registers
void VirtualMachine::return_(int& rd, int& i, int& rs, int& val)
{
    clock += 1;
    if(sp > 256-6)                              //underflow
    {
        sr |= 0x0080;
        return;
    }


    sr = mem[sp++];   
    
    for(int i = 3;i > -1; i--)
    {
        r[i] = mem[sp++];
    }

    pc = mem[sp++];      
}

//set I/O register and VM return status based on rd
void VirtualMachine::read(int& rd, int& i, int& rs, int& val,fstream& in)
{
    clock += 1;
    in >> r[rd];
    sr &= 0xfffff01f;
    sr |= 0xc0;
    sr |= rd<<8;
}


///set I/O register and VM return status based on rd
void VirtualMachine::write(int& rd, int& i, int& rs, int& val,fstream& out)
{
    clock += 1;
    out << r[rd] << endl;
    sr &= 0xfffff01f;
    sr |= 0xe0;
    sr |= rd<<8;
}

//halts execution
void VirtualMachine::halt(int& rd, int& i, int& rs, int& val)
{
    clock += 1;
    sr |= 0x0020;                                   
}

//Does nothing but increases clock
void VirtualMachine::noop(int& rd, int& i, int& rs, int& val)
{
    clock += 1;
}

//returns clock value
int VirtualMachine::get_clock()
{
	return clock;
}

//get ref to mem location
int& VirtualMachine::getMemory(int pos)
{
	return mem[pos];
}

//set limit 
void VirtualMachine::setLimit(int _limit)
{
	limit = _limit;
}


