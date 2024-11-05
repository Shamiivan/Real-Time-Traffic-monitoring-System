#ifndef PLANE_H
#define PLANE_H

#include <string>
#include <atomic>
#include <mutex>
#include <time.h>
#include <sys/netmgr.h>
#include <sys/neutrino.h>
#include <iostream>
#include "vector.h"

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

private:
    static void* thread_func(void* arg);
    void run();

    std::string id;
    Vector position;
    Vector velocity;

    mutable std::mutex mtx;
    static constexpr int dt = 1;
};

#endif // PLANE_H
