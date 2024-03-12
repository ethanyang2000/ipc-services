#include "tinylib.hpp"
#include <fstream>
#include <thread>
#include <vector>

tiny_client::tiny_client(){
    // open main mq
    struct mq_attr attr;
    attr.mq_msgsize = sizeof(struct message);
    auto main_mq = mq_open("/main", O_RDWR, 0666, &attr);
    if (main_mq == (mqd_t)-1) {
        std::cout<<"client open main mq failed"<<std::endl;
        throw std::runtime_error("client open main mq failed!");
    }
    auto resp_mq = mq_open("/main_resp", O_RDWR, 0666, &attr);
    if (resp_mq == (mqd_t)-1) {
        std::cout<<"client open main mq failed"<<std::endl;
        throw std::runtime_error("client open main mq failed!");
    }

    // send init msg to main mq
    struct message msg;
    msg.msg_type = msg_t::INIT_CLIENT;
    if(mq_send(main_mq, (const char *)&msg, sizeof(struct message), 1) < 0){
        std::cout<<"client send through main mq failed"<<std::endl;
        throw std::runtime_error("client send through main mq failed!");
    }

    // waiting for reply to get the id(blocking), then use the id to open private mq/sem/shared_mem
    // prio=2 means it is a reply from server
    unsigned int temp = 1;
    mq_receive(resp_mq, (char *)&msg, sizeof(struct message), &temp);
    std::cout<<"rec client "<<msg.id<<std::endl;
    id = msg.id;
    num_seg = msg.num_seg;
    seg_size = msg.seg_size;
    mem_id = id % num_seg;

    mem_sem_name = "/" + std::to_string(mem_id) + "_mem_sem";
    mem_name = "/" + std::to_string(mem_id) + "_mem";
    mq_sem_name = "/" + std::to_string(id) + "_sem";
    mq_name = "/" + std::to_string(id) + "_mq";
    mq_close(main_mq);
    mq_close(resp_mq);

    mq = mq_open(mq_name.data(), O_RDWR, 0666, &attr);
    if (mq == (mqd_t)-1) {
        std::cout<<"client open mq failed"<<std::endl;
        throw std::runtime_error("client open mq failed!");
    }

    fd = shm_open(mem_name.data(), O_RDWR, 0666);
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

    mq_sem = sem_open(mq_sem_name.data(), O_RDWR, 0666, 1);
    mem_sem = sem_open(mem_sem_name.data(), O_RDWR, 0666, 1);

    chunk_size = int(seg_size * 0.9) - sizeof(size_t);
}

tiny_client::~tiny_client(){
    sem_close(mem_sem);
    sem_close(mq_sem);
    mq_close(mq);
    close(fd);
}

void tiny_client::compress_chunk(){
    sem_wait(mq_sem);
    // send request
    struct message msg;
    msg.msg_type = msg_t::COMPRESS;
    msg.id = id;
    if(mq_send(mq, (const char *)&msg, sizeof(struct message), 1) < 0){
        std::cout<<"client send through main mq failed"<<std::endl;
        throw std::runtime_error("client send through main mq failed!");
    }

    sem_wait(mq_sem);

    //call back
    sem_post(mq_sem);
}

void tiny_client::decompress_chunk(){
    sem_wait(mq_sem);
    // send request
    struct message msg;
    msg.msg_type = msg_t::DECOMPRESS;
    msg.id = id;
    if(mq_send(mq, (const char *)&msg, sizeof(struct message), 1) < 0){
        std::cout<<"client send through main mq failed"<<std::endl;
        throw std::runtime_error("client send through main mq failed!");
    }

    sem_wait(mq_sem);

    //call back
    sem_post(mq_sem);
}

void tiny_client::async_call(std::string filename, callback_f callback){
    do_compress(filename);
    callback((char*)(mem_ptr));
}

void tiny_client::Compress(std::string filename, bool async, callback_f callback){
    if(async){
        auto async = [this, filename, callback]() {
            this->async_call(filename, callback);
        };
        std::thread myThread(async);
        myThread.detach();
    }else{
        do_compress(filename);
    }
}

void tiny_client::do_compress(std::string filename){
    std::string output_name = filename + ".compressed";
    size_t pos = filename.find_last_of('/');
    std::string new_name;
    if (pos != std::string::npos) {
        new_name = filename.substr(0, pos + 1) + "new_" + filename.substr(pos + 1);
    } else {
        new_name = "new_" + filename;
    }

    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return;
    }
    std::ofstream output_file(output_name, std::ios::binary | std::ios::app);
    std::ofstream new_file(new_name, std::ios::binary | std::ios::app);

    file.seekg(0, std::ios::end);
    std::streampos file_size = file.tellg();
    file.seekg(0, std::ios::beg);

    size_t num_chunks = (file_size + chunk_size - 1) / chunk_size;
    sem_wait(mem_sem);

    for (size_t i = 0; i < num_chunks; ++i) {
        size_t curr_chunk_size = (i == num_chunks - 1) ? (file_size % chunk_size) : chunk_size;

        std::vector<char> chunk(curr_chunk_size);
        file.read(chunk.data(), curr_chunk_size);

        if (!file) {
            std::cerr << "Error reading chunk from file.\n";
            return;
        }

        std::memcpy(mem_ptr, &curr_chunk_size, sizeof(size_t));
        std::memcpy(mem_ptr+sizeof(size_t), chunk.data(), curr_chunk_size);
        compress_chunk();

        if (!output_file.is_open()) {
            std::cerr << "Error opening file: " << output_name << std::endl;
            return;
        }
        size_t out_size;
        std::memcpy(&out_size, mem_ptr, sizeof(size_t));
        output_file.write((const char*)(mem_ptr)+sizeof(size_t), out_size);

        if (!new_file.is_open()) {
            std::cerr << "Error opening file: " << new_name << std::endl;
            return;
        }

        decompress_chunk();
        std::memcpy(&out_size, mem_ptr, sizeof(size_t));
        new_file.write((const char*)(mem_ptr)+sizeof(size_t), out_size);
    }

    file.close();
    output_file.close();
    new_file.close();
    sem_post(mem_sem);
}