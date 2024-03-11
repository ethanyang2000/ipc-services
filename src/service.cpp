#include "service.hpp"

bool Server::check_and_retrive(mqd_t inp_mq){
    struct mq_attr attr;
    if (mq_getattr(inp_mq, &attr) == -1) {
        std::cout<<"check mq status failed"<<std::endl;
        throw std::runtime_error("check msg queue fail!");
    }
    if (attr.mq_curmsgs > 0) {
        unsigned int temp = 1;
        if (mq_receive(inp_mq, (char *)msg_buffer, sizeof(struct message), &temp) == -1) {
            std::cout<<"get mq message failed"<<std::endl;
            throw std::runtime_error("get msg fail!");
        }
        return true;
    }else{
        return false;
    }
}

int Server::genrate_mq_to_check(){
    return 1;
}

void Server::serve(){
    while(true){
        // check init requests
        while(true){
            unsigned int temp = 1;
            if (mq_receive(main_mq, (char *)msg_buffer, sizeof(struct message), &temp) == -1) {
                if (errno == EAGAIN) {
                    break;
                } else {
                    throw std::runtime_error("receive init msg failed!");
                }
            } else {
                msg_handler();
            }
        }
        // TODO: where the QoS can be applied
        int client_mq_id = genrate_mq_to_check();
        if(check_and_retrive(clients[client_mq_id]->client_mq)){
            msg_handler();
        }
    }
}

int Server::init_client(){
    // init client
    auto new_client = new client_info(global_counter);
    clients[global_counter++] = new_client;
    return global_counter - 1;
}

void Server::delete_client(int target){
    delete clients[target];
    clients.erase(target);
}

void Server::compress(char* inp_buffer){
    size_t inp_len;
    size_t out_len;
    std::memcpy(&inp_len, inp_buffer, sizeof(size_t));
    snappy_compress(&env, inp_buffer+sizeof(size_t), inp_len, output_buffer, &out_len);
    std::memcpy(inp_buffer, &out_len, sizeof(size_t));
    std::memcpy(inp_buffer+sizeof(size_t), output_buffer, out_len);
}

void Server::decompress(char* inp_buffer){
    size_t inp_len;
    size_t out_len;
    std::memcpy(&inp_len, inp_buffer, sizeof(size_t));
    snappy_uncompressed_length(inp_buffer+sizeof(size_t), inp_len, &out_len);
    snappy_uncompress(inp_buffer+sizeof(size_t), inp_len, output_buffer);
    std::memcpy(inp_buffer, &out_len, sizeof(size_t));
    std::memcpy(inp_buffer+sizeof(size_t), output_buffer, out_len);
}

void Server::msg_handler(){
    if(msg_buffer->msg_type == msg_t::INIT_CLIENT){
        int init_id = init_client();
        msg_buffer->msg_type = msg_t::INIT_REPLY;
        msg_buffer->id = init_id;
        msg_buffer->seg_size = seg_size;
        msg_buffer->num_seg = num_segments;
        if (mq_send(main_mq, (const char *)msg_buffer, sizeof(struct message), 2) == -1) {
            std::cout<<"send through main msg queue failed"<<std::endl;
            throw std::runtime_error("send reply fail!");
        }
    }else if(msg_buffer->msg_type == msg_t::COMPRESS){
        int mem_id = msg_buffer->id % num_segments;
        compress((char*)segments[mem_id]->mem_ptr);
        sem_post(clients[msg_buffer->id]->sem);
    }else if(msg_buffer->msg_type  == msg_t::DECOMPRESS){
        int mem_id = msg_buffer->id % num_segments;
        decompress((char*)segments[mem_id]->mem_ptr);
        sem_post(clients[msg_buffer->id]->sem);
    }
}

int main(int argc, char *argv[]){
    int seg_num;
    int seg_size;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "--n_sms" && i + 1 < argc) {
            seg_num = std::atoi(argv[i + 1]);
            ++i;
        }else if (arg == "--sms_size" && i + 1 < argc) {
            seg_size = std::atoi(argv[i + 1]);
            ++i;
        }
    }
    auto s = Server(seg_size, seg_num);
    s.serve();
}