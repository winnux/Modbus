#include <iostream>
#include "modbus.h"

#define BOOST_DATE_TIME_NO_LIB
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/circular_buffer.hpp>
#include <boost/thread.hpp>

using namespace boost::interprocess;
using namespace std;
struct RAW_COMM_DATA
{
    uint8_t data[255];
    uint8_t length ;
};
boost::circular_buffer<RAW_COMM_DATA> rawCommDatas ;
bool bQuit = false ;

extern "C" {

void busMonitorAddItem( uint8_t isRequest, uint8_t slave, uint8_t func, uint16_t addr, uint16_t nb, uint16_t expectedCRC, uint16_t actualCRC )
{
    int n = slave ;int m=func ;
    cout<<n<<" "<<m<<" "<<addr<<" "<<nb<<endl;

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

void busMonitorRawData( uint8_t * data, uint8_t dataLen, uint8_t addNewline )
{
    RAW_COMM_DATA raw ;
    raw.length = dataLen ;
    memcpy(raw.data,data,dataLen) ;
    rawCommDatas.push_back(raw);
}

}
void workerThread()
{
    modbus_t *modbus;
    modbus = modbus_new_rtu("COM1",9600,'N',8,1);
    if( modbus_connect( modbus ) == -1 )
    {
        printf("Connect Com1 error");
        return  ;
    }
    uint8_t dest[1024];
    uint16_t * dest16 = (uint16_t *) dest;
    while(!bQuit)
    {
        memset( dest, 0, 1024 );

        modbus_set_slave( modbus, 1 );
        int ret = modbus_read_registers( modbus, 1, 5, dest16 );
        boost::this_thread::sleep(boost::posix_time::seconds(1));
    }
    modbus_close(modbus);
    modbus_free(modbus);
}

int main()
{
    cout << "Hello World!" << endl;
    boost::thread worker(workerThread);
    boost::this_thread::sleep(boost::posix_time::seconds(4));
    bQuit =true ;
    worker.join();
    return 0;
}

