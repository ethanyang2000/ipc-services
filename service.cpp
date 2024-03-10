#include <iostream>
#include "service.hpp"

void Server::init_mq(){
    mq_id = mq_open("/main", O_CREAT | O_EXCL | O_RDWR, 0666);
    if(mq_id < 0){
        cout<<"open msg queue fail!"<<endl;
        return -1;
    }

    mq_attr mqAttr;
    if(mq_getattr(mq_id, &mqAttr) < 0){
        cout<<"get msg debug info failed!"<<endl;
    }
    cout<<"mq_flags:"<<mqAttr.mq_flags<<endl;
    cout<<"mq_maxmsg:"<<mqAttr.mq_maxmsg<<endl;
    cout<<"mq_msgsize:"<<mqAttr.mq_msgsize<<endl;
    cout<<"mq_curmsgs:"<<mqAttr.mq_curmsgs<<endl;
}

void Server::init_snappy(){

}

int check_and_retrive(mqd_t inp_mq, message* msg_buffer){
    struct mq_attr attr;
    if (mq_getattr(inp_mq, &attr) == -1) {
        cout<<"check mq status failed"<<endl;
        return -1;
    }
    if (attr.mq_curmsgs > 0) {
        if (mq_receive(inp_mq, (char *)msg_buffer, sizeof(struct message), NULL) == -1) {
            cout<<"get mq message failed"<<endl;
            return -1;
        }
        return 1;
    } else {
        return -1;
    }
}

void Server::serve(){
    while(true){
        while(check_and_retrive(mq, msg_buffer) == 1){
            msg_handler(msg_buffer);
        }
        // TODO: where the QoS can be applied
        client_mq_id = genrate_mq_to_check();
        if(check_and_retrive(clients[client_mq_id], msg_buffer)){
            msg_handler(msg_buffer);
        }
    }
}

client_info::client_info(int client_id){
    id = client_id;
    sem_name = "/"+std::to_string(id)+"_sem";
    mem_name = "/"+std::to_string(id)+"_mem";
    mq_name = "/"+std::to_string(id)+"_mq";

    client_mq = mq_open(mq_name, O_CREAT | O_EXCL | O_RDWR, 0666);
    if(client_mq < 0){
        cout<<"open client "<< id <<" msg queue fail!"<<endl;
    }
    fd = shm_open(mem_name, O_CREAT | O_EXCL | O_RDWR, 0666);
    if(fd < 0){
        cout<<"shared mem "<<id<<" init failed"<<endl;
    }
    // TODO: ?
    //ftruncate(fd,SHM_SIZE);

    sem = sem_open(sem_name, O_CREAT | O_EXCL | O_RDWR, 0666, 1);
}

client_info::~client_info(){
    if (sem_close(sem) == -1) {
        cout<<"sem close at "<<id<<"failed"<<endl;
    }
    if(sem_unlink(sem_name) == -1){
        cout<<"sem delete at "<<id<<"failed"<<endl;
    }
    if (mq_close(client_mq) == -1) {
        cout<<"mq close at "<<id<<"failed"<<endl;
    }
    if(mq_unlinke(mq_name) == -1){
        cout<<"mq delete at "<<id<<"failed"<<endl;
    }
    close(fd);
    if(shm_unlink(mem_name) == -1){
        cout<<"shared mem delete at "<<id<<"failed"<<endl;
    }
}

void Server::init_client(){
    // init client
    client_info new_client = client_info(global_counter);
    clients[global_counter++] = &new_client;
}

void Server::compress(char* inp_buffer, size_t inp_length){
    snappy::Compress()
}

void Server::decompress(char* inp_buffer, size_t inp_length){
    snappy::Uncompress()
}


void Server::msg_handler(message* req){
    if(req->msg_type == msg_t.INIT_CLIENT){
        init_client();
    }else if(req->msg_type == msg_t.COMPRESS){
        compress();
    }else if(req->msg_type  == msg_t.DECOMPRESS){
        decompress();
    }
}

int main(){
    s = Server();
    s.serve();
}