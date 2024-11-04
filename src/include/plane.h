#ifndef PLANE_H
#define PLANE_H

#include <string>
#include <atomic>
#include <mutex>
#include <time.h>
#include <sys/netmgr.h>
#include <sys/neutrino.h>
#include <iostream>

#define MY_PULSE_CODE _PULSE_CODE_MINAVAIL

struct Vector {
    float x, y, z;
};

union timer_msg {
    struct _pulse pulse;
};

class Plane {
public:
    Plane();
    Plane(std::string _id, Vector position, Vector speed);
    ~Plane();

    Vector get_pos() const;
    Vector get_speed() const;
    std::string get_id() const;

    void set_velocity(Vector speed);
    void set_pos(Vector position);
    //void read_planes(std::vector<std::unique_ptr<Plane>> planes);

    Vector& update_position();

    void start();
    void stop();

private:
    static void* thread_func(void* arg);
    void run();

    std::string id;
    Vector position;
    Vector velocity;

    std::atomic<bool> running;
    mutable std::mutex mtx;
    static constexpr int dt = 1;

    int chid;
    int thread_id;
    timer_t timer_id;
    struct sigevent event; // event to deliver the timer
    struct itimerspec itime; // the timer specification
    static constexpr int DEFAULT_PRIORITY = 10;
};

#endif // PLANE_H
