#include "include.hpp"

class tiny_client{
    int id;
    std::string mq_name;
    std::string mq_sem_name;
    std::string mem_name;
    std::string mem_sem_name;

    void* mem_ptr;
    sem_t* mq_sem;
    sem_t* mem_sem;
    int fd;
    mqd_t mq;
    int num_seg;
    int seg_size;
    int mem_id;

    tiny_client();
    ~tiny_client();
}