#include "head.h"
std::ofstream in("msg.txt");

udp_server::udp_server(boost::asio::io_service &io_service)
    : socket_(io_service, udp::endpoint(udp::v4(), 1234))
{
      udp_server::start_receive();
}

void udp_server::start_receive()
{
      socket_.async_receive_from(
          boost::asio::buffer(&size, sizeof(int)), remote_endpoint_,
          boost::bind(&udp_server::handle_receive, this,
                      boost::asio::placeholders::error,
                      boost::asio::placeholders::bytes_transferred));
}

void udp_server::handle_receive(const boost::system::error_code &error,
                                std::size_t)
{
      if (!error)
      {
            s = new char[size];
            socket_.async_receive_from(
                boost::asio::buffer(s, size), remote_endpoint_,
                boost::bind(&udp_server::handle_receive1, this,
                            boost::asio::placeholders::error,
                            boost::asio::placeholders::bytes_transferred));
      }

}

void udp_server::handle_receive1(const boost::system::error_code &error,
                                 std::size_t)
{
      if (!error)
      {

            recv_buffer_ = std::string(s);
            delete[] s;
            in << remote_endpoint_.address().to_string() + " " +  recv_buffer_<< std::endl;
            udp_server::start_receive();

            socket_.async_send_to(boost::asio::buffer("ok", 3), remote_endpoint_,
                boost::bind(&udp_server::handle_send, this,
                            boost::asio::placeholders::error,
                            boost::asio::placeholders::bytes_transferred));
      }
}

void udp_server::handle_send(const boost::system::error_code& /*error*/,
      std::size_t /*bytes_transferred*/)
  {
  }

int main()
{
      boost::asio::io_service io_service;
      udp_server server(io_service);
      io_service.run();
      in.close();
      return 0;
}
