#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <mutex>

// Both threads write to std::cout, which is a SHARED resource -- without
// coordination, two threads mid-write can interleave their characters and
// produce garbled output (this is exactly what happened when this program
// was run without the mutex: "Task ATask B finished\n finished").
//
// Note where the lock is (and isn't): it only wraps the actual print
// statements, NOT the sleep_for(). If we locked around the whole function,
// the two tasks would be forced to run one at a time again -- defeating
// the entire point. The lock protects the shared resource (cout), not the
// independent work each thread is doing.
std::mutex work_mutex;

// a dummy task
void task (const std::string& name, int work_seconds)
{
    {
        std::lock_guard<std::mutex> lock(work_mutex);
        std::cout << name << " started\n";
    }
    std::this_thread::sleep_for(std::chrono::seconds(work_seconds));
    {
        std::lock_guard<std::mutex> lock(work_mutex);
        std::cout << name << " finished\n";
    }
}

int main()
{
    auto program_start = std::chrono::steady_clock::now();

    std::thread t1(task, "Task A", 2);
    std::thread t2(task, "Task B", 2);

    t1.join();
    t2.join();

    auto program_end = std::chrono::steady_clock::now();

    auto total_ms = std::chrono::duration_cast<std::chrono::milliseconds>(program_end - program_start).count();

    std::cout << "Total time:" << total_ms << std::endl;
    
    return 0;
}