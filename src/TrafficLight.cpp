#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */


template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
    std::unique_lock<std::mutex> lock(_mtx);
    _cond.wait(lock, [this] {return !(this->_queue.empty());});

    T retorno = std::move(_queue.back());
    _queue.pop_back();
    return retorno;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    std::lock_guard<std::mutex> lock(_mtx);
    _queue.push_back(std::move(msg));
    _cond.notify_one();
}


/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    while(true){
        if (traffic_light_queue.receive() == TrafficLightPhase::green)
            return;
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. 
    // To do this, use the thread queue in the base class.
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases,this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 
    float stop_watch = 0;
    float stop_time = 4000 + 2000*(static_cast<float>(rand()))/(static_cast<float>(RAND_MAX));
    std::chrono::milliseconds cycle_time(0);
    std::chrono::time_point<std::chrono::high_resolution_clock> begin, end;
    while (true){
        begin = std::chrono::high_resolution_clock::now();
        //std::lock_guard<std::mutex> lock(_mtx);
        if (stop_watch < stop_time)
        {
            stop_watch += static_cast<float>(cycle_time.count());
        }
        else
        {
            stop_watch = 0;
            if (this->getCurrentPhase() == TrafficLightPhase::red)
            {
                this->_currentPhase = TrafficLightPhase::green;
            }
            else
            {
                this->_currentPhase = TrafficLightPhase::red;
            }
            this->traffic_light_queue.send(std::move(this->_currentPhase));
            
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        end = std::chrono::high_resolution_clock::now();
        cycle_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin); // Is this what is meant by loop cycle time?
    }
}
