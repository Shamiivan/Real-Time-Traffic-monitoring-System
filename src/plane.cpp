#include "plane.h"

Plane::Plane() {
}

Plane::Plane(std::string _id, Vector position, Vector velocity) {
  id = _id;
  this->position = position;
  this->velocity = velocity;
}

Plane::~Plane() {
}

Vector Plane::get_pos() const {
  return position;
}


Vector Plane::get_speed() const {
  return velocity;
}

std::string Plane::get_id() const {
  return id;
}

void Plane::set_velocity(Vector speed) {
  velocity = speed;
}

void Plane::set_pos(Vector position) {
  this->position = position;
}


Vector Plane::get_next_position() {
  return Vector{position.x + velocity.x, position.y + velocity.y, position.z + velocity.z};
}

