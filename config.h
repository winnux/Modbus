#ifndef CONFIG_H
#define CONFIG_H
#include <vector>
using namespace std;
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

struct Request
{
public:
    int reqType ;
    int reg;
    int num ;

    int yxNum ;         //本桢内遥信数量
    int ycNum ;
    int ddNum ;
    vector<Parse> parses ;
};

struct Module
{
public:
    int     addr ;

    int     yxNum ;     //本装置内遥信数量
    int     ycNum ;
    int     ddNum ;
    vector<Request> reqs ;
};

struct Bus
{
public:

    unsigned char port ;
    char          sPort[32] ;
    int           baud ;
    int           databits ;
    int           stopbits ;
    char          parity ;

    int           yxNum ;       //本物理端口遥信数量
    int           ycNum ;
    int           ddNum ;
    vector<Module> modules ;
};

class Config
{
public:
    Config();
    vector<Bus> busLines ;
    int         bus_number ;

    int         yxNum ;         //全部遥信数量
    int         ycNum ;
    int         ddNum ;
public:
    bool load();
    bool save();
};

#endif // CONFIG_H
