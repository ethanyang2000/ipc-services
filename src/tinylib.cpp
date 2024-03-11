#include "tinylib.hpp"

tiny_client::tiny_client(){
    // open main mq
    auto main_mq = mq_open("/main", O_RDWR, 0666, NULL);
    if (main_mq == (mqd_t)-1) {
        std::cout<<"client open main mq failed"<<std::endl;
        throw std::runtime_error("client open main mq failed!");
    }

    // send init msg to main mq
    struct message msg;
    msg.msg_type = msg_t.INIT_CLIENT;
    if(mq_send(main_mq, (const char *)&msg, sizeof(struct message), 1) < 0){
        std::cout<<"client send through main mq failed"<<std::endl;
        throw std::runtime_error("client send through main mq failed!");
    }

    // waiting for reply to get the id(blocking), then use the id to open private mq/sem/shared_mem
    // prio=2 means it is a reply from server
    mq_receive(main_mq, (const char *)&msg, sizeof(struct message), 2);
    id = msg.id;
    num_seg = msg.num_seg;
    seg_size = msg.seg_size;
    mem_id = id % num_seg;

    mem_sem_name = "/" + std::to_string(mem_id) + "_mem_sem";
    mem_name = "/" + std::to_string(mem_id) + "_mem";
    mq_sem_name = "/" + std::to_string(id) + "_sem";
    mq_name = "/" + std::to_string(id) + "_mq";
    mq_close(main_mq);

    mq = mq_open(mq_name, O_RDWR, 0666, NULL);
    if (mq == (mqd_t)-1) {
        std::cout<<"client open mq failed"<<std::endl;
        throw std::runtime_error("client open mq failed!");
    }

    fd = shm_open(mem_name, O_RDWR, 0666);
    if(fd < 0){
        std::cout<<"shared mem "<<id<<" init failed"<<std::endl;
        throw std::runtime_error("client shared mem failed!");
    }
    ftruncate(fd,seg_size);
    mem_ptr = mmap(0,seg_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (mem_ptr == MAP_FAILED) {
        std::cout<<"mmap "<<id<<" failed"<<std::endl;
        throw std::runtime_error("mmap failed!");
    }

    mq_sem = sem_open(mq_sem_name, O_RDWR, 0666, 1);
    mem_sem = sem_open(mem_sem_name, O_RDWR, 0666, 1);
}

tiny_client::~tiny_client(){
    sem_close(mem_sem);
    sem_close(mq_sem);
    mq_close(mq);
    close(fd);
}

void tiny_client::Compress(){
    
    // use mutex to protect shared mem
    // problem: if multiple tasks are locked at mutex, the execution order is not defined

    // if sync
    // sem - 1
    // do...
    // sem - 1
    // call back
    // sem + 1

    // if async
    // fork a new thread and return
    // sem - 1
    // do...
    // sem - 1
    // callback
    // sem + 1
}