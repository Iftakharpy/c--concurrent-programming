#include <thread>
#include <condition_variable>
#include <mutex>
#include <iostream>
#include <chrono>
#include <vector>
#include <atomic>
#include <iomanip>


class Worker{
private:
	unsigned int sleep_time = 10;
	float progress;
	std::vector<std::condition_variable*>& condition_variables_;
public:
	Worker(std::vector<std::condition_variable*>& condition_variables):
		condition_variables_(condition_variables){};

	// progress getter
	float currentProgress() const {
		return progress;
	};

	void doWork(){
		if (condition_variables_.size() == 0)
			return;
		std::condition_variable* first_cv = condition_variables_.at(0);
		first_cv->notify_one();
		
		if (condition_variables_.size() == 1)
			return;
		for (size_t i = 1; i < condition_variables_.size(); i++){
			std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time));
			progress += 100.0 / (condition_variables_.size()-1);
			condition_variables_.at(i)->notify_one();
		}
	};
};

int main() {
	std::mutex mtx;
	unsigned int condition_variables_length = 55+1;
	std::vector<std::condition_variable*> condition_variables;
	for (size_t i = 0; i < condition_variables_length; i++){
		condition_variables.push_back(new std::condition_variable());
	}
	
	Worker worker(std::ref(condition_variables));
	std::thread t(&Worker::doWork, &worker);
	std::cout << std::fixed << std::setprecision(2);
	for (auto& cv : condition_variables) {
		std::unique_lock<std::mutex> lk(mtx);
		cv->wait(lk);
		std::cout << "\rProgress: "<< std::setw(6) << worker.currentProgress() <<"%"<< std::endl;
	}

	t.join();
	return 0;
}

