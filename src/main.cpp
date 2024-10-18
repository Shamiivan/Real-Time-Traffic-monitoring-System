#include <iostream>
#include <cstring>
#include <pthread.h>
#include <unistd.h>
#include <sys/neutrino.h>

#define MAX_TEXT_LEN 30
#define TEXT_MSG_TYPE 0

struct Message {
    uint16_t type;
    char text[MAX_TEXT_LEN];
};

struct Reply {
    int crc;
};

class CrcCalculator {
public:
    static int calculate(const char* text) {
        int crc = 0;
        while (*text) {
            crc += *text++;
        }
        return crc;
    }
};

class Server {
private:
    int chid;

public:
    Server() : chid(-1) {}

    bool initialize() {
        chid = ChannelCreate(0);
        if (chid == -1) {
            std::cerr << "Server: ChannelCreate failed: " << strerror(errno) << std::endl;
            return false;
        }
        return true;
    }

    void run() {
        Message msg;
        Reply reply;

        while (true) {
            int rcvid = MsgReceive(chid, &msg, sizeof(msg), nullptr);
            if (rcvid == -1) {
                std::cerr << "Server: MsgReceive failed: " << strerror(errno) << std::endl;
                continue;
            }

            if (msg.type == TEXT_MSG_TYPE) {
                std::cout << "Server: Received message: '" << msg.text << "'" << std::endl;
                reply.crc = CrcCalculator::calculate(msg.text);
                MsgReply(rcvid, EOK, &reply, sizeof(reply));
            } else {
                std::cout << "Server: Got unknown message type " << msg.type << std::endl;
                MsgError(rcvid, ENOSYS);
            }
        }
    }

    int getChannelId() const { return chid; }

    ~Server() {
        if (chid != -1) {
            ChannelDestroy(chid);
        }
    }
};

class Client {
private:
    int coid;

public:
    Client() : coid(-1) {}

    bool connect(int chid) {
        coid = ConnectAttach(0, 0, chid, _NTO_SIDE_CHANNEL, 0);
        if (coid == -1) {
            std::cerr << "Client: ConnectAttach failed: " << strerror(errno) << std::endl;
            return false;
        }
        return true;
    }

    void run() {
        Message msg;
        Reply reply;

        while (true) {
            std::cout << "Client: Enter a message (or 'q' to quit): ";
            std::cin.getline(msg.text, MAX_TEXT_LEN);

            if (strcmp(msg.text, "q") == 0) break;

            msg.type = TEXT_MSG_TYPE;

            int ret = MsgSend(coid, &msg, sizeof(msg), &reply, sizeof(reply));
            if (ret == -1) {
                std::cerr << "Client: MsgSend failed: " << strerror(errno) << std::endl;
            } else {
                std::cout << "Client: Got reply, CRC = " << reply.crc << std::endl;
            }
        }
    }

    ~Client() {
        if (coid != -1) {
            ConnectDetach(coid);
        }
    }
};

void* clientThread(void* arg) {
    int chid = *static_cast<int*>(arg);
    Client client;
    if (client.connect(chid)) {
        client.run();
    }
    return nullptr;
}

int main() {
    Server server;
    if (!server.initialize()) {
        return 1;
    }

    pthread_t clientThreadId;
    int chid = server.getChannelId();
    pthread_create(&clientThreadId, nullptr, clientThread, &chid);

    server.run();

    pthread_join(clientThreadId, nullptr);

    return 0;
}
