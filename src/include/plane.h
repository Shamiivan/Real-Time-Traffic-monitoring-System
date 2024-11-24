// plane.h
#ifndef PLANE_H
#define PLANE_H

#include <string>
#include <mutex>
#include <pthread.h>
#include <sys/neutrino.h>
#include "vector.h"

class Plane {
public:
    Plane();
    Plane(std::string _id, Vector position, Vector speed);
    ~Plane();

    void start();
    void stop();

    Vector get_pos() const;
    Vector get_speed() const;
    std::string get_id() const;

    void set_velocity(Vector speed);
    void set_pos(Vector position);

    Vector update_position();

    int getChannelId() const;
    int getChannelId1() const;

private:
    static void* threadFunc(void* arg);
    static void* msgThreadFunc(void* arg);
    static void* courseCorrectThreadFunc(void* arg);
    void messageLoop();
    void courseCorrectLoop();

private:

    std::string id;
    Vector position;
    Vector velocity;

    pthread_t thread_;       // Position update thread
    pthread_t msg_thread_;   // Message handling thread
    pthread_t course_currect_thread_; //course correction
    bool running_;
    mutable std::mutex mtx;

    // IPC variables
    int chid_; // Channel ID for receiving messages
    int chid1_;

    // Time step for position updates
    double dt;

    // Synchronization for IPC
    pthread_mutex_t mutex_;
};

#endif // PLANE_H
