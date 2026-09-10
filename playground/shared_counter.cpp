#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>

int shared_counter = 0;
std::mutex counter_mutex;

void increment(int times)
{
    for (int i = 0; i < times; ++i)
    {
        std::lock_guard<std::mutex> lock(counter_mutex);
        shared_counter++;
    }
}

int main()
{
    const int iter_per_thread = 1000000;

    std::thread t1(increment, iter_per_thread);
    std::thread t2(increment, iter_per_thread);

    t1.join();
    t2.join();

    std::cout << "Expected " << iter_per_thread *2 << "\n";
    std::cout << "Actual " << shared_counter << "\n";

    if (shared_counter == (iter_per_thread * 2)) {
        std::cout << "MATCH\n";
    } else {
        std::cout << "MISMATCH -- lost " << ((iter_per_thread * 2) - shared_counter)
                  << " updates\n";
    }
    return 0;
}