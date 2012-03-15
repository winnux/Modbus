#ifndef NETSERVER_H
#define NETSERVER_H
#define BOOST_DATE_TIME_NO_LIB
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/circular_buffer.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <map>

using namespace std ;
using boost::asio::ip::tcp;
#define LISTEN_PORT     10010
#define MAX_PORT_NUM    16

struct RAW_COMM_DATA
{
    uint8_t data[255];
    uint8_t length ;
    uint8_t isRecv ;
};

map<int,boost::circular_buffer<RAW_COMM_DATA> > rawCommDatas ;

boost::mutex commData_mutex[MAX_PORT_NUM];
class session
{
public:
  session(boost::asio::io_service& io_service)
    : socket_(io_service)
  {
  }

  tcp::socket& socket()
  {
    return socket_;
  }

  void start()
  {
    socket_.async_read_some(boost::asio::buffer(data_, max_length),
        boost::bind(&session::handle_read, this,
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));
  }

private:
  //send:0x71、 端口号（1，2..)、0x00、0x00、0x00、0x00、0x00、0x00
  //eply: 0x61、端口号（1，2..)、长度高、长度低、0x00、0x00、0x00、0x00
  //err reply: echo
  void on_data_recv(size_t bytes_transferred)
  {
      int length = bytes_transferred;
      if(data_[0] == 0x71)
      {
          int com = data_[1] ;
          //查找对应串口通讯数据map找到对应的缓存circle buffer，然后发送
          //任何错误仅仅echo接收桢
          if(rawCommDatas.find(com)!=rawCommDatas.end())
          {
              {
                  data_[0] = 0x61 ;
                  length = 0 ;
                  boost::mutex::scoped_lock lock(commData_mutex[com]);

                  for(size_t i = 0 ;i < rawCommDatas[com].size()&&length<max_length ;i++)
                  {
                      memcpy(data_+8+length,rawCommDatas[com][i].data,rawCommDatas[com][i].length) ;
                      length += rawCommDatas[com][i].length ;
                      if(rawCommDatas[com][i].isRecv)
                      {//\n\n
                          data_[8+length]=0x5c;
                          data_[8+length+1]=0x6e;
                          data_[8+length+2]=0x5c;
                          data_[8+length+3]=0x6e;
                          //data_[8+length]  = ';' ;
                          //length++ ;
                          length+=4 ;
                      }else
                      {
                          //data_[8+length] =',';    //每个发送后加一个逗号
                          //length++;
                          data_[8+length]=0x5c;
                          data_[8+length+1]=0x74;
                          data_[8+length+2]=0x5c;
                          data_[8+length+3]=0x74;
                          length+=4;
                      }
                  }
                  rawCommDatas[com].clear() ;
              }

              data_[2] = length/256 ;
              data_[3] = length%256 ;
          }
      }
      boost::asio::async_write(socket_,
               boost::asio::buffer(data_, length+8),
               boost::bind(&session::handle_write, this,
                 boost::asio::placeholders::error));

  }

  void handle_read(const boost::system::error_code& error,
      size_t bytes_transferred)
  {
    if (!error)
    {
        on_data_recv(bytes_transferred);
//      boost::asio::async_write(socket_,
//          boost::asio::buffer(data_, bytes_transferred),
//          boost::bind(&session::handle_write, this,
//            boost::asio::placeholders::error));
    }
    else
    {
      delete this;
    }
  }

  void handle_write(const boost::system::error_code& error)
  {
    if (!error)
    {
      socket_.async_read_some(boost::asio::buffer(data_, max_length),
          boost::bind(&session::handle_read, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
    }
    else
    {
      delete this;
    }
  }

  tcp::socket socket_;
  enum { max_length = 4096 };
  char data_[max_length];
};

class server
{
public:
  server(boost::asio::io_service& io_service, short port)
    : io_service_(io_service),
      acceptor_(io_service, tcp::endpoint(tcp::v4(), port))
  {
    start_accept();
  }

private:
  void start_accept()
  {
    session* new_session = new session(io_service_);
    acceptor_.async_accept(new_session->socket(),
        boost::bind(&server::handle_accept, this, new_session,
          boost::asio::placeholders::error));
  }

  void handle_accept(session* new_session,
      const boost::system::error_code& error)
  {
    if (!error)
    {
      new_session->start();
    }
    else
    {
      delete new_session;
    }

    start_accept();
  }

  boost::asio::io_service& io_service_;
  tcp::acceptor acceptor_;
};
#endif // NETSERVER_H
