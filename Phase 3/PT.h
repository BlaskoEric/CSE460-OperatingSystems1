/*********************************************************************************************
 * Eric Blasko
 * Taylor Helms
 *
 * PT.h
 *
 * 12/2/18
 *
 * This header contains three classes: Row, PageTable and InvertedPageTable. Each class contains
 * simple definitions which make up ether a page table or inverted page table. PageTable class
 * contains a function which converts logical addresses into physical address.
 *
 ************************************************************************************************/

#ifndef PT_H
#define PT_H

#include <vector>

using namespace std;

class OS;
class VirtualMachine;

class InvertedPageTable{
    string pid;
    int page;
    int timeStamp;
    friend class OS;
    friend class VirtualMachine;
};

class Row
{
    int frame;
    bool valid;
    bool modified;
    int timeStamp;

    friend class PageTable;
    friend class OS;
    friend class VirtualMachine;
};

class PageTable{
private:
    vector<Row> page_table;
    
public:
    PageTable() {};
    PageTable(int size)
    {
        page_table = vector<Row>(size);
        for( int i = 0; i < size; i++)
        {
            page_table[i].valid = false;    
            page_table[i].modified = false;
            page_table[i].timeStamp = 0;
        }
    }

    void operator=(const PageTable & p)
    {
        page_table = p.page_table;
    }
    
    void operator=(PageTable && p)
    {
        swap(page_table,p.page_table);
    }

    int log_to_phys(int addr)
    {
        int page_num = addr/8;
        if(page_table[page_num].valid){
            int frame = page_table[page_num].frame;
            int phys_addr = frame*8 + addr%8;
            return phys_addr;
        } else
            return -1;
    }
    friend
        class OS;
    friend
        class VirtualMachine;
};

#endif
