#include <iostream>
#include <chrono>
#include <future>
#include <random>

enum class TaskType { LIGHT, HEAVY};


class RandomNumberGenerator {
public:
  RandomNumberGenerator(TaskType taskType) : _taskType(taskType) {}

  void GenerateNumbers() {
    if (_taskType == TaskType::LIGHT) {
      GenerateLightNumbers();
    } else if (_taskType == TaskType::HEAVY) {
      GenerateHeavyNumbers();
    }
  }

private:
  TaskType _taskType;

  void GenerateLightNumbers() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(1, 100);

    for (int i = 0; i < 100; i++) {
      dist(gen);
    }
  }
  void GenerateHeavyNumbers() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(1, 10000000);

    for (int i = 0; i < 10000000; i++) {
      dist(gen);
    }
  }
};

int main() {
  std::vector<std::future<void>> futures;

  // =======================
  // Run tasks with mixed launching policies (deferred and async)
  for (int i = 0; i < 100; i++) {
    TaskType taskType = (std::rand() % 2 == 0) ? TaskType::LIGHT : TaskType::HEAVY;
    RandomNumberGenerator generator(taskType);

    if (taskType == TaskType::LIGHT) {
      futures.push_back(std::async(std::launch::deferred, &RandomNumberGenerator::GenerateNumbers, &generator));
    } else if (taskType == TaskType::HEAVY) {
      futures.push_back(std::async(std::launch::async, &RandomNumberGenerator::GenerateNumbers, &generator));
    }
  }

  auto start = std::chrono::high_resolution_clock::now();
  for (auto& future : futures) {
    future.wait();
  }
  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = end - start;
  std::cout << "Time elapsed: " << elapsed.count() << " seconds" << std::endl;
  // =======================



  // =======================
  // Run all tasks with async launching
  futures.clear();

  for (int i = 0; i < 100; i++) {
    TaskType taskType = (std::rand() % 2 == 0) ? TaskType::LIGHT : TaskType::HEAVY;
    RandomNumberGenerator generator(taskType);

    futures.push_back(std::async(std::launch::async, &RandomNumberGenerator::GenerateNumbers, &generator));
  }

  start = std::chrono::high_resolution_clock::now();
  for (auto& future : futures) {
    future.wait();
  }
  end = std::chrono::high_resolution_clock::now();
  elapsed = end - start;

  std::cout << "Time elapsed with all async launching: " << elapsed.count() << " seconds" << std::endl;
  // =======================

  // Output of the program on my machine compiled using c++17:
  // Time elapsed: 37.4423 seconds
  // Time elapsed with all async launching: 12.9984 seconds

  // Findings:
  // Running all tasks with async launching is faster than using a mix of deferred and async launching.
  // This is because deferred launching does not start the task until the future's get() method is called.
  // In this case, the get() methods are called serially, so the tasks are not actually executed in parallel.
  // With async launching, the tasks are started immediately, so they can be executed in parallel as quickly as possible.

  return 0;
}
