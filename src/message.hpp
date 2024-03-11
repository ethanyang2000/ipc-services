#ifndef MESSAGE
#define MESSAGE


enum msg_t{
    INIT_CLIENT = 0,
    COMPRESS = 1,
    DECOMPRESS = 2,
    INIT_REPLY = 3
};

struct message{
    msg_t msg_type;
    int id;
    int num_seg;
    int seg_size;
};


#endif