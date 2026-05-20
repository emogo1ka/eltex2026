#include <linux/module.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>
#include <net/sock.h>
#include <linux/string.h>

#define NETLINK_USER 31

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("smells like netlink kernel module");
MODULE_AUTHOR("emogotka");

static struct sock *nl_sk = NULL;

static void nl_recv_msg(struct sk_buff *skb) {
    struct nlmsghdr *nlh;
    struct sk_buff *skb_out;
    u32 pid;
    const char *msg = "OMG THIS IS kernel message!";
    size_t msg_size = strlen(msg) + 1;
    int res;

    nlh = (struct nlmsghdr *)skb->data;
    pid = nlh->nlmsg_pid;

    skb_out = nlmsg_new(msg_size, GFP_KERNEL);
    if (!skb_out) return;

    nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, (int)msg_size, 0);
    NETLINK_CB(skb_out).dst_group = 0;
    memcpy(nlmsg_data(nlh), msg, msg_size);

    res = nlmsg_unicast(nl_sk, skb_out, pid);
    if (res < 0) {
        pr_info("Error with sending back to user\n");
    }
}

static int __init netlink_init(void) {
    struct netlink_kernel_cfg cfg = {
        .input = nl_recv_msg,
    };

    nl_sk = netlink_kernel_create(&init_net, NETLINK_USER, &cfg);
    if (!nl_sk) {
        return -ENOMEM;
    }
    return 0;
}

static void __exit netlink_exit(void) {
    netlink_kernel_release(nl_sk);
}

module_init(netlink_init);
module_exit(netlink_exit);
