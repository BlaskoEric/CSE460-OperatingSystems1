/*********************************************************************************************
 * Eric Blasko
 * Taylor Helms
 *
 * Assemebler.cpp
 *
 * 10/31/2018
 *
 * This program reads in assembley language and converts it to object code. ( in is '.s' file,
 * out is '.o' file). Based on a given opcode and up to 2 arguments, the program converts it
 * to a unique integer that can be used in the Virtual Machine. All ! comments and empty lines
 * are ignored when reading in from '.s' file. 
 *
 * ********************************************************************************************/
#include "Assembler.h"

//Creating map with valid opcodes and their associated value
Assembler::Assembler() 
{
    opcodes["load"] = &Assembler::load;
    opcodes["loadi"] = &Assembler::loadi;
    opcodes["store"] = &Assembler::store;
    opcodes["add"] = &Assembler::add;
    opcodes["addi"] = &Assembler::addi;
    opcodes["addc"] = &Assembler::addc;
    opcodes["addci"] = &Assembler::addci;
    opcodes["sub"] = &Assembler::sub;
    opcodes["subi"] = &Assembler::subi;
    opcodes["subc"] = &Assembler::subc;
    opcodes["subci"] = &Assembler::subci;
    opcodes["and"] = &Assembler::and_;
    opcodes["andi"] = &Assembler::andi;
    opcodes["xor"] = &Assembler::xor_;
    opcodes["xori"] = &Assembler::xori;
    opcodes["compl"] = &Assembler::compl_;
    opcodes["shl"] = &Assembler::shl;
    opcodes["shla"] = &Assembler::shla;
    opcodes["shr"] = &Assembler::shr;
    opcodes["shra"] = &Assembler::shra;
    opcodes["compr"] = &Assembler::compr;
    opcodes["compri"] = &Assembler::compri;
    opcodes["getstat"] = &Assembler::getstat;
    opcodes["putstat"] = &Assembler::putstat;
    opcodes["jump"] = &Assembler::jump;
    opcodes["jumpl"] = &Assembler::jumpl;
    opcodes["jumpe"] = &Assembler::jumpe;
    opcodes["jumpg"] = &Assembler::jumpg;
    opcodes["call"] = &Assembler::call;
    opcodes["return"] = &Assembler::return_;
    opcodes["read"] = &Assembler::read;
    opcodes["write"] = &Assembler::write;
    opcodes["halt"] = &Assembler::halt;
    opcodes["noop"] = &Assembler::noop;
}

//reads in each line of '.s' file and calls a function based on its opcode.
//returns 0 on success, 1 if error accured
int Assembler::Assemble(fstream& inFile, fstream& outFile) 
{
    string line;
    string opcode;
    const int success = false;
    const int error = true;
    int instruction;

    getline(inFile,line);
    while(!inFile.eof())
    {
        istringstream str(line.c_str());
        str >> opcode;
        if(opcode[0] == '!')
        {
            getline(inFile, line);
            continue;
        }
        
        try
        {
            if (not opcodes[opcode])
                throw NullPointerException();
            else instruction = (this->*opcodes[opcode])(str);
        } 
        catch (NullPointerException e)
        {
            cerr << e.what() << endl;
            return error;
        }

        if (instruction == -1)
            return error;

        if(instruction <= 9 and instruction > 0)
        {
            outFile << "0000" << instruction << endl;
        }
        else if (instruction <= 99 and instruction > 9)
        {
            outFile << "000" << instruction << endl;;
        }
        else if (instruction <= 999 and instruction > 99)
        {
            outFile << "00" << instruction << endl;
        }
        else if (instruction <= 9999 and instruction > 999)
        {
            outFile << "0" << instruction << endl;
        }
        else
            outFile << instruction << endl;
 
        getline(inFile, line);
    }
    return success;
}

//RD ADDR
int Assembler::load(istringstream & str)
{
    int rd, addr;
    str >> rd >> addr;
    if (rd < 0 || rd > 3)
        return -1;
    if (addr < 0 || addr > 255)
        return -1;
    int instruction = 0;
    instruction = instruction << 11 | rd << 9 | addr;
    return instruction;
}

//RD CONST
int Assembler::loadi(istringstream & str)
{
    int rd, constant;
    str >> rd >> constant;
    if (rd < 0 || rd > 3)
        return -1;
    if (constant < -128 || constant > 127)
        return -1;
    int instruction = 0;
    instruction = instruction << 11 | rd << 9 | 1 << 8 | (0x000000ff & constant);
    return instruction;
}

