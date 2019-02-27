/**************************************************************************
*Eric Blasco
*Taylor Helms
*
*VirtualMachine.cpp
*
*The VirtualMachine accepts an object code input from the Assembler
*and runs the correlating function which can update the registers,
*increment/decrement the program counter, and read/write files.
**************************************************************************/


#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <string>
#include <cstdlib>

#include "VirtualMachine.h"

using namespace std;

//default constructor
VirtualMachine::VirtualMachine()
    :msize(256), rsize(4), clock(0)
{
	mem.reserve(msize);
	mem.resize(msize);
    r.reserve(rsize);
    r.resize(rsize);
	stop = 0;
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

void VirtualMachine::run(fstream& in, fstream& out)
{
    int opcode, rd, i, rs, val;
    base = 0;
 
    //Set status register, stack pointer, and program counter
    sr = 2;
    sp = msize;
    pc = 0;

    //run until out of memory or halt
    while(pc < limit)
    {
        ir = mem[pc];                           //get current instruction
        pc++;                                   //get next instruction
        opcode = (ir & 0xf800) >> 11;           //break up instruction
        rd = (ir & 0x600) >> 9;
        i = (ir & 0x100) >> 8;
        rs = (ir & 0xc0) >> 6;
        val = (ir & 0xff);
        
        if (opcode == 22)                       //opcode == read
            read(rd,i,rs,val,in);
        else if (opcode == 23)                  //opcode == write
            write(rd,i,rs,val,out);
        else                                    //all other opcodes
            (this->*VMjumpTable[opcode])(rd,i,rs,val);
        
        if(stop)                                //if halt reached
            break;
    }
}

//adjust carry bit if needed
void VirtualMachine::setcarry(int rd) 
{
    if(r[rd] & 0x8000)                          //check if sign bit
        sr = sr | 1;                            //set carry bit
    else                                        //no sign bit
        sr = sr & 0x000e;                       //clear carry bit
}

//returns if carry bit is 1
bool VirtualMachine::getcarry()
{
    if(sr & 1)
        return 1;                               //return true
    else    
        return 0;                               //return false
}

//if load val = addr, if loadi val = constant
void VirtualMachine::load(int rd, int i, int rs, int val)
{
    if(i == 0)                                  //load
    {   
        clock += 1;
        r[rd] = mem[val];                       //load address into r[rd]
    }
    else                                        //loadi
    {
        if(val & 0x80)                          //check sign bit
            val |= 0xffffff00;                  //flip if negative
        clock += 1;
        r[rd] = val;                            //load constant into r[rd]
    }
}

//mem[addr] = r[rd]
void VirtualMachine::store(int rd, int i, int rs, int addr)
{
    clock += 4;
    mem[addr] = r[rd];                          //load r[rd] into memory address 
}


void VirtualMachine::add(int rd, int i, int rs, int constant)
{
    clock += 1;
    if(i == 0)                                  //add
    {
        //if two positive numbers == a negative number, set overflow
        if(r[rd] >= 0 && r[rs] >= 0 && (r[rd] +  r[rs] < 0))
            sr = sr | 0x0010;
        
        //if two negative numbers == a positive number, set overflow
        else if( r[rd] < 0 && r[rs] < 0 && (r[rd] + r[rs] >= 0))
            sr = sr | 0x0010;
    
        r[rd] += r[rs];                         //add r[rs] to r[rd]
        setcarry(rd);                           //check if carry
    }
    else                                        //addi
    {
        if(constant & 0x80)                     //check sign bit
            constant |= 0xffffff00;              //flip if negative
 
        //if two positive numbers == a negative number, set overflow
        if(r[rd] >= 0 && constant >= 0 && (r[rd] + constant < 0))
            sr = sr | 0x0010;

        //if two negative numbers == a positive number, set overflow   
        else if(r[rd] < 0 && constant < 0 && (r[rd] + constant >= 0))
            sr = sr | 0x0010;
    
        r[rd] += constant;                      //add constant to r[rd]
        setcarry(rd);                           //check if carry        
    }   
}

void VirtualMachine::addc(int rd, int i, int rs, int constant)
{
    clock += 1;
    if(i == 0)                                  //addc
    {
        //if two positive numbers == a negative number, set overflow
        if(r[rd] >= 0 && r[rs] >= 0 && (r[rd] +  r[rs] < 0))
            sr = sr | 0x0010;
        else if( r[rd] < 0 && r[rs] < 0 && (r[rd] + r[rs] >= 0))
            sr = sr | 0x0010;
    
        if(getcarry())                          //if carry
        {
            r[rd] += r[rs] + 1;                 //add r[rs] + 1 to r[rd]
        }
        else                                    //no carry
        {
            r[rd] += r[rs];                     //add r[rs] to r[rd]
        }
        setcarry(rd);                           //check if carry
    }
    else                                        //addci
    {
        if(constant & 0x80)                     //check sign bit
            constant |= 0xffffff00;             //flip if negative
 
        //if two positive numbers == a negative number, set overflow
        if(r[rd] >= 0 && constant >= 0 && (r[rd] + constant < 0))
            sr = sr | 0x0010;
        else if(r[rd] < 0 && constant < 0 && (r[rd] + constant >= 0))
            sr = sr | 0x0010;
    
        if(getcarry())                          //if carry bit
        {
            r[rd] += constant + 1;              //add const + 1 to r[rd]
        }
        else                                    //if no carry bit
        {
            r[rd] += constant;                  //add const to r[rd]
        }
        setcarry(rd);                           //set carry
    }

}

void VirtualMachine::sub(int rd, int i, int rs, int constant)
{
    clock += 1;
    if(i == 0)                                  //sub 
    {
        //if two positive numbers == a negative number, set overflow
        if(r[rd] >= 0 && r[rs] >= 0 && (r[rd] +  r[rs] < 0))
            sr = sr | 0x0010;
        else if( r[rd] < 0 && r[rs] < 0 && (r[rd] + r[rs] >= 0))
            sr = sr | 0x0010;
    
        r[rd] -= r[rs];                         //sub r[rs] from r[rd]
        setcarry(rd);                           //set carry
    }
    else                                        //subi
    {
        if(constant & 0x80)                     //check sign bit
            constant |= 0xffffff00;             //flip if negative
 
        //if two positive numbers == a negative number, set overflow
        if(r[rd] >= 0 && constant >= 0 && (r[rd] + constant < 0))
            sr = sr | 0x0010;
        else if(r[rd] < 0 && constant < 0 && (r[rd] + constant >= 0))
            sr = sr | 0x0010;
    
        r[rd] -= constant;                      //sub const from r[rd]
        setcarry(rd);                           //set carry
    }
}

void VirtualMachine::subc(int rd, int i, int rs, int constant)
{
    clock += 1;
    if(i == 0)                                  //subc
    {
        //if two positive numbers == a negative number, set overflow
        if(r[rd] >= 0 && r[rs] >= 0 && (r[rd] +  r[rs] < 0))
            sr = sr | 0x0010;
        else if( r[rd] < 0 && r[rs] < 0 && (r[rd] + r[rs] >= 0))
            sr = sr | 0x0010;
    
        if(getcarry())                          //if carry bit
        {
            r[rd] -= r[rs] - 1;                 //sub r[rs] -1 from r[rd]
        }
        else                                    //no carry bit
        {
            r[rd] -= r[rs];                     //sub r[rs] from r[rd]
        }
        setcarry(rd);                           //set carry
    }
    else                                        //subci
    {
        if(constant & 0x80)                     //check sign bit
            constant |= 0xffffff00;             //flip if negative
 
        //if two positive numbers == a negative number, set overflow
        if(r[rd] >= 0 && constant >= 0 && (r[rd] + constant < 0))
            sr = sr | 0x0010;
        else if(r[rd] < 0 && constant < 0 && (r[rd] + constant >= 0))
            sr = sr | 0x0010;
    
        if(getcarry())                          //if carry bit
        {
            r[rd] -= constant - 1;              //sub const -1 from r[rd]
        }
        else                                    //no carry bit
        {
            r[rd] -= constant;                  //sub const from r[rd]
        }
        setcarry(rd);                           //set carry
    }
}

void VirtualMachine::and_(int rd, int i, int rs, int constant)
{
    clock += 1;
    if(i == 0)                                  //and
    {
        r[rd] = r[rd] & r[rs];
    }
    else                                        //andi
    {   
        if(constant & 0x80)                     //check sign bit
            constant |= 0xffffff00;             //flip if negative
 
        r[rd] = r[rd] & constant;
    }
}

void VirtualMachine::xor_(int rd, int i, int rs, int constant)
{
    clock += 1;
    if (i == 0)                                 //xor
    {
        r[rd] = r[rd] ^ r[rs];
    }
    else                                        //xori
    {
        if(constant & 0x80)                     //check sign bit
            constant |= 0xffffff00;             //flip if negative
 
        r[rd] = r[rd] ^ constant;
    }
}

//r[rd] = !r[rd]
void VirtualMachine::compl_(int rd, int i, int rs, int val)
{
    clock += 1;
    r[rd] = ~r[rd];                             //complement r[rd]
}

//r[rd] = r[rd] << 1, shift in bit = 0, set carry
void VirtualMachine::shl(int rd, int i, int rs, int val)
{
    clock += 1;
    r[rd] = r[rd] << 1;                         //shift left one
    setcarry(rd);                               //set carry
}

//shl arithmetic
void VirtualMachine::shla(int rd, int i, int rs, int val)
{
    clock += 1;
    if(r[rd] < 0)                               //is negative?
    {
        r[rd] = r[rd] << 1;                     //shift left one
        r[rd] = r[rd] | 0x8000;                 //flip sign bit
    }
    else                                        //greater than 0
    {
        r[rd] = r[rd] << 1;                     //shift left one
        r[rd] = r[rd] | 0x7fff;                 //clear sign bit
    }
    setcarry(rd);                               //set carry
}

//r[rd] = r[rd] >> 1, shift in bit == 0. Set carry
void VirtualMachine::shr(int rd, int i, int rs, int val)
{
    clock += 1;
    if(r[rd] & 1)                               //check if 1st bit is 1
        sr = sr | 1;                            //set carry
    
    r[rd] = r[rd] >> 1;                         //shift right 1
}

//shr arithmetic
void VirtualMachine::shra(int rd, int i, int rs, int val)
{
    clock += 1;
    if(r[rd] & 1)                               //check if 1st bit is 1
        sr = sr | 1;
    if(r[rd] < 0)                               //if value is negative
    {    
        r[rd] = r[rd] >> 1;                     //shift bits right 1
        r[rd] = r[rd] | 0x8000;                 //flip sign bit
    }
    else 
        r[rd] = r[rd] >> 1;                     //shift bits right 1
}

//compare...both compr and compri
void VirtualMachine::compr(int rd, int i, int rs, int constant)
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
        if(constant & 0x80)                     //if contains 1000000
            constant |= 0xffffff00;             //sign extend.

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

//r[rd] = SR
void VirtualMachine::getstat(int rd, int i, int rs, int val)
{
    clock += 1;
    r[rd] = sr;                                 //put sr into r[rd]
}

//SR = r[rd]
void VirtualMachine::putstat(int rd, int i, int rs, int val)
{
    clock += 1;
    sr = r[rd];                                 //pur rd status in sr
}

//PC == ADDR
void VirtualMachine::jump(int rd, int i, int rs, int addr)
{
    clock += 1;
    pc = addr;                                  //load pc to address
}

//if Less == 1, pc = ADDR, else do nothing
void VirtualMachine::jumpl(int rd, int i, int rs, int addr)
{
    clock += 1;
    if(sr & 8)                                  //if sr contains 01000
        pc = addr;                              //load pc to address
}

//If Equal == 1, pc = ADDR, else do nothing
void VirtualMachine::jumpe(int rd, int i, int rs, int addr)
{
    clock += 1;
    if(sr & 4)                                  //if sr contains 00100
        pc = addr;                              //load pc to address
}

//if greater == 1, pc = ADDR, else do nothing
void VirtualMachine::jumpg(int rd, int i, int rs, int addr)
{
    clock += 1;
    if(sr & 2)                                  //if sr contains 00010
        pc = addr;                              //load pc to address

}

//push VM status; pc == ADDR
void VirtualMachine::call(int rd, int i, int rs, int addr)
{
    clock += 1;
    mem[--sp] = pc;                             //push pc
    
    for(int i = 0; i < 4; i++)                  //pushing r[0] - r[3]
        mem[--sp] = r[i];

    mem[--sp] = sr;                             //pushing sp
    pc = addr;                                  //loading pc o address
}

//pop and restore VM status
void VirtualMachine::return_(int rd, int i, int rs, int val)
{
    clock += 1;
    sr = mem[sp++];                             //pop sr
    
    for(int i = 3;i > -1; i--)
    {
        r[i] = mem[sp++];                       //popping r[0] - r[3]
    }

    pc = mem[sp++];                             //popping pc
}

//read from .in file
void VirtualMachine::read(int rd, int i, int rs, int val,fstream& inFile)
{
    clock += 28;
    inFile >> r[rd];                            //read to rd
}

//write to .out file
void VirtualMachine::write(int rd, int i, int rs, int val,fstream& outFile)
{
    clock += 28;
    outFile << r[rd] << endl;                   //write rd to .in file
}

//halts execution
void VirtualMachine::halt(int rd, int i, int rs, int val)
{
    clock += 1;
    stop = 1;                                   //set stop to true
}

//Does nothing but increases clock
void VirtualMachine::noop(int rd, int i, int rs, int val)
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


