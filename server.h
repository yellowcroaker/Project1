#ifndef _SERVER_
#define _SERVER_

#include "player.h"
#include "message.hpp"

#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>

#include <memory>
#include <unordered_map>

class player_connection : public std::enable_shared_from_this<player_connection> {

	using connection_ptr = std::shared_ptr<player_connection>;

public:
	static std::unordered_map<unsigned long, connection_ptr> _conns;

	player_connection(boost::asio::io_service& service,
		boost::asio::ip::tcp::socket socket)
		: _socket(std::move(socket))
		, _service(service)
		, _write_strand(_service)
	{}

	void start();

	player& get_player();

	boost::asio::ip::tcp::socket& get_socket();

private:
	void read_header();

	void read_body();

private:
	boost::asio::io_service& _service;
	boost::asio::ip::tcp::socket _socket;
	boost::asio::io_service::strand _write_strand;

	message _read_msg;
	player _player;
};

class player_action_server {

	using connection_ptr = std::shared_ptr<player_connection>;

public:
	player_action_server()
		: _acceptor(_io_service)
		, _socket(_io_service)
	{}

	void start_server(uint16_t);

private:
	void handle_new_connection();

private:
	boost::asio::io_service _io_service;
	boost::asio::ip::tcp::socket _socket;
	boost::asio::ip::tcp::acceptor _acceptor;
	std::vector<std::thread> _thread_pool;
};


#endif // !_SERVER_

