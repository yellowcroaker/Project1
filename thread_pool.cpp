#include "thread_pool.h"
#include <iostream>

const unsigned int thread_pool::thread_num(std::thread::hardware_concurrency());


void task_queue::put(_TASK _task) {
	std::lock_guard<std::mutex> lck(_mtx);

	_tasks.push_back(_task);

	_cond.notify_one();
}

_TASK task_queue::take() {
	std::unique_lock<std::mutex> lck(_mtx);
	_cond.wait(lck, [&] { return !_tasks.empty(); });

	_TASK _task = _tasks.front();
	_tasks.pop_front();

	return _task;
}

void thread_pool::execute(_TASK _task) {
	_queue.put(_task);
}

void thread_pool::initialize() {
	for (unsigned int i = 0; i < thread_num; ++i) {
		_workers.push_back(std::thread([&] {
			while (!_terminated.load()) {
				try {
					_TASK _task = _queue.take();
					_task();
				}
				catch (...) {
					// do something
				}
			}
			}));
	}
}

void thread_pool::shut_down() {
	bool curr_status = _terminated.load();
	if (_terminated.compare_exchange_strong(curr_status, true)) {

		for (unsigned int i = 0; i < thread_num; ++i)
			_queue.put([] { });

		for (auto iter = _workers.begin(); iter != _workers.end(); ++iter) {
			if (iter->joinable())
				iter->join();
		}
	}
}