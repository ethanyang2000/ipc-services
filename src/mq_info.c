#include <stdio.h>
#include <mqueue.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <mq_name>\n", argv[0]);
        return 1;
    }

    const char *mq_name = argv[1];
    struct mq_attr attr;

    mqd_t mq = mq_open(mq_name, O_RDONLY);
    if (mq == (mqd_t)-1) {
        perror("mq_open");
        return 1;
    }

    if (mq_getattr(mq, &attr) == -1) {
        perror("mq_getattr");
        mq_close(mq);
        return 1;
    }

    printf("Number of messages in the queue '%s': %ld\n", mq_name, attr.mq_curmsgs);

    mq_close(mq);

    return 0;
}
