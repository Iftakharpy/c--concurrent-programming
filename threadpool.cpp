#include <iostream>
#include <thread>
#include <mutex>
#include <queue>
#include <vector>
#include <condition_variable>
#include <chrono>
#include <stdexcept>
#include <functional>
#include <memory>



// source: https://www.youtube.com/watch?v=ZKIhHLM9MfQ
class thread_pool{
	public:
		thread_pool(const thread_pool&) = delete;
		thread_pool(thread_pool&&) = delete;
		thread_pool& operator = (const thread_pool&) = delete;
		thread_pool& operator = (thread_pool&&) = delete;

		explicit thread_pool(std::size_t thread_count=(std::thread::hardware_concurrency() - 1)){
			if (!thread_count) throw std::invalid_argument("Thread count must be non-zero.");
			std::size_t max_threads = std::thread::hardware_concurrency();
			if (thread_count>0 && !(thread_count<max_threads)) std::out_of_range(std::string("Thread count must be between 1 and %s.", max_threads-1));

			std::cout << "Creating thread_pool with " << thread_count << " threads" << std::endl;
			std::mutex mtx_cout;
			for (std::size_t i=1; i<=thread_count; ++i){
				worker_threads_.push_back(std::thread([this, thread_id=i](){
					while (true){
						work_item_ptr_t work{nullptr};
						{
							// Thread safe way to take a work item off the queue
							std::unique_lock<std::mutex> guard(mtx_work_queue_);
							cv_work_queue_.wait(guard, [this](){ return !work_queue_.empty(); }); // wait until there is some work to do
							work = std::move(work_queue_.front());
							work_queue_.pop();
						};

						if(!work) break; // when the work is a null pointer stop the thread

						// Run the work item
						(*work)();
					}
				}));
			}
		};
		~thread_pool(){
			if (worker_threads_.size() ==0) return;
			{
				// stop the threads by pushing null pointer work items
				std::unique_lock<std::mutex> guard(mtx_work_queue_);
				for (std::size_t i=0; i<worker_threads_.size(); ++i) work_queue_.push(work_item_ptr_t(nullptr));
				cv_work_queue_.notify_all();
			}
			for (auto& t : worker_threads_) if (t.joinable()) t.join();
			worker_threads_ = std::vector<std::thread>{};
		};

		using work_item_t = std::function<void(void)>;
		void do_work(work_item_t work_item){
			auto wi = std::make_unique<work_item_t>(std::move(work_item));
			{
				std::unique_lock<std::mutex> guard(mtx_work_queue_);
				work_queue_.push(std::move(wi));
			}
			cv_work_queue_.notify_one();
		};

	private:
		using work_item_ptr_t = std::unique_ptr<work_item_t>;
		using work_queue_t = std::queue<work_item_ptr_t>;
		
		work_queue_t work_queue_;
		std::mutex mtx_work_queue_;
		std::condition_variable cv_work_queue_;

		using threads_t = std::vector<std::thread>;
		threads_t worker_threads_;
};

int main(){
	using namespace std;
	mutex cout_lock;

	auto start_time = chrono::high_resolution_clock::now();

	{
		thread_pool tp(2);
		for (size_t i=1; i<=20; i++){
			tp.do_work([&cout_lock, work_item_id=i](){
				{
					unique_lock<mutex> guard(cout_lock);
					cout << "work item " << work_item_id << " is starting up ..." << endl;
				}
				using namespace chrono_literals;
				this_thread::sleep_for(2ms);
				{
					unique_lock<mutex> guard(cout_lock);
					cout << "work item " << work_item_id << " is stopping ..." << endl;
				}
			});
		}
	}

	auto end_time = chrono::high_resolution_clock::now();
	auto duration = chrono::duration_cast<chrono::milliseconds>(end_time - start_time);

	std::cout << "Time taken: " << duration.count() << "ms" << std::endl;
	return 0;
}
