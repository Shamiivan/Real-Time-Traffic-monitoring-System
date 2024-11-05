#include "plane.h"
#include <cstdio>
#include <cstdlib>
#include <sys/netmgr.h>
#include <sys/neutrino.h>
#include <iostream>
#include <pthread.h>


Plane::Plane(){

}
Plane::Plane(std::string _id, Vector position, Vector speed)
    : id(_id), position(position), velocity(speed){}

Plane::~Plane() {
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

Vector Plane::update_position() {
    std::lock_guard<std::mutex> lock(mtx);
    position.x += velocity.x * dt;
    position.y += velocity.y * dt;
    position.z += velocity.z * dt;
    return position;
}
