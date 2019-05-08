#include <iostream>
#include <fstream>
#include <string>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

std::ofstream in("msg.txt");
//Если один из сокетов прислал stop
//То значение заменяется на true и все соединения обрываются 
bool sockets_close = false;

//Наследуем , чтобы создать дополнительные экземпляры сокета
class tcp_connection : public boost::enable_shared_from_this<tcp_connection>{
public:
	typedef boost::shared_ptr<tcp_connection> pointer;

	static pointer create(boost::asio::io_context& io_context)
	{
		return pointer(new tcp_connection(io_context));
	}

	tcp::socket& socket()
	{
		return socket_;
	}

	//Отсюда начинаются все операции чтения/записи
	void start(int sock_number){
		socket_number = sock_number;
		//Считываем размер данных
		if (!sockets_close) {
			socket_.async_read_some(boost::asio::buffer(&size, sizeof(int)),
				boost::bind(&tcp_connection::handle_read1, shared_from_this(),
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred));
		}
	}

private:
	tcp_connection(boost::asio::io_context& io_context)
		: socket_(io_context)
	{ }

	//После того как узнали размер буфера, считываем сам буфер
	void handle_read1(const boost::system::error_code& err, size_t bytes_transferred) {
		if (!err and !sockets_close) {
			this->s = new char[size];

			socket_.async_read_some(boost::asio::buffer(this->s, size),
				boost::bind(&tcp_connection::handle_read2, shared_from_this(),
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred));
		}
		else {
			std::cerr << "error: " << err.message() << std::endl;
			socket_.close();
		}
	}
	//Прочитав буфер отправляем "ok"
	void handle_read2(const boost::system::error_code& err, size_t bytes_transferred){
		if (!err and !sockets_close) {
			msg = std::string(this->s);
			
			delete [] this->s;

			socket_.async_write_some(boost::asio::buffer("ok", 3),
				boost::bind(&tcp_connection::handle_write,shared_from_this(),
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred));

			in << socket_.remote_endpoint().address().to_string() + " " + msg << std::endl;
			std::size_t pos = msg.find("stop");
			if (pos == -1){
				start(socket_number);
			}
			else {
				sockets_close = true;
			}
		}
		else {
			std::cerr << "error: " << err.message() << std::endl;
			socket_.close();
		}
	}

	void handle_write(const boost::system::error_code& error, size_t bytes_transferred)
	{
		if (error){
			std::cerr << "error: " << error.message() << std::endl;
			socket_.close();
		}
	}
	tcp::socket socket_;
	std::string msg;
	int socket_number;
	int size;
	char *s;
};

class tcp_server{
public:
	tcp_server(boost::asio::io_context& io_context)
		: io_context_(io_context), count_sockets(0),
		acceptor_(io_context, tcp::endpoint(tcp::v4(), 1234))
	{
		start_accept();
	}

private:
	//Функция создает сокет и запускает асинхронную версию accept для ожидания нового соединения.
	void start_accept()
	{
		tcp_connection::pointer new_connection =
			tcp_connection::create(io_context_);

		if (!sockets_close) {
			acceptor_.async_accept(new_connection->socket(),
				boost::bind(&tcp_server::handle_accept, this, new_connection,
					boost::asio::placeholders::error));
		}
	}

	//Вызывается после async_accept и если есть возможность переходит к чтению/записи
	//Если нет возможности ожидает новое соединение
	void handle_accept(tcp_connection::pointer new_connection,
		const boost::system::error_code& error)
	{
		if (!sockets_close) {
			count_sockets++;
			if (!error)
			{
				new_connection->start(count_sockets);
			}

			start_accept();
		}
	}

	int count_sockets;
	boost::asio::io_context& io_context_;
	tcp::acceptor acceptor_;
};

int main(){
	boost::asio::io_context io_context;
	tcp_server server(io_context);
	io_context.run();
	in.close();
	return 0;
}
