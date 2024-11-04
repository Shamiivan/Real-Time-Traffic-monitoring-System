#include "plane.h"
#include <cstdio>
#include <cstdlib>
#include <sys/netmgr.h>
#include <sys/neutrino.h>
#include <iostream>
#include <pthread.h>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
Plane::Plane(){

}
Plane::Plane(std::string _id, Vector position, Vector speed)
    : id(_id), position(position), velocity(speed), running(false), thread_id(0) {
    // Set up timer
        chid = ChannelCreate(0);
        if (chid == -1) {
            std::cerr << "Failed to create channel\n";
            exit(EXIT_FAILURE);
        }

        event.sigev_notify = SIGEV_PULSE;
        event.sigev_coid = ConnectAttach(ND_LOCAL_NODE, 0, chid, _NTO_SIDE_CHANNEL, 0);
        if (event.sigev_coid == -1) {
            std::cerr << "Failed to attach channel\n";
            exit(EXIT_FAILURE);
        }

        event.sigev_priority = DEFAULT_PRIORITY;
        event.sigev_code = MY_PULSE_CODE;

        if (timer_create(CLOCK_MONOTONIC, &event, &timer_id) == -1) {
            std::cerr << "timer_create failed\n";
            exit(EXIT_FAILURE);
        }

        itime.it_value.tv_sec = dt;    // First expiry in 1 sec
        itime.it_value.tv_nsec = 0;
        itime.it_interval.tv_sec = dt; // Interval of 1 sec
        itime.it_interval.tv_nsec = 0;

        timer_settime(timer_id, 0, &itime, NULL);
    std::cout << "Plane " << id << " created: Channel ID: " << chid << std::endl;
}

Plane::~Plane() {
    stop();
    timer_delete(timer_id);
    ConnectDetach(event.sigev_coid);
    ChannelDestroy(chid);
}

void Plane::start() {
    running = true;
    thread_id = ThreadCreate(0, Plane::thread_func, this, NULL);
    if (thread_id == -1) {
        std::cerr << "Failed to create thread\n";
        exit(EXIT_FAILURE);
    }
    printf("Plane %s on thread %d\n", id.c_str(), thread_id);
}

void Plane::stop() {
    running = false;
    if (thread_id != 0) {
        ThreadJoin(thread_id, NULL);
        ThreadDestroy(thread_id, 0, NULL);
        thread_id = 0;
    }
}

void* Plane::thread_func(void* arg) {
    Plane* plane = static_cast<Plane*>(arg);
    plane->run();
    return NULL;
}

void Plane::run() {
    timer_msg msg;
    int rcvid;


    while (running) {
        rcvid = MsgReceive(chid, &msg, sizeof(msg), NULL);
        if (rcvid == 0 && msg.pulse.code == MY_PULSE_CODE) {
            update_position();
        }
    }
}

Vector Plane::get_pos() const {
    std::lock_guard<std::mutex> lock(mtx);
    return position;
}

Vector Plane::get_speed() const {
    std::lock_guard<std::mutex> lock(mtx);
    return velocity;
}

std::string Plane::get_id() const {
    return id;
}

void Plane::set_velocity(Vector speed) {
    std::lock_guard<std::mutex> lock(mtx);
    velocity = speed;
}

void Plane::set_pos(Vector position) {
    std::lock_guard<std::mutex> lock(mtx);
    this->position = position;
}

Vector& Plane::update_position() {
    std::lock_guard<std::mutex> lock(mtx);
    position.x += velocity.x * dt;
    position.y += velocity.y * dt;
    position.z += velocity.z * dt;
    return position;
}

