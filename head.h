#ifndef HEADER_H
#define HEADER_H
#include <iostream>
#include <string>
#include <fstream>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>
using boost::asio::ip::udp;

class udp_server{
public:
  udp_server(boost::asio::io_service& io_service);
private:
  //отсюда начинают работу сокеты и дальше идут по вызовам handle_receive и handle_receive1
  void start_receive();

  void handle_receive(const boost::system::error_code &,
      std::size_t );
  
  //Отсюда снова переходим в start_receive
  void handle_receive1(const boost::system::error_code &,
      std::size_t );

  int size;
  char * s;
  udp::socket socket_;
  udp::endpoint remote_endpoint_;
  std::string recv_buffer_;
};

#endif
