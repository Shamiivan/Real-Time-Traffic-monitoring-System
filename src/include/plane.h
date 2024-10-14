#ifndef PLANE_H
#define PLANE_H

#include <string>
#include <atomic>
#include <pthread.h>
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

    Vector update_position();

    void start();
    void stop();
    void run();

private:
    std::string id;
    Vector position;
    Vector velocity;

    std::atomic<bool> running;
    static constexpr float dt = 1.0f;
    pthread_t m_thread;
    timer_t timer_id;

    int chid;
    int coid;
    struct sigevent event;
    struct itimerspec itime;
    timer_msg msg;
    struct sched_param sch_params;
    int priority;
    static constexpr int DEFAULT_PRIORITY = 10;

    static void* thread_callback(void* arg) {
        Plane* plane = static_cast<Plane*>(arg);
        plane->run();
        return nullptr;
    }
};

#endif // PLANE_H