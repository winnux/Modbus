#ifndef CONFIG_H
#define CONFIG_H

#if defined(_WIN32)
#include <Windows.h> //for getmodulefilename and NULL
#endif
#include "ezlogger/ezlogger_headers.hpp"
#include <vector>
using namespace std;

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
    }

    vector<Parse> parses ;
};

class Module: public AbstractConfObject
{

public:
    int     addr ;

public:
    void check(){
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
    void check(){
        for(size_t n = 0 ; n< busLines.size();n++)
            busLines[n].check();
    }


public:
    bool load();
    bool save();
};

#endif // CONFIG_H
