/*
Airport
4 people are at the airport. To prepare for departure, 
each of them has to first scan their boarding pass (which takes 1 min),
and then to do the security check (which takes 10 minutes).

Assume that there is only one machine for scanning the boarding pass 
and only one security line. Explain why this pipeline is unbalanced. Compute its throughput.
Now assume that there are 2 security lines. Which is the new throughput?
If there were 4 security lines opened, would the pipeline be balanced?

Implement a test program to verify your findings.
Tip: You can simulate the throughput faster than 1/10 minutes. But after simulation, show the results based on original 1/10 minutes.
Tip: Terms.
Latency: Time needed to perform a given computation (e.g., process a customer)
Throughput: amount of work that can be done by a system in a given period of time (e.g., how many customers can be processed in one minute)
Balanced/Unbalanced Pipeline: a pipeline is balanced if it has constant latency



#####################################
Terms in more details:
============================
Latency:
	Latency refers to the time it takes for an individual at the airport to complete a single step in the process.
	It measures the delay between the start and completion of that person's specific action.
	In the context of the scenario, the latency for each individual person is the time it takes for that person to complete both their boarding pass scan and their security check.
	For example, the latency for one person in the scenario is 1 minute for their boarding pass scan and 10 minutes for their security check, totaling 11 minutes.

Throughput:
	Throughput measures how many individuals can complete the entire process (boarding pass scan + security check) in a given time frame.
	It represents the airport's capacity to handle multiple individuals simultaneously.
	Throughput is usually expressed in terms of how many people can be processed per unit of time (e.g., persons per minute).


Case 1: One Security Line
============================
Latency:
	Boarding pass scanning: 1 minute
	Security check: 10 minutes
	Total latency: 11 minutes
Throughput:
	Throughput is limited by the security check, which takes 10 minutes per person.
	Throughput = 1 person per 10 minutes = 1/10 = 0.1 person per minute
This pipeline is unbalanced because the latency is not constant.

To process 4 people it will take 41 minutes. Because the boarding pass machine can process one person every 1 minute,
when first person is getting security check done other 3 people can get their boarding pass scanned. but they still
have to wait in the queue to get the security check done for the person at the front.


Case 2: Two Security Lines & One Boarding Pass Machine
========================================================
Latency:
	Boarding pass scanning: 1 minute
	Security check: 10 minutes
	Total latency: 11 minutes
Throughput:
	Throughput is limited by the security check, which takes 10 minutes to process 2 people.
	Throughput = 2 person per 10 minutes = 2/10 = 0.2 person per minute
This pipeline is unbalanced because the latency is not constant.

To process all 4 people it will take 22 minutes. Because the boarding pass machine can process one person every 1 minute,
when first person is getting security check done, the 2nd person can start scanning his/her boarding pass it will take 1 minutes to get his/her
boarding pass scanned. After that the person can get the security check done at the 2nd second security check machine. The 3rd and 4th person can 
get their boarding pass scanned and get their security check done right after 1st and 2nd person gets their security check done.


Case 3: Four Security Lines & One Boarding Pass Machine
========================================================
Latency:
	Boarding pass scanning: 1 minute
	Security check: 10 minutes
	Total latency: 11 minutes
Throughput:
	Throughput is limited by the security check, which takes 10 minutes to process 4 people.
	Throughput = 4 person per 10 minutes = 4/10 = 0.4 person per minute
This pipeline is unbalanced because the latency is not constant.

To process all 4 people it will take 14 minutes. Because the boarding pass machine can process one person every 1 minute,
every can start doing the security check right after they are done scanning their boarding pass.



Conclusion
============================
The pipeline is unbalanced with one security line. The throughput is limited by the security check.
With two security lines, the throughput is increased to 2 people per 10 minutes. With four security lines,
the throughput is increased to 4 people per 10 minutes. With more than four security lines, the throughput is not increased any further.

The pipeline is unbalanced because the latency is not constant. The latency is not constant because the security check takes longer than the boarding pass scan.
The security check is the bottleneck of the pipeline. The throughput is limited by the security check.

The throughput can be increased by adding more security lines in theory when the load is constant and can be made balanced.
To balance the pipeline and make the latency constant, 1 boarding pass machine and 10 security lines are needed.
This is because the boarding pass scan takes 1 minute and the security check takes 10 minutes. The throughput is 10 people per 10 minutes.
The latency is constant after first 9 minutes under constant load.

In this case, the throughput is increased to 10/10 minutes(eg. 1 person per minute).
*/

