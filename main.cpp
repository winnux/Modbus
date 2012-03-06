#include <iostream>
#include <vector>
#include <exception>
#include "modbus.h"
#include "modbus-private.h"
#include "config.h"
#ifndef _MSC_VER
#include <signal.h>
#endif
#define BOOST_DATE_TIME_NO_LIB
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/circular_buffer.hpp>
#include <boost/thread.hpp>

using namespace boost::interprocess;
using namespace std;

char * byteToHexMap[256] = {
  "00", "01", "02", "03", "04", "05", "06", "07", "08", "09",
  "0a", "0b", "0c", "0d", "0e", "0f", "10", "11", "12", "13",
  "14", "15", "16", "17", "18", "19", "1a", "1b", "1c", "1d",
  "1e", "1f", "20", "21", "22", "23", "24", "25", "26", "27",
  "28", "29", "2a", "2b", "2c", "2d", "2e", "2f", "30", "31",
  "32", "33", "34", "35", "36", "37", "38", "39", "3a", "3b",
  "3c", "3d", "3e", "3f", "40", "41", "42", "43", "44", "45",
  "46", "47", "48", "49", "4a", "4b", "4c", "4d", "4e", "4f",
  "50", "51", "52", "53", "54", "55", "56", "57", "58", "59",
  "5a", "5b", "5c", "5d", "5e", "5f", "60", "61", "62", "63",
  "64", "65", "66", "67", "68", "69", "6a", "6b", "6c", "6d",
  "6e", "6f", "70", "71", "72", "73", "74", "75", "76", "77",
  "78", "79", "7a", "7b", "7c", "7d", "7e", "7f", "80", "81",
  "82", "83", "84", "85", "86", "87", "88", "89", "8a", "8b",
  "8c", "8d", "8e", "8f", "90", "91", "92", "93", "94", "95",
  "96", "97", "98", "99", "9a", "9b", "9c", "9d", "9e", "9f",
  "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7", "a8", "a9",
  "aa", "ab", "ac", "ad", "ae", "af", "b0", "b1", "b2", "b3",
  "b4", "b5", "b6", "b7", "b8", "b9", "ba", "bb", "bc", "bd",
  "be", "bf", "c0", "c1", "c2", "c3", "c4", "c5", "c6", "c7",
  "c8", "c9", "ca", "cb", "cc", "cd", "ce", "cf", "d0", "d1",
  "d2", "d3", "d4", "d5", "d6", "d7", "d8", "d9", "da", "db",
  "dc", "dd", "de", "df", "e0", "e1", "e2", "e3", "e4", "e5",
  "e6", "e7", "e8", "e9", "ea", "eb", "ec", "ed", "ee", "ef",
  "f0", "f1", "f2", "f3", "f4", "f5", "f6", "f7", "f8", "f9",
  "fa", "fb", "fc", "fd", "fe", "ff",
};

struct RAW_COMM_DATA
{
    uint8_t data[255];
    uint8_t length ;
};
boost::circular_buffer<RAW_COMM_DATA> rawCommDatas ;
bool bQuit = false ;

Config config ;
vector < boost::shared_ptr<boost::thread> > threads ;

extern "C" {

//void busMonitorAddItem( uint8_t isRequest, uint8_t slave, uint8_t func, uint16_t addr, uint16_t nb, uint16_t expectedCRC, uint16_t actualCRC )
void busMonitorSendData(uint8_t *data,uint8_t dataLen)
{
    for(uint8_t i = 0 ; i< dataLen; i++)
    {
        cout<<byteToHexMap[data[i]]<<" ";
    }
    cout<<endl;

    //try{

    //	shared_memory_object sharedMem(open_or_create,"Test",read_write);
    //	sharedMem.truncate(256);
    //	mapped_region mmap(sharedMem,read_write);
    //
    //	memcpy(mmap.get_address(),&n,4);
    //	memcpy((char*)(mmap.get_address())+4,&m,4);
    //	memcpy((char*)(mmap.get_address())+8,&addr,sizeof(uint16_t));
    //	memcpy((char*)(mmap.get_address())+8+sizeof(uint16_t),&nb,sizeof(uint16_t));

    //	shared_memory_object::remove("Test");
    //}
    //catch(interprocess_exception &e)
    //{
    //	/*shared_memory_object::remove("Test");*/
    //	cout<<e.what()<<endl;
    //}
}

void busMonitorRecvData( uint8_t * data, uint8_t dataLen, uint8_t addNewline )
{
    RAW_COMM_DATA raw ;
    raw.length = dataLen ;
    memcpy(raw.data,data,dataLen) ;
    rawCommDatas.push_back(raw);
    for(uint8_t i = 0 ; i< dataLen; i++)
    {
        cout<<byteToHexMap[data[i]]<<" ";
    }
    if(addNewline)
        cout<<endl ;
}

}
#if defined(_WIN32)
BOOL WINAPI controlHandler(DWORD t)
{
    if(t==CTRL_BREAK_EVENT||t==CTRL_C_EVENT)
        bQuit = true ;
    for(size_t i = 0 ;i < threads.size();i++)
        threads[i]->join();
    exit(1);
    return true ;
}

