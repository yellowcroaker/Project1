#ifndef _THREAD_POOL
#define _THREAD_POOL

#include<list>
#include<vector>
#include<mutex>
#include<thread>
#include<atomic>

using _TASK = void(*)();

class task_queue {
public:
	friend class thread_pool;

	task_queue() = default;

	task_queue(const task_queue&) = delete;

private:
	std::list<_TASK> _tasks;

	std::mutex _mtx;
	std::condition_variable _cond;

public:
	void put(_TASK);

	_TASK take();
};

class thread_pool {
public:
	thread_pool() : _terminated(false) {
		initialize();
	}

	thread_pool(const thread_pool&) = delete;

	~thread_pool() {
		if (!_terminated)
			shut_down();
	}

	static const unsigned int thread_num;

private:
	task_queue _queue;

	std::atomic<bool> _terminated;

	std::vector<std::thread> _workers;

public:
	void execute(_TASK);

	void shut_down();

private:
	void initialize();
};

#endif // !_THREAD_POOL