#include <iostream>
#include <thread>
#include <vector>
#include <future>
#include <chrono>
#include <ctime>
#include <string>
#include <queue>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <stdexcept>
#include <functional>
#include <memory>


template <typename S>
std::ostream& operator<<(std::ostream& os,
                    const std::vector<S>& vector)
{
    // Printing all the elements
    // using <<
	os << "[";
    for (auto element : vector) {
        os << element << ", ";
    }
	os << "]";
    return os;
}


class Person
{
private:
	std::string name_;

public:
	// Regular constructor
	Person(std::string name) : name_(std::move(name)) {}

	// Copy constructor
	Person(const Person &other) : name_(other.name_)
	{
		// Any additional copy logic would go here
	}

	// Move constructor
	Person(Person &&other) noexcept : name_(std::move(other.name_))
	{
		// Any additional move logic would go here
	}

	Person& operator=(const Person& other){
		if (this != &other) { // protect against invalid self-assignment
			name_ = other.name_;
		}
		return *this;
	}

	friend std::ostream& operator<<( std::ostream& output, const Person& person ) { 
		output << "Person(" << "\"" << person.name_ << "\"" << ")" << std::endl;
		return output;
	}

	// Getter
	std::string getName() const {
		return name_;
	}
	// void setName(std::string name){
	// 	name_ = name;
	// }
};

bool DEBUG = true;
auto PROGRAM_START_TIME = std::chrono::high_resolution_clock::now();
std::mutex cout_lock;
long scaleFactor = 10;
std::vector<Person> passengers;
std::queue<Person> atBoardingPassCheck;
std::queue<Person> atSecurityCheck;
std::queue<Person> enteredAirport;
std::mutex mtx_atBoardingPassCheck;
std::mutex mtx_atSecurityCheck;
std::mutex mtx_enteredAirport;
std::condition_variable cv_atBoardingPassCheck;
std::condition_variable cv_atSecurityCheck;
std::condition_variable cv_enteredAirport;
bool isBoardingPassCheckDone = false;
bool isSecurityCheckDone = false;
bool isAllChecksDone = false;

// source: https://www.youtube.com/watch?v=ZKIhHLM9MfQ
class thread_pool
{
public:
	thread_pool(const thread_pool &) = delete;
	thread_pool(thread_pool &&) = delete;
	thread_pool &operator=(const thread_pool &) = delete;
	thread_pool &operator=(thread_pool &&) = delete;

	explicit thread_pool(std::size_t thread_count = (std::thread::hardware_concurrency() - 1))
	{
		if (!thread_count)
			throw std::invalid_argument("Thread count must be non-zero.");
		std::size_t max_threads = std::thread::hardware_concurrency() - 1; // -1 because we need one thread for the main thread
		if (!(thread_count > 0) || !(thread_count <= max_threads))
			std::out_of_range(std::string("Thread count must be between 1 and %s.", max_threads));

		for (std::size_t i = 1; i <= thread_count; ++i)
		{
			worker_threads_.push_back(std::thread([this, thread_id = i]()
												  {
					while (true){
						work_item_ptr_t work{nullptr};
						{
							// Thread safe way to take a work item off the queue
							std::unique_lock<std::mutex> guard(mtx_work_queue_);
							cv_work_queue_.wait(guard, [this](){ return !work_queue_.empty(); }); // wait until there is some work to do

							work = std::move(work_queue_.front());
							work_queue_.pop();
						}

						if(!work) break; // when the work is a null pointer stop the thread

						// Run the work item
						(*work)();
					} }));
		}
	};
	~thread_pool()
	{
		if (worker_threads_.size() == 0)
			return;
		{
			// stop the threads by pushing null pointer work items
			std::unique_lock<std::mutex> guard(mtx_work_queue_);
			for (std::size_t i = 0; i < worker_threads_.size(); ++i)
				work_queue_.push(work_item_ptr_t(nullptr));
			cv_work_queue_.notify_all();
		}

		for (auto &t : worker_threads_)
			if (t.joinable())
				t.join();
		worker_threads_ = std::vector<std::thread>{};
	};

