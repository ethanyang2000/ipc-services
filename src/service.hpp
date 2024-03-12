#ifndef SERVICE
#define SERVICE

#include "include.hpp"
extern "C" {
    #include "snappy.h"
}

class client_info{
public:
    client_info(int client_id, struct mq_attr attr){
        id = client_id;
        sem_name = "/"+std::to_string(id)+"_sem";
        mq_name = "/"+std::to_string(id)+"_mq";

        client_mq = mq_open(mq_name.data(), O_CREAT | O_EXCL | O_RDWR, 0666, &attr);
        if(client_mq < 0){
            std::cout<<"open client "<< id <<" msg queue fail!"<<std::endl;
            throw std::runtime_error("open client msg queue fail!");
        }
        sem = sem_open(sem_name.data(), O_CREAT | O_EXCL | O_RDWR, 0666, 1);
    }

    ~client_info(){
        if (sem_close(sem) == -1) {
            std::cout<<"sem close at "<<id<<"failed"<<std::endl;
            throw std::runtime_error("sem close fail!");
        }
        if(sem_unlink(sem_name.data()) == -1){
            std::cout<<"sem delete at "<<id<<"failed"<<std::endl;
            throw std::runtime_error("sem delete fail!");
        }
        if (mq_close(client_mq) == -1) {
            std::cout<<"mq close at "<<id<<"failed"<<std::endl;
            throw std::runtime_error("mq close fail!");
        }
        if(mq_unlink(mq_name.data()) == -1){
            std::cout<<"mq delete at "<<id<<"failed"<<std::endl;
            throw std::runtime_error("mq delete fail!");
        }
    }

    // unique name
    int id;
    // msg queue
    mqd_t client_mq;
    // semaphore
    sem_t* sem;
    std::string sem_name;
    std::string mq_name;
};

class segment{
public:
    int id;
    int fd;
    int seg_size;
    void* mem_ptr;
    sem_t* sem;

    std::string mem_name;
    std::string sem_name;

    segment(int seg_id, int segment_size){
        id = seg_id;
        seg_size = segment_size;
        sem_name = "/"+std::to_string(id)+"_mem_sem";
        mem_name = "/"+std::to_string(id)+"_mem";

        fd = shm_open(mem_name.data(), O_CREAT | O_RDWR, 0666);
        if(fd < 0){
            std::cout<<"shared mem "<<id<<" init failed"<<std::endl;
            fprintf(stderr, "Error: %s\n", strerror(errno));
            throw std::runtime_error("open shared mem fail!");
        }
        ftruncate(fd,seg_size);
        sem = sem_open(sem_name.data(), O_CREAT | O_EXCL | O_RDWR, 0666, 1);

        mem_ptr = mmap(0,seg_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (mem_ptr == MAP_FAILED) {
            std::cout<<"mmap "<<id<<" failed"<<std::endl;
        }
    }

    ~segment(){
        if (munmap(mem_ptr, seg_size) == -1) {
            std::cout<<"unmap failed"<<std::endl;
        }
        mem_ptr = nullptr;
        close(fd);
        if(shm_unlink(mem_name.data()) == -1){
            std::cout<<"shared mem delete at "<<id<<"failed"<<std::endl;
            throw std::runtime_error("delete shared mem fail!");
        }
        if (sem_close(sem) == -1) {
            std::cout<<"sem close at "<<id<<"failed"<<std::endl;
            fprintf(stderr, "Error: %s\n", strerror(errno));
        }
        if(sem_unlink(sem_name.data()) == -1){
            std::cout<<"sem delete at "<<id<<"failed"<<std::endl;
            throw std::runtime_error("sem delete fail!");
        }
    }
};

class Server{
public:
    Server(){return;}
    Server(int seg_s, int seg_n){
        init = 1;
        attr.mq_flags = 0;
        attr.mq_maxmsg = 10;
        attr.mq_msgsize = sizeof(struct message);
        main_mq = mq_open("/main", O_CREAT | O_EXCL | O_RDWR | O_NONBLOCK, 0666, &attr);
        if(main_mq < 0){
            fprintf(stderr, "Error: %s\n", strerror(errno));
            throw std::runtime_error("open msg queue fail!");
        }
        resp_mq = mq_open("/main_resp", O_CREAT | O_EXCL | O_RDWR | O_NONBLOCK, 0666, &attr);
        if(resp_mq < 0){
            fprintf(stderr, "Error: %s\n", strerror(errno));
            throw std::runtime_error("open msg queue fail!");
        }
        msg_buffer = new struct message();
        num_segments = seg_n;
        seg_size = seg_s;
        for(int i=0;i<num_segments;i++){
            segments[i] = new segment(i, seg_size);
        }
        output_buffer = (char*)malloc(seg_size);
        snappy_init_env(&env);
    }
    ~Server(){
        if(!init){return;}
        if(mq_close(main_mq) < 0){
            std::cout<<"close server mq failed"<<std::endl;
            fprintf(stderr, "Error: %s\n", strerror(errno));
        }
        if(mq_unlink("/main") < 0){
            std::cout<<"delete server mq failed"<<std::endl;
            fprintf(stderr, "Error: %s\n", strerror(errno));
        }
        if(mq_close(resp_mq) < 0){
            std::cout<<"close server mq failed"<<std::endl;
            fprintf(stderr, "Error: %s\n", strerror(errno));
        }
        if(mq_unlink("/main_resp") < 0){
            std::cout<<"delete server mq failed"<<std::endl;
            fprintf(stderr, "Error: %s\n", strerror(errno));
        }
        for(auto p:clients){
            delete p.second;
        }
        for(auto s:segments){
            delete s.second;
        }
        delete msg_buffer;
        free(output_buffer);
        snappy_free_env(&env);
    }
    void init_mq();
    void serve();
    void msg_handler();
    int init_client();
    bool check_and_retrive(mqd_t);
    int genrate_mq_to_check();
    void compress(char*);
    void decompress(char*);
    void send_init_reply(int);
    int num_segments;
    int seg_size;
    void delete_client(int);

    mqd_t main_mq;
    mqd_t resp_mq;
    struct message* msg_buffer;
    int global_counter = 0;
    std::unordered_map<int, client_info*> clients;
    std::unordered_map<int, segment*> segments;
    char* output_buffer;
    struct snappy_env env;
    struct mq_attr attr;
    int init = 0;
    int queue_counter = 0;
};


#endif