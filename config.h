#ifndef CONFIG_H
#define CONFIG_H

#if defined(_WIN32)
#include <Windows.h> //for getmodulefilename and NULL
#endif
#include "ezlogger/ezlogger_headers.hpp"
#include <string.h>
#include <vector>
using namespace std;
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>

#define MAX_YX_NUM  10000
#define MAX_YC_NUM  10000
#define MAX_DD_NUM  1024

#define YX_VAR_LEN  2
#define YC_VAR_LEN  5
#define DD_VAR_LEN  5

#define TOTAL_MEM_REQ   (MAX_YX_NUM*YX_VAR_LEN+MAX_YC_NUM*YC_VAR_LEN+MAX_DD_NUM*DD_VAR_LEN)

#define MAX_CACHE_COMMDATA_NUM   16

enum PowerDataType{yx,yc,dd,END};
struct Parse
{
public:
    int dataNums ;       //数据个数
    int powerType ;      //数据电力系统类型（1-遥信；2-遥测；3-电度）
    int startIndex ;     //数据起始地址（在本桢内相对偏移）
    int dataSize ;       //数据位宽(1,8,16,32)
    int dataOrder ;      //数据字节顺序(位宽为16，32时标明数据高低字节顺序,可取值4321、3412、1234、2143、12、21）
    int dataType ;       //数据类型（1,2,3分别代表有符号、无符号整形，浮点数）
    float  baseVar ;     //基值
    float  mulVar ;      //乘数
    float  deadBand ;    //死区
};
class ConfObjectException : public std::exception
{

public:
  ConfObjectException(int lo,int code) throw() {
      exception_location = lo ;
      exception_code = code ;

  }

  enum{bus,mod,req,other};
private:
    int     exception_location ;

    int     exception_code ;
public:

    virtual const char* what() const throw()
    {
        char sz[256] ;
        switch(exception_location)
        {
        case bus:
            sprintf(sz,"总线-");
            break;
        case mod:
            sprintf(sz,"设备-");
            break;
        case req:
            sprintf(sz,"命令-");
            break;
         default:
            sprintf(sz,"一般性错误-");
            break;
        }
        switch(exception_code)
        {
        case 1:
            strcat(sz,"最多支持16条总线");
            break;
        case 2:
            strcat(sz,"遥信、遥测、电度数据总数越界");
            break;
        case 100:
            strcat(sz,"串口参数错误");
            break;
        case 1000:
            strcat(sz,"模块号只能在1-63之间");
            break;
        case 10000:
            strcat(sz,"命令码必须在1-16之间");
            break;
        case 10001:
            strcat(sz,"寄存器首地址必须在0-9999之间");
            break;
        case 10002:
            strcat(sz,"寄存器长度必须在1-127之间");
            break;
        case 10003:
            strcat(sz,"解析数据个数错误");
            break;
        case 10004:
            strcat(sz,"解析数据电力类型错误");
            break;
        case 10005:
            strcat(sz,"解析数据起始偏移错误");
            break;
        case 10006:
            strcat(sz,"解析数据位宽错误");
            break;
        case 10007:
            strcat(sz,"解析数据字节顺序错误");
            break;
        case 10008:
            strcat(sz,"解析数据类型错误");
            break;
        default:
            break;

        }
        return sz ;
    }
};

class  AbstractConfObject
{
public:
    AbstractConfObject(){
        for(int i= 0 ;i< END ;i++)
            dataNums [i] = 0 ;
    }
public:
    virtual void check() = 0 ;
public:

      int dataNums[END] ;

};

class  Request: public AbstractConfObject
{
public:
    int reqType ;
    int reg;
    int num ;

public:
    void check(){
        if(reqType<1||reqType>16)
        {
            ConfObjectException e(ConfObjectException::req,10000);
            BOOST_THROW_EXCEPTION(e);
        }
        if(reg<0||reg>9999)
        {
            ConfObjectException e(ConfObjectException::req,10001);
            BOOST_THROW_EXCEPTION(e);
        }
        if(num<0||(reqType==1&&num>1016)||(reqType==2&&num>1016)||(reqType==3&&num>127)||(reqType==4&&num>127))
        {
            ConfObjectException e(ConfObjectException::req,10002);
            BOOST_THROW_EXCEPTION(e);
        }
        for(size_t n = 0 ;n < parses.size();n++)
        {
            if(parses[n].dataNums<0||(parses[n].dataNums*parses[n].dataSize)/16>num)
            {
                ConfObjectException e(ConfObjectException::req,10003);
                BOOST_THROW_EXCEPTION(e);
            }
            if(parses[n].powerType!=1&&parses[n].powerType!=2&&parses[n].powerType!=3)
            {
                ConfObjectException e(ConfObjectException::req,10004);
                BOOST_THROW_EXCEPTION(e);
            }
            if(parses[n].startIndex<0||parses[n].startIndex>=num)
            {
                ConfObjectException e(ConfObjectException::req,10005);
                BOOST_THROW_EXCEPTION(e);
            }
            if(parses[n].dataSize!=1&&parses[n].dataSize!=8&&parses[n].dataSize!=16&&parses[n].dataSize!=32)
            {
                ConfObjectException e(ConfObjectException::req,10006);
                BOOST_THROW_EXCEPTION(e);
            }
            if(parses[n].dataOrder!=21&&parses[n].dataOrder!=12&&parses[n].dataOrder!=2143&&parses[n].dataOrder!=1234
                    &&parses[n].dataOrder!=3412&&parses[n].dataOrder!=4321)
            {
                ConfObjectException e(ConfObjectException::req,10007);
                BOOST_THROW_EXCEPTION(e);
            }
            if(parses[n].dataType!=1&&parses[n].dataType!=2&&parses[n].dataType!=3)
            {
                ConfObjectException e(ConfObjectException::req,10008);
                BOOST_THROW_EXCEPTION(e);
            }

        }
    }

    vector<Parse> parses ;
};

class Module: public AbstractConfObject
{

public:
    int     addr ;

public:
    void check(){
        if(addr<1||addr>63)
        {
            ConfObjectException e(ConfObjectException::mod,1000);
            BOOST_THROW_EXCEPTION(e);
        }
        for(size_t n = 0 ;n<reqs.size();n++)
            reqs[n].check();
    }
    vector<Request> reqs ;
};

class Bus: public AbstractConfObject
{
public:

    unsigned char port ;
    char          sPort[32] ;
    int           baud ;
    int           databits ;
    int           stopbits ;
    char          parity ;



public:
    void check(){
        if(baud<300||baud>115200||databits<6||databits>8||stopbits<1||stopbits>3||(parity!='N'&&parity!='E'&&parity!='O'))
        {
            ConfObjectException e(ConfObjectException::bus,100);
            BOOST_THROW_EXCEPTION(e);
        }
        for(size_t n = 0 ;n<modules.size();n++)
            modules[n].check();
    }
    vector<Module> modules ;
};

class Config: public AbstractConfObject
{
public:
    Config();
    vector<Bus> busLines ;
    int         bus_number ;

public:
    void check();


public:
    bool load();
    bool save();
};

#endif // CONFIG_H