//RD ADDR
int Assembler::store(istringstream & str)
{
    int rd, addr;
    str >> rd >> addr;
    if (rd < 0 || rd > 3)
        return -1;
    if (addr < 0 || addr > 255)
        return -1;
    int instruction = 1;
    instruction = instruction << 11 | rd << 9 | 1 << 8 |  addr;
    return instruction;
}

//RD RS
int Assembler::add(istringstream & str)
{
    int rd, rs;
    str >> rd >> rs;
    if (rd < 0 || rd > 3)
        return -1;
    if (rs < 0 || rs > 3)
        return -1;
    int instruction = 2;
    instruction = instruction << 11 | rd << 9 | rs << 6;
    return instruction;
}

//RD CONST
int Assembler::addi(istringstream & str)
{
    int rd, constant;
    str >> rd >> constant;
    if (rd < 0 || rd > 3)
        return -1;
    if (constant < -128 || constant > 127)
        return -1;
    int instruction = 2;
    instruction = instruction << 11 | rd << 9 | 1 << 8 | (0x000000ff & constant);
    return instruction;
}

//RD RS
int Assembler::addc(istringstream & str)
{
    int rd, rs;
    str >> rd >> rs;
    if (rd < 0 || rd > 3)
        return -1;
    if (rs < 0 || rs > 3)
        return -1;
    int instruction = 3;
    instruction = instruction << 11 | rd << 9 | rs << 6;
    return instruction;

}

//RD CONST
int Assembler::addci(istringstream & str)
{
    int rd, constant;
    str >> rd >> constant;
    if (rd < 0 || rd > 3)
        return -1;
    if (constant < -128 || constant > 127)
        return -1;
    int instruction = 3;
    instruction = instruction << 11 | rd << 9 | 1 << 8 | (0x000000ff & constant);
    return instruction;
}

//RD RS
int Assembler::sub(istringstream & str)
{
    int rd, rs;
    str >> rd >> rs;
    if (rd < 0 || rd > 3)
        return -1;
    if (rs < 0 || rs > 3)
        return -1;
    int instruction = 4;
    instruction = instruction << 11 | rd << 9 | rs << 6;
    return instruction;
}

//RD CONST
int Assembler::subi(istringstream & str)
{
    int rd, constant;
    str >> rd >> constant;
    if (rd < 0 || rd > 3)
        return -1;
    if (constant < -128 || constant > 127)
        return -1;
    int instruction = 4;
    instruction = instruction << 11 | rd << 9 | 1 << 8 | (0x000000ff & constant);
    return instruction;
}

//RD RS
int Assembler::subc(istringstream & str)
{
    int rd, rs;
    str >> rd >> rs;
    if (rd < 0 || rd > 3)
        return -1;
    if (rs < 0 || rs > 3)
        return -1;
    int instruction = 5;
    instruction = instruction << 11 | rd << 9 | rs << 6;
    return instruction;

}

//RD CONST
int Assembler::subci(istringstream & str)
{
    int rd, constant;
    str >> rd >> constant;
    if (rd < 0 || rd > 3)
        return -1;
    if (constant < -128 || constant > 127)
        return -1;
    int instruction = 5;
    instruction = instruction << 11 | rd << 9 | 1 << 8 | (0x000000ff & constant);
    return instruction;

}

//RD RS
int Assembler::and_(istringstream & str)
{
    int rd, rs;
    str >> rd >> rs;
    if (rd < 0 || rd > 3)
        return -1;
    if (rs < 0 || rs > 3)
        return -1;
    int instruction = 6;
    instruction = instruction << 11 | rd << 9 | rs << 6;
    return instruction;

}

//RD CONST
int Assembler::andi(istringstream & str)
{
    int rd, constant;
    str >> rd >> constant;
    if (rd < 0 || rd > 3)
        return -1;
    if (constant < -128 || constant > 127)
        return -1;
    int instruction = 6;
    instruction = instruction << 11 | rd << 9 | 1 << 8 | (0x000000ff & constant);
    return instruction;
}

//RD RS
int Assembler::xor_(istringstream & str)
{
    int rd, rs;
    str >> rd >> rs;
    if (rd < 0 || rd > 3)
        return -1;
    if (rs < 0 || rs > 3)
        return -1;
    int instruction = 7;
    instruction = instruction << 11 | rd << 9 | rs << 6;
    return instruction;

}

//RD CONST
int Assembler::xori(istringstream & str)
{
    int rd, constant;
    str >> rd >> constant;
    if (rd < 0 || rd > 3)
        return -1;
    if (constant < -128 || constant > 127)
        return -1;
    int instruction = 7;
    instruction = instruction << 11 | rd << 9 | 1 << 8 | (0x000000ff & constant);
    return instruction;
}

