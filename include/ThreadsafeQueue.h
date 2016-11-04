#ifndef _THREADSAFEQUEUE_H_
#define _THREADSAFEQUEUE_H_

////////////////////////////////////////////////////////////////////////////////
/// 
/// \class ThreadsafeQueue
/// Template for all queues used to send data from one thread/loop to the next.
///
////////////////////////////////////////////////////////////////////////////////

#include <cassert>
#include <iostream>

#ifndef __CINT__
#include <atomic>
#include <memory>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <queue>
#endif

class TDetector;

template<typename T>
class ThreadsafeQueue {
	public:
		ThreadsafeQueue(std::string name = "default", size_t maxSize = 50000);
		~ThreadsafeQueue();
#ifndef __CINT__
		int Push(T obj);
		long Pop(T& output, int millisecond_wait = 1000);

		size_t ItemsPushed() const;
		size_t ItemsPopped() const;
		size_t Size() const;

		std::string Name() { return fName; }

		//int ObjectSize(T&) const;

		bool IsFinished() const;
		void SetFinished(bool finished = true);

	private:
		std::string fName;
		mutable std::mutex mutex;
		std::queue<T> queue;
		std::condition_variable can_push;
		std::condition_variable can_pop;

		std::atomic_int num_writers;

		size_t max_queue_size;

		size_t items_in_queue;
		size_t items_pushed;
		size_t items_popped;

		std::atomic_bool is_finished;
#endif
};

#ifndef __CINT__
template<typename T>
ThreadsafeQueue<T>::ThreadsafeQueue(std::string name, size_t maxSize)
	: fName(name), max_queue_size(maxSize),
	items_in_queue(0), items_pushed(0), items_popped(0),
	is_finished(false) { }

template<typename T>
ThreadsafeQueue<T>::~ThreadsafeQueue() { }

template<typename T>
int ThreadsafeQueue<T>::Push(T obj) {
	std::unique_lock<std::mutex> lock(mutex);
	if(queue.size() > max_queue_size){
		can_push.wait(lock);
	}

	items_pushed++;
	items_in_queue++;

	queue.push(obj);
	can_pop.notify_one();
	return 1;
}

template<typename T>
long ThreadsafeQueue<T>::Pop(T& output, int millisecond_wait) {
	std::unique_lock<std::mutex> lock(mutex);
	if(!queue.size() && millisecond_wait){
		can_pop.wait_for(lock, std::chrono::milliseconds(millisecond_wait));
	}

	if(!queue.size()){
		return -1;
	}

	output = queue.front();
	queue.pop();

	items_popped++;
	items_in_queue--;

	can_push.notify_one();
	return queue.size();
	//return ObjectSize(output);
}

template<typename T>
size_t ThreadsafeQueue<T>::Size() const {
	std::unique_lock<std::mutex> lock(mutex);
	return items_in_queue;
}

template<typename T>
size_t ThreadsafeQueue<T>::ItemsPushed() const {
	std::unique_lock<std::mutex> lock(mutex);
	return items_pushed;
}

template<typename T>
size_t ThreadsafeQueue<T>::ItemsPopped() const {
	std::unique_lock<std::mutex> lock(mutex);
	return items_popped;
}

template<typename T>
bool ThreadsafeQueue<T>::IsFinished() const {
	return is_finished;
}

template<typename T>
void ThreadsafeQueue<T>::SetFinished(bool finished) {
	//std::cout<<std::endl<<fName<<": finished = "<<finished<<std::endl;
	is_finished = finished;
}
#endif /* __CINT__ */

#endif /* _THREADSAFEQUEUE_H_ */
