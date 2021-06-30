#include "server.h"
#include "player.h"

#include <thread>

std::unordered_map<unsigned long, std::shared_ptr<player_connection>> player_connection::_conns;

void player_connection::start() {
	_service.post(_write_strand.wrap([this, self = shared_from_this()]{
		_conns.emplace(_player._id, self);
		}));
	read_header();
}

player& player_connection::get_player() {
	return _player;
}

boost::asio::ip::tcp::socket& player_connection::get_socket() {
	return _socket;
}

void player_connection::read_header() {
	boost::asio::async_read(_socket, boost::asio::buffer(_read_msg.data(), message::header_length),
		[this, self = shared_from_this()](const boost::system::error_code& ec, size_t bytes_xfer) {
		if (!ec && _read_msg.decode_header())
			read_body();
		else
			_conns.erase(_player._id);
	});
}

void player_connection::read_body() {
	boost::asio::async_read(_socket, boost::asio::buffer(_read_msg.body(), _read_msg.body_length()),
		[this, self = shared_from_this()](const boost::system::error_code& ec, size_t bytes_xfer) {
		if (!ec) {
			_player.do_action(_read_msg.body());
			boost::asio::async_write(_socket, boost::asio::buffer("OK"),
				[](const boost::system::error_code& ec, size_t bytes_xfer) {});
			read_header();
		}
		else
			_conns.erase(_player._id);
	});
}

void player_action_server::start_server(uint16_t port) {
	boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), port);

	_acceptor.open(endpoint.protocol());
	_acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
	_acceptor.bind(endpoint);
	_acceptor.listen();

	player_action::initialize();

	handle_new_connection();

	size_t thread_count = std::thread::hardware_concurrency();
	_thread_pool.reserve(thread_count);
	for (int i = 0; i < thread_count; ++i)
		_thread_pool.emplace_back([=] { _io_service.run(); });

	for (std::thread& t : _thread_pool) {
		if (t.joinable())
			t.join();
	}
}

void player_action_server::handle_new_connection() {
	_acceptor.async_accept(_socket, [=](auto ec) {
		if (!ec) {
			// to do 初始化玩家信息
			auto connnection = std::make_shared<player_connection>(_io_service, std::move(_socket));
			connnection->start();
		}
		handle_new_connection();
		});
}