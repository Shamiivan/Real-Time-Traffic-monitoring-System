#ifndef MESSAGE_H
#define MESSAGE_H

#include <string>
#include <stdint.h>

typedef struct {
    uint16_t id;
    std::string content;
}Message;

#define MESSAGE_TYPE_DATA 1
#define MESSAGE_TYPE_PULSE _PULSE_CODE_MINAVAIL // qnx pulse code
#endif //MESSAGE_H
