// tmanager.h
#ifndef TMANAGER_H
#define TMANAGER_H

#include <string>
#include <map>
#include <vector>
#include <functional>
#include <mutex>
#include <pthread.h>
#include "message.h"

class TopicManager {
public:
    static TopicManager& getInstance();

    int createTopic(const std::string& topic_name);
    int getTopicChannel(const std::string& topic_name);
    void destroyTopic(const std::string& topic_name);

    void registerSubscriber(const std::string& topic_name, int rcvid, const struct sigevent& event, std::function<void(const Message&)> callback);
    void unregisterSubscriber(const std::string& topic_name, int rcvid);

    void publish(const std::string& topic_name, const Message& message);

private:
    TopicManager();
    ~TopicManager();

    TopicManager(const TopicManager&) = delete;
    TopicManager& operator=(const TopicManager&) = delete;

    static void* notificationThreadEntry(void* arg);
    void notificationThread();

    std::map<std::string, int> topics_;
    std::map<std::string, std::vector<SubscriberInfo>> subscribers_;
    std::mutex mutex_;
    pthread_t notification_thread_;
    bool should_run_;
};

#endif // TMANAGER_H
