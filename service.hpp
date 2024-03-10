#ifndef SERVICE
#define SERVICE

#include "include.hpp"
#include <unordered_map.h>

class client_info{
    client_info(int);
    int map_shared_mem(){
        // TODO: mmap set up
        void* ptr = mmap(0,SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (ptr == MAP_FAILED) {
            cout<<"mmap "<<id<<" failed"<<endl;
            return -1;
        }
        mem_ptr = ptr;
        return 1;
    }
    int unmap_shared_mem(){
        //TODO: munmap set up
        if (munmap(mem_ptr, SHM_SIZE) == -1) {
            cout<<"unmap failed"<<endl;
            return -1;
        }
        mem_ptr = nullptr;
        return 0;
    }

    // unique name
    int id;
    // shared mem fd
    int fd;
    // shared mem ptr
    void* mem_ptr;
    // msg queue
    mqd_t client_mq;
    // semaphore
    sem_t* sem;
    std::string sem_name;
    std::string mq_name;
    std::string mem_name;
}

class Server{
    Server(){
        init_mq();
        server_sem = sem_open("/main_sem", O_CREAT | O_EXCL | O_RDWR, 0666, 1);
        msg_buffer = new message();
    }
    ~Server(){
        if(mq_close(mq_id) < 0){
            cout<<"close server mq failed"<<endl;
        }
        if(mq_unlinke("/main") < 0){
            cout<<"delete server mq failed"<<endl;
        }
        if (sem_close(server_sem) == -1) {
            cout<<"close server sem failed"<<endl;
        }

        if (sem_unlink("/my_semaphore") == -1) {
            cout<<"delete server sem failed"<<endl;
        }
        for(auto p:clients){
            delete p.second;
        }
        delete msg_buffer;
    }
    void init_mq();
    void serve();
    void msg_handler(message*);
    void init_client();
    void init_snappy();
    message* check_and_retrive(mqd_t);
    int genrate_mq_to_check();
    void compress(char*, size_t);
    void decompress(char*, size_t);

    mqd_t mq_id;
    set_t* server_sem;
    message* msg_buffer;
    int global_counter = 0;
    unordered_map<int, client_info*> clients;
}


#endif