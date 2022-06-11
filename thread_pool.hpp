/*
*  Haining
*  20210612
*/
#ifndef _THREAD_POOL_
#define _THREAD_POOL

#include<queue>
#include<condition_variable>
#include<thread>
#include<vector>
#include<functional>
#include<mutex>
#include<atomic>

namespace posix {
class ThreadPool
{
public:
	ThreadPool()
	:running_(false)
	{
	};
	~ThreadPool() 
	{
		running_ = false;

		{
			std::lock_guard<decltype(mutex_)> _(mutex_);
			while (!queue_.empty()) { queue_.pop(); }
		}

		cv_.notify_all();
		for (auto& it : threads_) 
		{
			it.join();
		}
	};
public:
	void Run(const int& thread_num)
	{
		if (running_) { return; }
		running_ = true;

		for (int i = 0; i < thread_num; i++)
		{
			threads_.emplace_back(std::thread([&]() {
				while (running_)
				{
					std::unique_lock<decltype(mutex_)> _(mutex_);

					cv_.wait(_, [&]() {	return !queue_.empty() || !running_; });
					if (!running_) { break; }

					auto fun = queue_.front();
					queue_.pop();
					_.unlock();

					if (fun) { fun(); }
				}
				}));
		}

	}

	template<typename F>
	bool Post(const F& fun)
	{
		if (!running_) { return false; }

		{
			std::lock_guard<decltype(mutex_)> _(mutex_);
			queue_.push([&]() {
				fun();
				});
		}
		cv_.notify_one();
		return true;
	}
private:
	std::vector<std::thread> threads_;
	std::queue<std::function<void()>> queue_;
	std::mutex mutex_;
	std::condition_variable cv_;
	std::atomic<bool> running_;
};
}
#endif // !_THREAD_POOL_


// ≤‚ ‘¥˙¬Î
//#include<chrono>
//std::mutex mutex__;
//int main()
//{
//	{
//		std::cout << "test begin." << std::endl;
//		posix::ThreadPool thread_pool_;
//		thread_pool_.Run(5);
//
//		for (int i = 0; i < 10; i++)
//		{
//			std::cout << i << std::endl;
//			thread_pool_.Post([=]() {
//
//				{
//					std::lock_guard<decltype(mutex__)> _(mutex__);
//					std::cout << "time: " << std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count() << " thread id: " << std::this_thread::get_id() << " i: " << i << std::endl;
//				}
//
//				std::this_thread::sleep_for(std::chrono::seconds(2));
//				});
//		}
//		std::cout << "test end." << std::endl;
//
//		std::this_thread::sleep_for(std::chrono::seconds(20));
//	}
//}