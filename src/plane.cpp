#include "plane.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <sys/netmgr.h>
#include <sys/neutrino.h>


Plane::Plane() {
}
Plane::Plane(std::string _id, Vector position, Vector speed)
    : id(_id), position(position), velocity(speed) {

  // set up timer
  chid = ChannelCreate(0);

  if(SchedGet(0, 0, &sch_params) != -1) {
    priority = sch_params.sched_priority;
  } else {
    priority = DEFAULT_PRIORITY;
  }

  event.sigev_notify = SIGEV_PULSE;
  event.sigev_coid = ConnectAttach(ND_LOCAL_NODE, 0, chid, _NTO_SIDE_CHANNEL, 0);
  event.sigev_priority = this->priority;
  event.sigev_code = MY_PULSE_CODE;
  timer_create(CLOCK_MONOTONIC, &event, &timer_id);

  itime.it_value.tv_sec = 1; // first expiry in 1 sec
  itime.it_interval.tv_sec = 1; // interval of 1 sec
  timer_settime(timer_id, 0, &itime, NULL);

  running = true;
  pthread_create(&m_thread, nullptr, thread_callback, this);

}

Plane::~Plane() {
  if(running) {
    running = false;
    pthread_join(m_thread, NULL);
  }
  timer_delete(timer_id);
  ConnectDetach(event.sigev_coid);
  ChannelDestroy(chid);
}

void Plane::run() {
  while(running) {
    int rcvid = MsgReceive(chid, &msg, sizeof(msg), NULL);
    if(rcvid == 0) {
      Vector pos = update_position();
      printf("Plane %s: x=%f, y=%f, z=%f\n", id.c_str(), pos.x, pos.y, pos.z);
    }
  }
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


Vector Plane::update_position() {
  position.x += velocity.x * dt;
  position.y += velocity.y * dt;
  position.z += velocity.z * dt;
  return position;
}