	using work_item_t = std::function<void(void)>;
	void do_work(work_item_t work_item)
	{
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
	work_queue_t finished_work_queue_;
	std::mutex mtx_work_queue_;
	std::condition_variable cv_work_queue_;

	using threads_t = std::vector<std::thread>;
	threads_t worker_threads_;
};

class SecurityCheckStage
{
private:
	std::string name_;
	std::chrono::milliseconds timeToProcess1Person_; // time in miliseconds
	thread_pool security_check_pool_;

public:
	SecurityCheckStage(
		std::string name,
		std::chrono::milliseconds timeToProcess1Person,
		int numberOfMachines
		) : 
			name_(name),
			timeToProcess1Person_(timeToProcess1Person),
			security_check_pool_(numberOfMachines) {}

	void process_person(Person person, std::function<void(Person person)> callback) {
		security_check_pool_.do_work([this, person = person, callback = callback](){
			if (DEBUG) {
				auto end_time = std::chrono::high_resolution_clock::now();
				auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - PROGRAM_START_TIME);
				long long scaledTime = duration.count() / scaleFactor;
				std::lock_guard<std::mutex> guard(cout_lock);
				std::cout << "At " << scaledTime << "mins \t" << name_ << " is processing " << person.getName() << std::endl;
			}
			
			// Simulate doing work
			std::this_thread::sleep_for(timeToProcess1Person_ * scaleFactor);
			if (DEBUG) {
				auto end_time = std::chrono::high_resolution_clock::now();
				auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - PROGRAM_START_TIME);
				long long scaledTime = duration.count() / scaleFactor;
				std::lock_guard<std::mutex> guard(cout_lock);
				std::cout << "At " << scaledTime << "mins \t" << name_ << " is done processing " << person.getName() << std::endl;
			}

			// After processing done pass it to next stage
			callback(person);
		});
	};
};

class SecurityCheckStages
{
private:
	SecurityCheckStage boardingPassStage;
	SecurityCheckStage securityCheckStage;
	std::thread stage_manager;

public:
	SecurityCheckStages(
		std::size_t numberOfBoardingPassMachines = 1,
		std::size_t numberOfSecurityCheckMachines = 1
		) :
			boardingPassStage("Boarding Pass Check", std::chrono::milliseconds(1), numberOfBoardingPassMachines),
			securityCheckStage("Security Check", std::chrono::milliseconds(10), numberOfSecurityCheckMachines)
	{
		// initializer
		stage_manager = std::thread([this](){
			manage_security_check_stage_pipeline();
		});
	}
	~SecurityCheckStages() {
		if (stage_manager.joinable()) stage_manager.join();
	};

