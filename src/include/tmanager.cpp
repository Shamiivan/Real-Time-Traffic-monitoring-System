#include "tmanager.h"
#include <stdio.h>
#include <mutex>
#include <sys/neutrino.h>


TopicManager& TopicManager::getInstance() {
    static TopicManager instance;
    return instance;
}

int TopicManager::get_channel(const std::string& topic_name) const {
    std::lock_guard<std::mutex> lock(mtx);
    auto it = topics.find(topic_name);
    if (it != topics.end()) {
        return it->second;
    }
    return -1;
}

int TopicManager::create_topic(const std::string& topic_name) {
    std::lock_guard<std::mutex> lock(mtx);
    if (topics.find(topic_name) != topics.end()) {
         perror("Topic already exists on channel %d", topics[topic_name]);
        return topics[topic_name];
    }
    int chid = ChannelCreate(0);
    if (chid == -1) {
        perror("ChannelCreate failed");
        return -1;
    }
    topics[topic_name] = chid;
    return chid;
}


TopicManager::~TopicManager() {
    for (auto& topic : topics) {
        ChannelDestroy(topic.second);
    }
}