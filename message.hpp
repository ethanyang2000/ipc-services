#ifndef MESSAGE
#define MESSAGE


enum msg_t{
    INIT_CLIENT = 0,
    COMPRESS = 1,
    DECOMPRESS = 2
};

struct message{
    msg_t msg_type;
};


#endif