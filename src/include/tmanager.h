#ifndef TMANAGER_H
#define TMANAGER_H

#include <string>
#include <map>
#include <vector>
#include <memory>
#include <sys/neutrino.h>
#include <sys/dispatch.h>
#include <pthread.h>
#include <message.h>



class TopicManager {
 public :
   static TopicManager& getInstance();
    int create_topic(const std::string& topic_name);
    void delete_topic(std::string topic_name);
    int get_channel(const std::string& topic_name) const;
  private:
    TopicManager() = default;
    ~TopicManager();

    std::map<std::string, int> topics;
    std::mutex mtx;
};

#endif //TMANAGER_H
