#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

using boost::asio::ip::udp;

struct MESG {
	int date;
	int num;
	int size_msg;
	std::string str;
};

//Преобразуем тип MESG в удобную строку для отправки
std::string create_msg(MESG msg) {
	return(std::to_string(msg.date) + std::to_string(msg.num) + std::to_string(msg.size_msg) + msg.str);
}

//Отправляем на сервер длинну сообщения и сообщение
bool write_msg(udp::socket &socket, std::string msg, udp::endpoint endpoint) {
	boost::system::error_code error;
	int leng = (size(msg));
	socket.send_to(boost::asio::buffer(&leng, sizeof(int)), endpoint);
	socket.send_to(boost::asio::buffer(msg.c_str(), leng), endpoint);
	if (error) {
		std::cout << "send failed: " << msg << std::endl;
		return false;
	}
	return true;
}

int main() {
	boost::asio::io_service service;
	int port = 1234;
	std::string ip = "192.168.56.102";

	udp::endpoint endpoint(udp::endpoint(boost::asio::ip::address::from_string("192.168.56.102"), 1234));

	udp::socket socket(service);
	socket.open(udp::v4());

	MESG first_msg;
	first_msg.date = 0;
	first_msg.num = 0;
	first_msg.str = std::string("put");
	first_msg.size_msg = size(first_msg.str);
	
	std::cout << socket.native_non_blocking() << std::endl;
	
	std::string send_str = create_msg(first_msg);
	if (!write_msg(socket, send_str, endpoint)) {
		system("pause");
		return 0;
	}

	for (int i = 0; i < 600000; i++) {
		MESG msg;
		msg.date = i;
		msg.num = i;
		msg.str = std::to_string(i);
		msg.size_msg = size(msg.str);
		std::cout << i << std::endl;
		
		send_str = create_msg(msg);
		if (!write_msg(socket, send_str, endpoint)) {
			system("pause");
			return 0;
		}
	}

	MESG last_msg;
	last_msg.date = 0;
	last_msg.num = 0;
	last_msg.str = std::string("stop");
	last_msg.size_msg = size(last_msg.str);

	send_str = create_msg(last_msg);

	if (!write_msg(socket, send_str, endpoint)) {
		system("pause");
		return 0;
	}

	socket.close();
	system("pause");
	return 0;
}
