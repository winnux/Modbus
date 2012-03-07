#ifndef NETSERVER_H
#define NETSERVER_H
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/circular_buffer.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <map>

using namespace std ;
using boost::asio::ip::tcp;
#define LISTEN_PORT     10010

struct RAW_COMM_DATA
{
    uint8_t data[255];
    uint8_t length ;
    uint8_t add ;
};

map<int,boost::circular_buffer<RAW_COMM_DATA> > rawCommDatas ;

boost::mutex commData_mutex;
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
  //eply: 0x81、端口号（1，2..)、长度高、长度低、0x00、0x00、0x00、0x00
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
                  data_[0] = 0x81 ;
                  length = 8 ;
                  boost::mutex::scoped_lock lock(commData_mutex);

                  for(size_t i = 0 ;i < rawCommDatas[com].size()&&length<10000 ;i++)
                  {
                      memcpy(data_+length,rawCommDatas[com][i].data,rawCommDatas[com][i].length) ;
                      length += rawCommDatas[com][i].length ;
                      if(rawCommDatas[com][i].add)
                      {
                          data_[length++]  = 0x20 ;//每个完整命令后增加一个空格
                      }
                  }
                  rawCommDatas[com].clear() ;
              }
              data_[2] = length%256 ;
              data_[3] = length/256 ;
          }
      }
      boost::asio::async_write(socket_,
               boost::asio::buffer(data_, length),
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
  enum { max_length = 10240 };
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
