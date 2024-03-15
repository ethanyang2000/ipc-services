#include "tinylib.hpp"
#include <stdio.h>
#include <sys/time.h>
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
long calculateCST(struct timeval start, struct timeval end) {
    long seconds = (end.tv_sec - start.tv_sec);
    long micros = ((seconds * 1000000) + end.tv_usec) - (start.tv_usec);
    return micros;
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
    bool state_bool = true;
    
    struct timeval startTime, returntime, endTime;
    gettimeofday(&startTime, NULL);

    c = new tiny_client();
    c->Compress(file, state_bool, callback_func);
    gettimeofday(&returntime, NULL);
    std::cout<<"nonblocking after async call"<<std::endl;
    while(!done){}
    gettimeofday(&endTime, NULL);
    long returntime1 = calculateCST(startTime, returntime);
    long cst = calculateCST(startTime, endTime);
    printf("Asyncronized Client-Side main Thread return Time (CST): %ld microseconds\n", returntime1);
    printf("Asyncronized Client-Side Callback Return Time (CST): %ld microseconds\n", cst);
    //delete c;

    //sync call
    state_bool = false;
    gettimeofday(&startTime, NULL);

    //c = new tiny_client();
    c->Compress(file, state_bool, nullptr);
    
    std::cout<<"nonblocking after async call"<<std::endl;
    gettimeofday(&endTime, NULL);
    cst = calculateCST(startTime, endTime);
    printf("Syncronized Client-Side Service Time (CST): %ld microseconds\n", cst);
    delete c;
}