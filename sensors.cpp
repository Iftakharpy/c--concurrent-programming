#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <future>


class Sensor{
    private:
        std::string name_;
        std::promise<int> sensorValuePromise;
        std::future<int> sensorValueFuture;
        std::thread sensorThread;
        bool isSensorValueReady = false;
        bool isRunning = true;

        void readSensor(){
            while(isRunning){
                if (!isSensorValueReady) {
                    // // Simulate sensor reading
                    // std::this_thread::sleep_for(std::chrono::milliseconds(100));

                    // Generate random sensor value
                    int sensorValue = rand() % 4096;
                    sensorValuePromise.set_value(sensorValue);
                    isSensorValueReady=true;
                };
            }
        }

    public:
        Sensor(std::string name){
            name_ = name;
            sensorValueFuture = sensorValuePromise.get_future();
            sensorThread = std::thread(&Sensor::readSensor, this);
        }
        Sensor(Sensor&& other){
            name_ = other.name_;
            isSensorValueReady = other.isSensorValueReady;
            sensorThread = std::thread(&Sensor::readSensor, this);
        }
        ~Sensor(){ // Destructor
            stop();
        }
        std::string getName(){
            return name_;
        }
        int getSensorData() {
            sensorValuePromise = std::promise<int>();
            sensorValueFuture = sensorValuePromise.get_future();
            isSensorValueReady = false;
            while (isRunning && (!isSensorValueReady || !sensorValueFuture.valid()));
            if (!isRunning) return -1;
            return sensorValueFuture.get();
        }
        void stop(){ // Clean up thread
            isRunning = false;
            if (sensorThread.joinable()) sensorThread.join();
        }
};

class SensorReader{
    private:
        std::vector<Sensor> sensors;
        std::thread readerThread;
        bool isRunning = true;

        void readSensors(){
            while(isRunning){
                for (auto &sensor : sensors){
                    int sensorValue = sensor.getSensorData();
                    if (sensorValue > 4000) std::cout << sensor.getName() << ": "<< sensorValue << std::endl;
                }
            }
        }
    public:
        SensorReader(int numberOfSensors) {
            // Don't create more sensors than there are threads available
            int maxThreads = std::thread::hardware_concurrency();
            // -2 to account for the main and readSensors
            if (maxThreads-2 < numberOfSensors) numberOfSensors = maxThreads-2;

            for (int i = 1; i <= numberOfSensors; i++){
                sensors.push_back(Sensor(std::string("Sensor " + std::to_string(i))));
            }
            readerThread = std::thread(&SensorReader::readSensors, this);
        }
        ~SensorReader(){ // Destructor
            stop();
        }
        void stop(){ // Clean up thread
            isRunning = false;
            if (readerThread.joinable()) readerThread.join();
        }
};



int main(){
    srand(time(NULL));
    int numberOfSensors = 4;
    SensorReader sensorReader(numberOfSensors);
    std::cin.get(); // Wait for user to press enter
    // Destructor of sensorReader and sensors will stop their own threads
    return 0;
}

