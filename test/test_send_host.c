#include <linux/netdevice.h>
#include <linux/module.h>
MODULE_LICENSE("GPL");

extern int dev_xmit_tpcm_host(char * eth, u_char* pkt, int pkt_len);

static u_char* pkt = "hello virtio_tcp_guest";

static int __init init(void)
{
    printk(KERN_ALERT "virtio_tpcm test send\n");

    dev_xmit_tpcm_host("macvtap0", pkt, 30);

    return 0;
}

static void __exit fini(void)
{
    printk(KERN_ALERT "virtio_tpcm  test send exit\n");
}

module_init(init);
module_exit(fini);
