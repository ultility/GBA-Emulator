#include "data_sizes.h"
#include <stdint.h>
struct request_data
{
    enum {input, output} request_type;
    enum {word, half_word, byte} data_type;
    uint32_t address;
    union
    {
        WORD word;
        HALF_WORD half_word;
        BYTE byte;
    } data;
};

struct request_channel
{
    const char* name;
    int id;
    int memory_address;
    int memory_range;
    void (*push_to_channel)(struct request_data*);
};