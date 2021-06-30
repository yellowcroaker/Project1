#define PLAYER_ORDER "order"

#include "player.h"
#include "server.h"

std::unordered_map<std::string, void(player_action::*)(boost::property_tree::ptree&)> player_action::_actions;

inline void coordinate::vary(const int del_x, const int del_y) {
	_x += del_x;
	_y += del_y;
}

inline void skill::increment(const size_t index) {
	++_levels[index - 1];
}

bool fight_rel::invoke_fight(fight_rel& rhs) {
	std::lock(_mut, rhs._mut);
	std::lock_guard<std::mutex> lock_a(_mut, std::adopt_lock);
	std::lock_guard<std::mutex> lock_b(rhs._mut, std::adopt_lock);

	if (!_status && !rhs._status) {
		_status = rhs._status = true;
		return true;
	}
	return false;
}

void player_action::initialize() {
	_actions.reserve(16);

	_actions.emplace("chat", &player_action::chat);
	_actions.emplace("move", &player_action::move);
	_actions.emplace("skill_up", &player_action::skill_up);
	_actions.emplace("level_up", &player_action::level_up);
	_actions.emplace("start_fight", &player_action::start_fight);
}

void player_action::move(JSON_OBJ& msg) {
	int del_x = msg.get<int>("x");
	int del_y = msg.get<int>("y");
	_player._coordinate.vary(del_x, del_y);
}

void player_action::skill_up(JSON_OBJ& msg) {
	unsigned int skill_index = msg.get<unsigned int>("index");
	_player._skill.increment(skill_index);
}

void player_action::level_up(JSON_OBJ& msg) {
	++_player._level;
}

void player_action::start_fight(JSON_OBJ& msg) {
	unsigned long id = msg.get<unsigned long>("id");
	auto ret = player_connection::_conns.find(id);
	if (ret != player_connection::_conns.end()) {
		if (_player._fight.invoke_fight(ret->second->get_player()._fight)) {
			// to do 
			// {"id":1134, "msg":"1134向你发起了攻击"}
			std::string to_enemy;
			boost::asio::async_write(ret->second->get_socket(), boost::asio::buffer(to_enemy),
				[](const boost::system::error_code& ec, size_t bytes_xfer) {});
		}
	}
}

void player_action::chat(JSON_OBJ& msg) {
}

inline void player::set_action(player_action* PA) {
	_PA = PA;
}

void player_action::do_action(const char* msg_body) {
	JSON_OBJ msg(msg_body);
	std::string order = msg.get<std::string>(PLAYER_ORDER);

	auto action = _actions.find(order);
	if (action != _actions.end()) {
		(this->*action->second)(msg);
	}
}

void player::do_action(const char* msg_body) {
	_PA->do_action(msg_body);
}