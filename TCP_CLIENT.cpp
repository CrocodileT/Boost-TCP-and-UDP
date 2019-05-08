#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

using namespace boost::asio;
using ip::tcp;

struct MESG {
	int date;
	int num;
	int size_msg;
	std::string str;
};

bool try_connect(tcp::socket &socket, std::string ip_inf, int port) {
	boost::asio::io_service service;

	for (int i = 1; i <= 10; i++) {
		boost::system::error_code error;
		boost::asio::deadline_timer t(service, boost::posix_time::milliseconds(100));
		t.wait();
		ip::tcp::endpoint endpoint(tcp::endpoint(boost::asio::ip::address::from_string(ip_inf), 1234));
		socket.connect(endpoint, error);

		if (!error) return true;
	}
	std::cout << "FAIL CONNECT" << std::endl;
	return false;
}

std::string create_msg(MESG msg) {
	return(std::to_string(msg.date) + std::to_string(msg.num) + std::to_string(msg.size_msg) + msg.str);
}

bool write_msg(tcp::socket &socket, std::string msg) {
	boost::system::error_code error;
	int leng = msg.size()+1;
    
	boost::asio::write(socket, boost::asio::buffer(&leng, sizeof(int)), error);
	boost::asio::write(socket, boost::asio::buffer((msg + "\0").c_str(), leng), error);

    char OK[3];

    socket.receive(boost::asio::buffer(OK,3));
    std::cout << OK << std::endl;
	if (error) {
		std::cout << "send failed: " << msg << std::endl;
		return false;
	}
	return true;
}

int main() {
	boost::asio::io_service service;
	int port = 1234;
	std::string ip = "192.168.56.101";

	MESG first_msg;
	first_msg.date = 0;
	first_msg.num = 0;
	first_msg.str = "put";
	first_msg.size_msg = first_msg.str.size();


	tcp::socket socket(service);

	if (!try_connect(socket, ip, port)) {
		system("pause");
		return 0;
	}

	std::cout << socket.native_non_blocking() << std::endl;
	
	std::string send_str = create_msg(first_msg);
	if (!write_msg(socket, send_str)) {
		system("pause");
		return 0;
	}

	for (int i = 0; i < 100; i++) {
		MESG msg;
		msg.date = i;
		msg.num = i;
		msg.str = std::to_string(i);
		msg.size_msg = msg.str.size();

		send_str = create_msg(msg);
		if (!write_msg(socket, send_str)) {
			system("pause");
			return 0;
		}
	}

	MESG last_msg;
	last_msg.date = 0;
	last_msg.num = 0;
	last_msg.str = "stop";
	last_msg.size_msg = last_msg.str.size();

	send_str = create_msg(last_msg);

	if (!write_msg(socket, send_str)) {
		system("pause");
		return 0;
	}
    std::cout << "stop" << std::endl;
	socket.close();
	system("pause");
	return 0;
}
