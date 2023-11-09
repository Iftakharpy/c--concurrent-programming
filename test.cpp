/*
    None of the proposed pipelines are balanced. The input is 1 per minute, 
    so to output the same 1 per minute on average you would need 10 security checks.
    Even then, we have to take into account that this only approaches close to being balanced.
    A pipeline can only be truly balanced when each part of the pipeline takes the same amount of time.
 */

#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <vector>
#include <condition_variable>

const int num_people = 4;
const std::chrono::seconds boarding_pass_scan_latency = std::chrono::seconds(1);
const std::chrono::seconds security_check_latency = std::chrono::seconds(10);
const int num_security_checks = 4;

std::mutex boarding_pass_scan_mutex;
std::mutex security_check_mutex;
std::condition_variable security_check_cv;
int num_checks_in_progress = 0;

void processPassenger(int passenger_num) {
    // Boarding pass scanning
    {
        std::lock_guard<std::mutex> lock(boarding_pass_scan_mutex);
        std::this_thread::sleep_for(boarding_pass_scan_latency);
        std::cout << "Passenger " << passenger_num << " has scanned their boarding pass." << std::endl;
    }

    // Security check
    {
        std::unique_lock<std::mutex> lock(security_check_mutex);

        // Wait if all security check points are in use
        security_check_cv.wait(lock, [&] { return num_checks_in_progress < num_security_checks; });

        // Start the security check
        num_checks_in_progress++;

        lock.unlock();

        std::this_thread::sleep_for(security_check_latency);

        lock.lock();
        num_checks_in_progress--;

        // Notify that a security check is completed
        security_check_cv.notify_one();
		std::cout << "Passenger " << passenger_num << " has completed the security check." << std::endl;
    }

}

int main() {
    std::thread passengers[num_people];

    auto start_time = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < num_people; ++i) {
        passengers[i] = std::thread(processPassenger, i);
    }

    for (int i = 0; i < num_people; ++i) {
        passengers[i].join();
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto total_time = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time).count();
    float average_per_person = total_time / float(num_people);

    std::cout << "Total time: " << total_time << " seconds" << std::endl;
    std::cout << "Average time per person: " << average_per_person << " seconds" << std::endl;
}
