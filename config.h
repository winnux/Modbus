#ifndef CONFIG_H
#define CONFIG_H
#include <vector>
using namespace std;
struct Request
{
public:
    int reqType ;
    int reg;
    int num ;
};

struct Module
{
public:
    int     addr ;

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

    vector<Module> modules ;
};

class Config
{
public:
    Config();
    vector<Bus> busLines ;
    int         bus_number ;
public:
    bool load();
    bool save();
};

#endif // CONFIG_H