	void manage_security_check_stage_pipeline(){
		// Enqueue all passengers to boarding pass check
		for (auto person : passengers) {
			std::lock_guard<std::mutex> guard(mtx_atBoardingPassCheck);
			atBoardingPassCheck.push(person);
			cv_atBoardingPassCheck.notify_one();

			boardingPassStage.process_person(person, [](Person person){
				// After processing done remove it from boarding pass check queue
				{
					std::lock_guard<std::mutex> guard(mtx_atBoardingPassCheck);
					atBoardingPassCheck.pop();
					if (atBoardingPassCheck.empty()) isBoardingPassCheckDone = true;
				}
				cv_atBoardingPassCheck.notify_one();

				// Pass it to security check
				{
					std::lock_guard<std::mutex> guard(mtx_atSecurityCheck);
					atSecurityCheck.push(person);
				}
				cv_atSecurityCheck.notify_one();
			});
		};

		// Whenever a person is put on security check queue, process it
		while (!isSecurityCheckDone && !isBoardingPassCheckDone){
			Person person{"Unknown"};
			{
				std::unique_lock<std::mutex> guard(mtx_atSecurityCheck);
				// wait until there is some work to do or all passengers are processed
				cv_atSecurityCheck.wait(guard, [](){ return (!atSecurityCheck.empty() || (isSecurityCheckDone && isBoardingPassCheckDone)); });
				if (isSecurityCheckDone && isBoardingPassCheckDone) break;
				if (!atSecurityCheck.empty()){
					person = atSecurityCheck.front();
					atSecurityCheck.pop();
				}
			}


			securityCheckStage.process_person(person, [](Person person){
				// After processing done remove it from security check queue
				{
					if (atSecurityCheck.empty()) isSecurityCheckDone = true;
					else {
						std::lock_guard<std::mutex> guard(mtx_atSecurityCheck);
						atSecurityCheck.pop();
					}
				}
				cv_atSecurityCheck.notify_one();
			});
			

			// If all passengers are processed then stop the thread
			// if (isBoardingPassCheckDone && isSecurityCheckDone) break;
		}
	}
};

void timeWithNumberOfMachines(std::size_t numberOfPassengers, std::size_t numberOfBoardingPassMachines, std::size_t numberOfSecurityMachines)
{
	using namespace std;
	// Populate airport with few people
	for (size_t i = 1; i <= numberOfPassengers; i++)
	{
		Person person = Person(std::string("Person ") + std::to_string(i));
		passengers.push_back(person);
	}
	PROGRAM_START_TIME = chrono::high_resolution_clock::now();
	auto start_time = chrono::high_resolution_clock::now();
	{
		SecurityCheckStages securityChecks{numberOfBoardingPassMachines, numberOfSecurityMachines};
	}
	auto end_time = chrono::high_resolution_clock::now();
	auto duration = chrono::duration_cast<chrono::milliseconds>(end_time - start_time);
	long long scaledTime = duration.count() / scaleFactor;
	cout << "==============================================================================================================" << endl;

	cout << "Passengers: "<<numberOfPassengers << " \tBoardingPassMachines: "<<numberOfBoardingPassMachines << " \tSecurityCheckMachines: "<< numberOfSecurityMachines << " \tElapsed: " << scaledTime << "mins." << endl;

	{
		// Reset values
		std::lock_guard<std::mutex> guard(mtx_atBoardingPassCheck);
		std::lock_guard<std::mutex> guard2(mtx_atSecurityCheck);
		std::lock_guard<std::mutex> guard3(mtx_enteredAirport);
		passengers.clear();
		atBoardingPassCheck = std::queue<Person>();
		atSecurityCheck = std::queue<Person>();
		enteredAirport = std::queue<Person>();
		isBoardingPassCheckDone = false;
		isSecurityCheckDone = false;
		isAllChecksDone = false;
	}

	{
		std::lock_guard<std::mutex> guard(cout_lock);
		cout << "==============================================================================================================" << endl << endl;
	}
}

int main()
{
	timeWithNumberOfMachines(4, 1, 1);
	timeWithNumberOfMachines(4, 1, 2);
	timeWithNumberOfMachines(4, 1, 3);
	timeWithNumberOfMachines(4, 1, 4);
	timeWithNumberOfMachines(10, 1, 10);
	timeWithNumberOfMachines(20, 1, 10);
	timeWithNumberOfMachines(100, 1, 10);
	return 0;
}
