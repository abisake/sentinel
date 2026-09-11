#include <chrono>
#include <iostream>
#include <condition_variable>
#include <mutex>
#include <thread>

int data = 0;
bool ready = false;
std::mutex mtx_;
std::condition_variable cv;

void producer()
{
    std::this_thread::sleep_for(std::chrono::seconds(1));
    {
        std::lock_guard<std::mutex> lock(mtx_);
        data = 42;
        ready = true;
    }
    // Wake up the consumer, which has been genuinely asleep
    cv.notify_one();
    std::cout << "Producer: data is ready\n";
}

void consumer()
{
    std::unique_lock<std::mutex> lock(mtx_);
    /* Blocks here -- actually asleep, using zero CPU -- until notify_one()
        is called AND the predicate (ready == true) is actually true. The
        predicate check matters even with notify: it guards against
        "spurious wakeups" (the OS is allowed to wake a waiting thread with
        no notify at all) and against a notify that arrives before wait()
        was even called.
    */
   cv.wait(lock, [] {return ready;});
    std::cout << "Consumer: got data = " << data << "\n";
}

int main() 
{
    std::thread p(producer);
    std::thread c(consumer);
    p.join();
    c.join();
    return 0;
}