//RD
int Assembler::compl_(istringstream & str)
{
    int rd;
    str >> rd;
    if (rd < 0 || rd > 3)
        return -1;
    int instruction = 8;
    instruction = instruction << 11 | rd << 9;
    return instruction;
}

//RD
int Assembler::shl(istringstream & str)
{
    int rd;
    str >> rd;
    if (rd < 0 || rd > 3)
        return -1;
    int instruction = 9;
    instruction = instruction << 11 | rd << 9;
    return instruction;
}

//RD
int Assembler::shla(istringstream & str)
{
    int rd;
    str >> rd;
    if (rd < 0 || rd > 3)
        return -1;
    int instruction = 10;
    instruction = instruction << 11 | rd << 9;
    return instruction;
}

//RD
int Assembler::shr(istringstream & str)
{   
    int rd;
    str >> rd;
    if (rd < 0 || rd > 3)
        return -1;
    int instruction = 11;
    instruction = instruction << 11 | rd << 9;
    return instruction;
}

//RD
int Assembler::shra(istringstream & str)
{
    int rd;
    str >> rd;
    if (rd < 0 || rd > 3)
        return -1;
    int instruction = 12;
    instruction = instruction << 11 | rd << 9;
    return instruction;
}

//RD RS
int Assembler::compr(istringstream & str)
{
    int rd, rs;
    str >> rd >> rs;
    if (rd < 0 || rd > 3)
        return -1;
    if (rs < 0 || rs > 3)
        return -1;
    int instruction = 13;
    instruction = instruction << 11 | rd << 9 | rs << 6;
    return instruction;

}

//RD CONST
int Assembler::compri(istringstream & str)
{
    int rd, constant;
    str >> rd >> constant;
    if (rd < 0 || rd > 3)
        return -1;
    if (constant < -128 || constant > 127)
        return -1;
    int instruction = 13;
    instruction = instruction << 11 | rd << 9 | 1 << 8 | (0x000000ff & constant);
    return instruction;
}

//RD
int Assembler::getstat(istringstream & str)
{
    int rd;
    str >> rd;
    if (rd < 0 || rd > 3)
        return -1;
    int instruction = 14;
    instruction = instruction << 11 | rd << 9;
    return instruction;
}

//RD
int Assembler::putstat(istringstream & str)
{
    int rd;
    str >> rd;
    if (rd < 0 || rd > 3)
        return -1;
    int instruction = 15;
    instruction = instruction << 11 | rd << 9;
    return instruction;
}

//ADDR
int Assembler::jump(istringstream & str)
{
    int addr;
    str >> addr;
    if (addr < 0 || addr > 255)
        return -1;
    int instruction = 16;
    instruction = instruction << 11 | 1 << 8 | addr;
    return instruction;
}

//ADDR
int Assembler::jumpl(istringstream & str)
{
    int addr;
    str >> addr;
    if (addr < 0 || addr > 255)
        return -1;
    int instruction = 17;
    instruction = instruction << 11 | 1 << 8 | addr;
    return instruction;
}

//ADDR
int Assembler::jumpe(istringstream & str)
{
    int addr;
    str >> addr;
    if (addr < 0 || addr > 255)
        return -1;
    int instruction = 18;
    instruction = instruction << 11 | 1 << 8 | addr;
    return instruction;
}

//ADDR
int Assembler::jumpg(istringstream & str)
{
    int addr;
    str >> addr;
    if (addr < 0 || addr > 255)
        return -1;
    int instruction = 19;
    instruction = instruction << 11 | 1 << 8 | addr;
    return instruction;
}

//ADDR
int Assembler::call(istringstream & str)
{
    int addr;
    str >> addr;
    if (addr < 0 || addr > 255)
        return -1;
    int instruction = 20;
    instruction = instruction << 11 | 1 << 8 | addr;
    return instruction;
}

//Just OP
int Assembler::return_(istringstream & str)
{
    int instruction = 21;
    instruction = instruction << 11;
    return instruction;
}

//RD
int Assembler::read(istringstream & str)
{
    int rd;
    str >> rd;
    if (rd < 0 || rd > 3)
        return -1;
    int instruction = 22;
    instruction = instruction << 11 | rd << 9;
    return instruction;
}

//RD
int Assembler::write(istringstream & str)
{
    int rd;
    str >> rd;
    if (rd < 0 || rd > 3)
        return -1;
    int instruction = 23;
    instruction = instruction << 11 | rd << 9;
    return instruction;
}

//Just Op
int Assembler::halt(istringstream & str)
{
    int instruction = 24;
    instruction = instruction << 11;
    return instruction;
}

//Just Op
int Assembler::noop(istringstream & str)
{
    int instruction = 25;
    instruction = instruction << 11;
    return instruction;
}

