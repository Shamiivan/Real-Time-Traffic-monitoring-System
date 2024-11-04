#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/neutrino.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

// Configuration constants
#define MAX_MESSAGE_LENGTH 30
#define DEFAULT_MESSAGE_TYPE 0

// Clear message structures
typedef struct {
    uint16_t type;
    char content[MAX_MESSAGE_LENGTH];
} Message;

typedef struct {
    int checksum;
} Response;

// Global variables (could be wrapped in a context struct)
static int server_channel_id;
static const char* input_device = NULL;
static const char* program_name = "message_handler";

// Function prototypes
int initialize_server(void);
void* handle_client(void* arg);
int calculate_checksum(const char* text);
int read_input(int fd, char* buffer, size_t max_length);
void handle_error(const char* message);

// Server implementation
int main(int argc, char** argv) {
    // Setup buffering for stdout
    setvbuf(stdout, NULL, _IOLBF, 0);

    // Parse command line arguments
    if (argc == 2) {
        input_device = argv[1];
    } else if (argc > 2) {
        printf("Usage: %s [device_path]\nExample: %s /dev/ttyp2\n",
               program_name, program_name);
        exit(EXIT_FAILURE);
    }

    // Initialize server
    if (initialize_server() != 0) {
        handle_error("Failed to initialize server");
    }

    // Create client thread
    pthread_t client_thread;
    if (pthread_create(&client_thread, NULL, handle_client, NULL) != 0) {
        handle_error("Failed to create client thread");
    }

    // Main server loop
    while (1) {
        Message received_msg;
        Response response;
        int client_id;

        // Wait for incoming messages
        client_id = MsgReceive(server_channel_id, &received_msg,
                             sizeof(received_msg), NULL);
        if (client_id == -1) {
            printf("Error receiving message: %s\n", strerror(errno));
            continue;
        }

        // Process message
        if (received_msg.type == DEFAULT_MESSAGE_TYPE) {
            printf("%s: Server received: '%s'\n",
                   program_name, received_msg.content);

            // Calculate response
            response.checksum = calculate_checksum(received_msg.content);

            // Send response
            if (MsgReply(client_id, 0, &response, sizeof(response)) == -1) {
                printf("Error sending reply: %s\n", strerror(errno));
            }
        } else {
            printf("%s: Unknown message type: %d\n",
                   program_name, received_msg.type);
            MsgError(client_id, ENOSYS);
        }
    }

    return 0;
}

// Client implementation
void* handle_client(void* arg) {
    Message msg;
    Response response;
    int connection_id, input_fd;

    // Connect to server channel
    connection_id = ConnectAttach(0, 0, server_channel_id,
                                _NTO_SIDE_CHANNEL, 0);
    if (connection_id == -1) {
        handle_error("Failed to connect to server");
    }

    // Open input device or use stdin
    input_fd = input_device ?
               open(input_device, O_RDONLY) :
               STDIN_FILENO;

    if (input_fd == -1) {
        handle_error("Failed to open input device");
    }

    // Client message loop
    while (1) {
        printf("%s: Reading from %s\n", program_name,
               input_device ? input_device : "stdin");

        // Read input
        int bytes_read = read_input(input_fd, msg.content,
                                  MAX_MESSAGE_LENGTH - 1);
        if (bytes_read == -1) {
            break;
        }

        // Prepare and send message
        msg.type = DEFAULT_MESSAGE_TYPE;
        msg.content[bytes_read] = '\0';

        printf("%s: Client sending: '%s'\n", program_name, msg.content);

        if (MsgSend(connection_id, &msg, sizeof(msg),
                    &response, sizeof(response)) == -1) {
            printf("Error in MsgSend: %s\n", strerror(errno));
            continue;
        }

        printf("%s: Client received checksum: %d\n",
               program_name, response.checksum);
    }

    close(input_fd);
    return NULL;
}

// Utility functions
int initialize_server(void) {
    server_channel_id = ChannelCreate(0);
    return (server_channel_id == -1) ? -1 : 0;
}

int calculate_checksum(const char* text) {
    int sum = 0;
    while (*text != '\0') {
        sum += *text++;
    }
    return sum;
}

int read_input(int fd, char* buffer, size_t max_length) {
    int bytes_read = read(fd, buffer, max_length);
    if (bytes_read == -1) {
        printf("%s: Read error: %s\n", program_name, strerror(errno));
    }
    return bytes_read;
}

void handle_error(const char* message) {
    printf("%s: %s: %s\n", program_name, message, strerror(errno));
    exit(EXIT_FAILURE);
}