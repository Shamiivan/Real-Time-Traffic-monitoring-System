// plane.cpp
#include "plane.h"
#include "messages.h"
#include <cstdio>
#include <cstdlib>
#include <sys/netmgr.h>
#include <sys/neutrino.h>
#include <iostream>
#include <unistd.h>
#include <cstring>
#include "Logger.h"

Plane::Plane() : running_(false), dt(1.0) {
    pthread_mutex_init(&mutex_, nullptr);
    chid_ = ChannelCreate(0);
    if (chid_ == -1) {
        LOG_ERROR("Plane", "Failed to create channel");
        exit(EXIT_FAILURE);
    }
}

Plane::Plane(
    std::string _id,
    Vector position,
    Vector speed) : id(_id), position(position), velocity(speed), running_(false), dt(1.0){
    pthread_mutex_init(&mutex_, nullptr);
    chid_ = ChannelCreate(0);
    if (chid_ == -1) {
        LOG_ERROR("Plane", "Failed to create channel");
        exit(EXIT_FAILURE);
    }

    chid1_ = ChannelCreate(0);
    if (chid_ == -1) {
        LOG_ERROR("Plane", "Failed to create channel");
        exit(EXIT_FAILURE);
    }
}

Plane::~Plane() {
    stop();
    pthread_mutex_destroy(&mutex_);
}

void Plane::start() {
    running_ = true;
    int ret = pthread_create(&thread_, nullptr, Plane::threadFunc, this);
    if (ret != 0) {
        LOG_ERROR("Plane", "Failed to create position thread");
        exit(EXIT_FAILURE);
    }

    // Start the message handling thread
    ret = pthread_create(&msg_thread_, nullptr, Plane::msgThreadFunc, this);
    if (ret != 0) {
        LOG_ERROR("Plane", "Failed to create message thread");
        exit(EXIT_FAILURE);
    }

    //start the course correction thread. Used for collision avoidance.
    ret = pthread_create(&course_currect_thread_, nullptr, Plane::courseCorrectThreadFunc, this);
        if (ret != 0) {
            perror("Plane: Failed to create course correction thread");
            exit(EXIT_FAILURE);
        }
    LOG_INFO("Plane",
                 "Plane started with id: " + id
                + " thread id: " + std::to_string(thread_)
                + " message thread id: " + std::to_string(msg_thread_)
                + " course correction thread id: " + std::to_string(course_currect_thread_)
                + " channel id: " + std::to_string(chid_)
                + " channel id1: " + std::to_string(chid1_));

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

Vector Plane::update_position() {//test
  {
    std::lock_guard<std::mutex> lock(mtx);
    position.x += velocity.x * dt;
    position.y += velocity.y * dt;
    position.z += velocity.z * dt;
  }
    LOG_INFO("Plane", "Plane " + id + " updated position to (" + std::to_string(position.x) + ", "
             + std::to_string(position.y) + ", " + std::to_string(position.z) + ")");
    return position;
}


void Plane::stop() {
    if (running_) {
        running_ = false;
        ChannelDestroy(chid_);
        pthread_join(thread_, nullptr);
        pthread_join(msg_thread_, nullptr);
        pthread_join(course_currect_thread_, nullptr);
    }
    LOG_INFO("Plane", "Plane stopped with id: " + id);
}


void* Plane::threadFunc(void* arg) {
    Plane* self = static_cast<Plane*>(arg);
    while (self->running_) {
        self->update_position();
        sleep(1);
    }
    return nullptr;
}

void* Plane::msgThreadFunc(void* arg) {
    Plane* self = static_cast<Plane*>(arg);
    self->messageLoop();
    return nullptr;
}

void* Plane::courseCorrectThreadFunc(void* arg) {
    Plane* self = static_cast<Plane*>(arg);
    self->courseCorrectLoop();
    return nullptr;
}


void Plane::messageLoop() {
    int rcvid;
    RadarQueryMsg queryMsg;

    while (running_) {
        rcvid = MsgReceive(chid_, &queryMsg, sizeof(queryMsg), NULL);

        if (rcvid == -1) {
            if (errno == EINTR) {
                 LOG_ERROR("Plane", "MsgReceive failed" + std::to_string(errno));
                continue;
            } else {
                 LOG_ERROR("Plane", "MsgReceive failed" + std::to_string(errno));
                break;
            }
        } else if (rcvid > 0) {
            PlaneResponseMsg responseMsg;
            {
                std::lock_guard<std::mutex> lock(mtx);
                strncpy(responseMsg.data.id, id.c_str(), sizeof(responseMsg.data.id));
                responseMsg.data.id[sizeof(responseMsg.data.id) - 1] = '\0';
                responseMsg.data.x = position.x;
                responseMsg.data.y = position.y;
                responseMsg.data.z = position.z;
                responseMsg.data.speedX = velocity.x;
                responseMsg.data.speedY = velocity.y;
                responseMsg.data.speedZ = velocity.z;
            }
            MsgReply(rcvid, EOK, &responseMsg, sizeof(responseMsg));
        }
    }
}

int Plane::getChannelId() const {
    return chid_;
}

int Plane::getChannelId1() const {
    return chid1_;
}

void Plane::courseCorrectLoop() {
	int rcvid;
	courseCorrectionMsg msg;
	while(running_) {
		rcvid = MsgReceive(chid1_, &msg, sizeof(msg), NULL);
		if (rcvid == -1) {
			if (errno == EINTR) {
				continue;
		    } else {
                LOG_ERROR("Plane", "Failed to receive course correction id :" + id);
		        break;
		    }
		}
        LOG_INFO("Plane", "Plane " + id + " Received Course Correction alert."
                         + " New Velocity: (" + std::to_string(msg.newVelocity.x) + ", "
                         + std::to_string(msg.newVelocity.y) + ", "
                         + std::to_string(msg.newVelocity.z) + ")");

        set_velocity(msg.newVelocity);
	}
}
