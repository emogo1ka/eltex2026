#include <sys/socket.h>
#include <linux/netlink.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#define NETLINK_USER 31
#define MAX_PAYLOAD 1024

int main() {
    int sock_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_USER);
    if (sock_fd < 0) {
        printf("socket creation error\n");
        return -1;
    }

    struct sockaddr_nl src_addr, dest_addr;
    
    memset(&src_addr, 0, sizeof(src_addr));
    src_addr.nl_family = AF_NETLINK;
    src_addr.nl_pid = getpid();

    bind(sock_fd, (struct sockaddr *)&src_addr, sizeof(src_addr));

    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.nl_family = AF_NETLINK;
    dest_addr.nl_pid = 0; 

    struct nlmsghdr *nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
    memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
    nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
    nlh->nlmsg_pid = getpid();
    nlh->nlmsg_flags = 0;

    strcpy(NLMSG_DATA(nlh), "nihao kernel");

    struct iovec iov;
    iov.iov_base = (void *)nlh;
    iov.iov_len = nlh->nlmsg_len;

    struct msghdr msg;
    memset(&msg, 0, sizeof(msg));
    msg.msg_name = (void *)&dest_addr;
    msg.msg_namelen = sizeof(dest_addr);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    printf("Sending msg to kernel\n");
    sendmsg(sock_fd, &msg, 0);

    printf("Waiting for response\n");
    recvmsg(sock_fd, &msg, 0);
    
    printf("Got from kernel: %s\n", (char *)NLMSG_DATA(nlh));
    
    close(sock_fd);
    free(nlh);
    return 0;
}
