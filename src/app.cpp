#include "tinylib.hpp"
static tiny_client* c;
static bool done = false;

void sig_exit_handler(int signal) {
    delete c;
    exit(signal);
}

void callback_func(char* inp){
    std::cout<<"callback called after done"<<std::endl;
    done = true;
}

int main(int argc, char *argv[]){
    signal(SIGSEGV, sig_exit_handler);
    signal(SIGILL, sig_exit_handler);
    signal(SIGFPE, sig_exit_handler);
    signal(SIGABRT, sig_exit_handler);
    signal(SIGBUS, sig_exit_handler);
    signal(SIGINT, sig_exit_handler);

    std::string state, file, files;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "--state" && i + 1 < argc) {
            state = argv[++i];
        } else if (arg == "--file" && i + 1 < argc) {
            file = argv[++i];
        } else if (arg == "--files" && i + 1 < argc) {
            files = argv[++i];
        }
    }
    bool state_bool = state == "SYNC"? false:true;
    c = new tiny_client();
    c->Compress(file, state_bool, callback_func);
    std::cout<<"nonblocking after async call"<<std::endl;
    while(!done){}
    delete c;
}