#else
void controlHandler(int)
{
    bQuit = true ;
    for(size_t i = 0 ;i < threads.size();i++)
        threads[i]->join();
    exit(1);
}

#endif


void workerThread(void* p)
{
    Bus *bus = (Bus*)p ;
    modbus_t *modbus;
    //modbus = modbus_new_rtu("/dev/ttyS0",9600,'N',8,1);
    modbus = modbus_new_rtu(bus->sPort,bus->baud,bus->parity,bus->databits,bus->stopbits);
    if( modbus_connect( modbus ) == -1 )
    {
        cout<<"connect "<<bus->sPort<<" error"<<endl ;
        return  ;
    }
    uint8_t dest[1024];
    uint16_t * dest16 = (uint16_t *) dest;
    while(!bQuit)
    {
        int ret ;
        memset( dest, 0, 1024 );

        for(size_t i = 0 ;i < bus->modules.size();i++)
        {
            for(size_t j = 0 ; j<bus->modules[i].reqs.size();j++)
            {
                modbus_set_slave( modbus, bus->modules[i].addr );
                switch(bus->modules[i].reqs[j].reqType)
                {
                case _FC_READ_COILS:
                    ret = modbus_read_bits( modbus, bus->modules[i].reqs[j].reg, bus->modules[i].reqs[j].num, dest );
                    break;
                case _FC_READ_DISCRETE_INPUTS:
                    ret = modbus_read_input_bits( modbus, bus->modules[i].reqs[j].reg, bus->modules[i].reqs[j].num, dest );
                    break;
                case _FC_READ_HOLDING_REGISTERS:
                    ret = modbus_read_registers( modbus, bus->modules[i].reqs[j].reg, bus->modules[i].reqs[j].num, dest16 );
                    break;
                case _FC_READ_INPUT_REGISTERS:
                    ret = modbus_read_input_registers( modbus, bus->modules[i].reqs[j].reg, bus->modules[i].reqs[j].num, dest16 );
                    break;
                }


                boost::this_thread::sleep(boost::posix_time::milliseconds(10));
            }
        }
    }
    modbus_close(modbus);
    modbus_free(modbus);
}
void breakHandle()
{
#if defined(_WIN32)
    SetConsoleCtrlHandler(controlHandler, true);
#else
    struct sigaction act;
    memset (&act, '\0', sizeof(act));
    act.sa_handler = &controlHandler;
    if (sigaction(SIGINT, &act, NULL) < 0) {
            cout<<"set break handler sig error"<<endl;
    }
#endif
}

int main()
{
    breakHandle();
    try{
        config.load();
    }catch(exception &e)
    {
        cout<<e.what()<<endl ;
        return 1 ;
    }

    cout<<config.bus_number<<endl ;

    for(int i = 0 ;i < config.bus_number ;i++)
    {
        boost::shared_ptr<boost::thread> thread(new boost::thread (boost::bind(workerThread,&config.busLines[0])));
        threads.push_back(thread);
    }

    char s = 0;
    while(s!='q')
    {
        cin>>s  ;
        boost::this_thread::sleep(boost::posix_time::milliseconds(100));
    }
    bQuit = true ;
    for(size_t i = 0 ;i < threads.size();i++)
        threads[i]->join();
    return 0;
}

