#include <linux/netdevice.h>
#include <linux/module.h>
MODULE_LICENSE("GPL");

extern int dev_xmit_tpcm(char * eth, u_char* pkt, int pkt_len);

static u_char pkt[] = 
    "\x00\x00\x10\x00"
    "hello virtio_tcp";


static int __init init(void)
{
    printk(KERN_ALERT "virtio_tpcm test send\n");

    dev_xmit_tpcm("eth0", pkt, 20);

    return 0;
}

static void __exit fini(void)
{
    printk(KERN_ALERT "virtio_tpcm  test send exit\n");
}

module_init(init);
module_exit(fini);
