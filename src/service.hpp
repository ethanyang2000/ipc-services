#ifndef SERVICE
#define SERVICE

#include "include.hpp"

class client_info{
    client_info(int client_id){
        id = client_id;
        sem_name = "/"+std::to_string(id)+"_sem";
        mq_name = "/"+std::to_string(id)+"_mq";

        client_mq = mq_open(mq_name, O_CREAT | O_EXCL | O_RDWR, 0666);
        if(client_mq < 0){
            std::cout<<"open client "<< id <<" msg queue fail!"<<endl;
            throw std::runtime_error("open client msg queue fail!");
        }
        sem = sem_open(sem_name, O_CREAT | O_EXCL | O_RDWR, 0666, 1);
    }

    ~client_info(){
        if (sem_close(sem) == -1) {
            std::cout<<"sem close at "<<id<<"failed"<<endl;
            throw std::runtime_error("sem close fail!");
        }
        if(sem_unlink(sem_name) == -1){
            std::cout<<"sem delete at "<<id<<"failed"<<endl;
            throw std::runtime_error("sem delete fail!");
        }
        if (mq_close(client_mq) == -1) {
            std::cout<<"mq close at "<<id<<"failed"<<endl;
            throw std::runtime_error("mq close fail!");
        }
        if(mq_unlinke(mq_name) == -1){
            std::cout<<"mq delete at "<<id<<"failed"<<endl;
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
}

class segment{
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

        fd = shm_open(mem_name, O_CREAT | O_EXCL | O_RDWR, 0666);
        if(fd < 0){
            std::cout<<"shared mem "<<id<<" init failed"<<endl;
            throw std::runtime_error("open shared mem fail!");
        }
        ftruncate(fd,seg_size);
        sem = sem_open(sem_name, O_CREAT | O_EXCL | O_RDWR, 0666, 1);

        mem_ptr = mmap(0,seg_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (mem_ptr == MAP_FAILED) {
            std::cout<<"mmap "<<id<<" failed"<<endl;
        }
    }

    ~segment(){
        if (munmap(mem_ptr, seg_size) == -1) {
            std::cout<<"unmap failed"<<endl;
        }
        mem_ptr = nullptr;
        close(fd);
        if(shm_unlink(mem_name) == -1){
            std::cout<<"shared mem delete at "<<id<<"failed"<<endl;
            throw std::runtime_error("delete shared mem fail!");
        }
        if (sem_close(sem) == -1) {
            std::cout<<"sem close at "<<id<<"failed"<<endl;
            throw std::runtime_error("sem close fail!");
        }
        if(sem_unlink(sem_name) == -1){
            std::cout<<"sem delete at "<<id<<"failed"<<endl;
            throw std::runtime_error("sem delete fail!");
        }
    }
}

class Server{
    Server(int seg_s, int seg_n){
        main_mq = mq_open("/main", O_CREAT | O_EXCL | O_RDWR | O_NONBLOCK, 0666);
        if(main_mq < 0){
            std::cout<<"open msg queue fail!"<<endl;
            throw std::runtime_error("open msg queue fail!");
        }
        msg_buffer = new struct message();
        num_segments = seg_n;
        seg_size = seg_s;
        for(int i=0;i<num_segments;i++){
            segments[i] = new segment(i, seg_size);
        }
        output_buffer = (char*)malloc(seg_size);
    }
    ~Server(){
        if(mq_close(main_mq) < 0){
            std::cout<<"close server mq failed"<<endl;
            throw std::runtime_error("close server mq failed!");
        }
        if(mq_unlinke("/main") < 0){
            std::cout<<"delete server mq failed"<<endl;
            throw std::runtime_error("delete server mq failed!");
        }
        for(auto p:clients){
            delete p.second;
        }
        for(auto s:segments){
            delete s.second;
        }
        delete msg_buffer;
        free(output_buffer);
    }
    void init_mq();
    void serve();
    void msg_handler();
    int init_client();
    bool check_and_retrive(mqd_t);
    int genrate_mq_to_check();
    void compress(char*, size_t);
    void decompress(char*, size_t);
    void send_init_reply(int);
    int num_segments;
    int seg_size;
    void delete_client(int);

    mqd_t main_mq;
    struct message* msg_buffer;
    int global_counter = 0;
    unordered_map<int, client_info*> clients;
    unordered_map<int, segment*> segments;
    char* output_buffer;
}


#endif