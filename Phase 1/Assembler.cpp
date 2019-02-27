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

/*
    //keep track of line count for error reporting
    readLine = 1;
    getline(inputFile,line);  
 
    //run till end of file
    while(!inputFile.eof())
    {
        opcode = "";
        rd = -1;
        rs = -1;
        instruction = 0;
        istringstream is(line.c_str()); //uses sstream header
        is >> opcode >> rd >> rs;
      
        if(line[0] == '!' || line.empty())
            goto end;  
        
        //RD RS --- I = 0
        if(rd >= 0 && rd <= 3 && rs >= 0 && rs <= 3 && 
            opcode == "add" || opcode == "addc" ||
            opcode == "sub" || opcode == "subc" ||
            opcode == "and" || opcode == "xor" ||
            opcode == "compr")
        {
            instruction += opcodes[opcode] << 11;
            instruction += rd << 9;
            instruction += rs << 6;
        }

        //RD --- I = d = 0
        else if(rd >= 0 && rd <= 3 && opcode == "compl" ||
            opcode == "shl" || opcode == "shla" ||
            opcode == "shr" || opcode == "shra" ||
            opcode == "getstat" || opcode == "putstat" ||
            opcode == "read" || opcode == "write")
        {
            instruction += opcodes[opcode] << 11;
            instruction += rd << 9;
        }

        //RD ADDR --- I = 0
        else if(rd >= 0 && rd <= 3 && rs >= 0 && rs < 256 && opcode == "load")     //Restrictions for rd and rs says anything besides 0,1,2,3 are illegal and should give an error message
        {
            instruction += opcodes[opcode] << 11;
            instruction += rd << 9;
            instruction += rs; 
        }

        //RD ADDR --- I = 1
        else if(rd >= 0 && rd <= 3 & rs >= 0 && rs < 256 && opcode == "store")
        {
            instruction += opcodes[opcode] << 11;
            instruction += rd << 9;
            instruction += 1 << 8;
            instruction += rs;
        }
        
        //RD CONST --- I = 1
        else if (rd >= 0 && rd <= 3 && rs >= -128 && rs < 128 &&
            opcode == "loadi" || opcode == "addi" || 
            opcode == "addci" || opcode == "subi" || 
            opcode == "subci" || opcode == "andi" || 
            opcode == "xori"  || opcode == "compri")
        {
            instruction += opcodes[opcode] << 11;
            instruction += rd << 9;
            instruction += 1 << 8;
            instruction += rs;
        }

        //ADDR --- I = 1
        else if (rd >= 0 && rd < 256 && opcode == "jump" ||
            opcode == "jumpl" || opcode == "jumpe" ||
            opcode == "jumpg" || opcode == "call")
        {
            instruction += opcodes[opcode] << 11;
            instruction += 1 << 8;
            instruction += rd;
        }

        //Just op code --- I = d = 0
        else if (rd < 0 && rs < 0 && opcode == "return" ||
            opcode == "halt" || opcode == "noop")
        {
            instruction += opcodes[opcode] << 11;
        }

        //display error if bad syntax
        else
        {
            cerr << "Syntax Error: #" << readLine << " Invalid opcode (" 
                 << line << ")" << endl;   
        }

        //print input line and output instruction (test only)
        if ( instruction != 0)
            cout << line << "\t\t" << instruction << endl;
       
        end:
        readLine++;
        getline(inputFile,line);    
    }

*/
