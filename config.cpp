#include "config.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>
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
                m.reqs.push_back(r);
            }
            bus.modules.push_back(m);
        }

        busLines.push_back(bus);
    }

    return true ;
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
