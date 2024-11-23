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

Plane::Plane() : running_(false), dt(1.0) {
    pthread_mutex_init(&mutex_, nullptr);
    chid_ = ChannelCreate(0);
    if (chid_ == -1) {
        perror("Plane: Failed to create channel");
        exit(EXIT_FAILURE);
    }
}

Plane::Plane(
    std::string _id,
    Vector position,
    Vector speed) : id(_id), position(position), velocity(speed), running_(false), dt(1.0)
{
    pthread_mutex_init(&mutex_, nullptr);
    chid_ = ChannelCreate(0);
    if (chid_ == -1) {
        perror("Plane: Failed to create channel");
        exit(EXIT_FAILURE);
    }
}

Plane::~Plane() {
    stop();
    pthread_mutex_destroy(&mutex_);
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
    std::lock_guard<std::mutex> lock(mtx);
    position.x += velocity.x * dt;
    position.y += velocity.y * dt;
    position.z += velocity.z * dt;
//    std::cout << "Plane " << id << " updated position to (" << position.x << ", "
//              << position.y << ", " << position.z << ")"<< std::endl;
    return position;
}

void Plane::start() {
    running_ = true;
    int ret = pthread_create(&thread_, nullptr, Plane::threadFunc, this);
    if (ret != 0) {
        perror("Plane: Failed to create position thread");
        exit(EXIT_FAILURE);
    } else {
        std::cout << "Plane " << id << " position thread started.\n";
    }

    // Start the message handling thread
    ret = pthread_create(&msg_thread_, nullptr, Plane::msgThreadFunc, this);
    if (ret != 0) {
        perror("Plane: Failed to create message thread");
        exit(EXIT_FAILURE);
    } else {
        std::cout << "Plane " << id << " message thread started.\n";
    }

    //start the course correction thread. Used for collision avoidance.
    ret = pthread_create(&course_currect_thread_, nullptr, Plane::courseCorrectThreadFunc, this);
        if (ret != 0) {
            perror("Plane: Failed to create course correction thread");
            exit(EXIT_FAILURE);
        } else {
            std::cout << "Plane " << id << " course correction thread started.\n";
        }
}

void Plane::stop() {
    if (running_) {
        running_ = false;
        ChannelDestroy(chid_);
        pthread_join(thread_, nullptr);
        pthread_join(msg_thread_, nullptr);
        pthread_join(course_currect_thread_, nullptr);
    }
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
                continue;
            } else {
                perror("Plane: MsgReceive failed");
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

void Plane::courseCorrectLoop() {
	int rcvid;
	courseCorrectionMsg msg;
	while(running_) {
		rcvid = MsgReceive(chid_, &msg, sizeof(msg), NULL);
		if (rcvid == -1) {
			if (errno == EINTR) {
				continue;
		    } else {
		    	perror("Plane: Failed to receive course correction");
		        break;
		    }
		}
		else if (rcvid > 0) {
			std::cout << "Plane " << id << " Received Course Correction alert.\n";
			set_velocity(msg.newVelocity);
		}
	}
}
