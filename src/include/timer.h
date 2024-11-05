// timer.h
#ifndef TIMER_H
#define TIMER_H

#include <functional>
#include <sys/netmgr.h>
#include <sys/neutrino.h>
#include <atomic>

using std::atomic;
using std::function;
class Timer {
public:
    Timer(int interval, function<void()> callback);
    ~Timer();

    void start();
    void stop();
    int get_chid() const;

private:
    int interval;
    void init();
    static void* timer_thread(void* arg);
    void run();

    int chid;
    timer_t timer_id;
    struct sigevent event;
    struct itimerspec timer_spec;
    int thread_id;
    atomic<bool> running;
    std::function<void()> callback;
    static constexpr int PRIORITY = 10;
};

#endif