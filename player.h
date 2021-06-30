#ifndef _PLAYER_
#define _PLAYER_

#include "item.h"

#include <boost/property_tree/ptree.hpp>

#include <mutex>
#include <vector>
#include <unordered_map>

struct coordinate {
public:
	coordinate() = default;
	coordinate(const coordinate& rhs) = delete;

public:
	inline void vary(const int, const int);

private:
	int _x;
	int _y;
};

struct skill {
public:
	inline void increment(const size_t);

private:
	std::vector<size_t> _levels = std::vector<size_t>(5, 1);
};

struct package {

	std::list<equipment*> _equips;
	std::list<item*> _items;
};

struct fight_rel {
public:
	bool invoke_fight(fight_rel&);

private:
	std::mutex _mut;
	bool _status{false};
};

class player;

class player_action {
public:
	using JSON_OBJ = boost::property_tree::ptree;

public:
	static void initialize();

	static std::unordered_map<std::string, void(player_action::*)(JSON_OBJ&)> _actions;

public:
	player_action(player& player) : _player(player) {}

	void do_action(const char*);

	void chat(JSON_OBJ&);
	void move(JSON_OBJ&);
	void skill_up(JSON_OBJ&);
	void level_up(JSON_OBJ&);
	void start_fight(JSON_OBJ&);

private:
	player& _player;
};

class player {
public:
	friend class player_action;
	friend class player_connection;

	player() {}

	~player() {
		if (_PA) {
			delete _PA;
		}
	}

	inline void set_action(player_action* PA);

	void do_action(const char*);

private:
	player_action* _PA{nullptr};

	const unsigned long _id{0};
	const std::string _name{""};
	unsigned int _level{0};
	coordinate _coordinate;
	skill _skill;
	package _package;
	fight_rel _fight;
};

#endif // _PLAYER_

