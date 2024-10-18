	// node.h

	#ifndef NODE_H
	#define NODE_H

	#include <string>
	#include <vector>
	#include <atomic>
	#include <mutex>
	#include <sys/neutrino.h>
	#include <pthread.h>
	#include "message.h"
	#include "tmanager.h"
	struct SubscriberData {
			std::string topic_name;
			int chid;
			pthread_t thread_id;
			std::atomic<bool> running;
		};

	class Node {
	public:
		Node(const std::string& name);
		~Node();

		template<typename T>
		void advertise(const std::string& topic_name, std::function<void(const T&)> publish_callback);

		template<typename T>
		void subscribe(const std::string& topic_name, std::function<void(const T&)> callback);

		template<typename T>
		void publish(const std::string& topic_name, const T& message);

	private:
		std::string name_;
		std::atomic<bool> running_;
		std::vector<std::pair<std::string, int>> subscriptions_;

		static void* subscriptionThreadFunc(void* arg);
		void subscriptionThread(const std::string& topic_name, int chid);
	};


	#endif // NODE_H
