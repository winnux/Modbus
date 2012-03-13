#include "config.h"

Config::Config()
{
}
bool Config::load()
{
    using boost::property_tree::ptree;
    ptree pt ;
    read_xml("configuration.xml",pt);
    bus_number = pt.get<int>("Config.BusTotal");

    BOOST_FOREACH(ptree::value_type &v,pt.get_child("Config.Lines"))
    {    
        Bus bus ;

        bus.port = v.second.get<int>("Comm.Port");
#if defined(_WIN32)
        sprintf(bus.sPort,"COM%d",bus.port);
#else
        sprintf(bus.sPort,"/dev/ttyS%d",bus.port-1);
#endif
        bus.baud = v.second.get<int>("Comm.Baud");
        bus.databits = v.second.get<int>("Comm.DataBits");
        bus.stopbits = v.second.get<int>("Comm.StopBits");
        bus.parity = v.second.get<char>("Comm.Parity");
        BOOST_FOREACH(ptree::value_type &v1,v.second.get_child("Modules"))
        {

            Module m ;

            m.addr = v1.second.get<int>("Addr");
            BOOST_FOREACH(ptree::value_type &v2,v1.second.get_child("Reqs"))
            {

                Request r ;

                r.reqType = v2.second.get<int>("ReqType");
                r.reg = v2.second.get<int>("ReqRegister");
                r.num = v2.second.get<int>("ReqNum");
                BOOST_FOREACH(ptree::value_type &v3,v2.second.get_child("DataParses"))
                {
                    Parse p ;
                    p.dataOffset = 0 ;

                    p.dataNums = v3.second.get<int>("DataNums");
                    p.baseVar = v3.second.get<float>("BaseVar");
                    p.dataOrder = v3.second.get<int>("DataOrder");
                    p.dataSize = v3.second.get<int>("DataSize");
                    p.dataType = v3.second.get<int>("DataType");
                    p.deadBand = v3.second.get<float>("DeadBand");
                    p.mulVar = v3.second.get<float>("MulVar");
                    p.startIndex =v3.second.get<int>("StartIndex");
                    p.powerType = v3.second.get<int>("PowerType");
                    if(p.powerType>0&&p.powerType<END)
                        p.dataOffset += r.dataNums[p.powerType-1];
                    else
                        p.dataOffset = 0 ;
                    r.parses.push_back(p);
                    if(p.powerType>0&&p.powerType<END)
                        r.dataNums[p.powerType-1] += p.dataNums ;

                }
                for(int i = 0; i< END ;i++)
                    r.dataOffset[i] += m.dataNums[i] ;
                m.reqs.push_back(r);
                for(int i = 0 ; i< END ;i++)
                    m.dataNums[i] += r.dataNums[i];
            }
            for(int i = 0;i<END ;i++)
            {
                m.dataOffset[i] += bus.dataNums[i] ;
            }
            bus.modules.push_back(m);
            for(int i =0 ;i < END ;i++)
                bus.dataNums[i]  += m.dataNums[i] ;

        }
        for(int i= 0 ;i<END ;i++)
            dataOffset[i] += bus.dataNums[i] ;
        busLines.push_back(bus);
        for(int i = 0; i< END ;i++)
            dataNums[i] += bus.dataNums[i];
    }
    check();
    return true ;
}
void Config::check()
{
    if(bus_number>MAX_CACHE_COMMDATA_NUM)
    {
        ConfObjectException e(ConfObjectException::other,1) ;
        BOOST_THROW_EXCEPTION(e) ;
    }
    if(dataNums[0]>MAX_YX_NUM||dataNums[1]>MAX_YC_NUM||dataNums[2]>MAX_DD_NUM)
    {
        ConfObjectException e(ConfObjectException::other,2) ;
        BOOST_THROW_EXCEPTION(e) ;
    }
    for(size_t n = 0 ; n< busLines.size();n++)
        busLines[n].check();

}

bool Config::save()
{
    using boost::property_tree::ptree;
    ptree pt;

    pt.put("Config.BusTotal", bus_number);

    BOOST_FOREACH(const Bus& bus,busLines)
    {
        ptree child;
        child.put("Port",bus.port);
        child.put("Baud",bus.baud);
        child.put("DataBits",bus.databits);
        child.put("StopBits",bus.stopbits);
        child.put("parity",bus.parity);
        pt.add_child("Config.Lines.Bus",child);
    }
    write_xml("configuration.xml", pt);
    return true ;
}
