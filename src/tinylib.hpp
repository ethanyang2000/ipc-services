#include "include.hpp"
typedef void (*callback_f)(char*);

class tiny_client{
public:
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
    int chunk_size;

    tiny_client();
    ~tiny_client();
    void Compress(std::string, bool, callback_f);
    void compress_chunk();
    void decompress_chunk();
    void do_compress(std::string);
    void async_call(std::string, callback_f);
};