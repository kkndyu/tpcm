#include <linux/netdevice.h>
#include <linux/module.h>
MODULE_LICENSE("GPL");

extern int dev_xmit_tpcm(char * eth, u_char* pkt, int pkt_len);
extern int tpcm_request(void *sbuffer, int ssize, void **rbuffer, int *rsize);

static u_char pkt[] = 
    "\x00\x00\x10\x00"
    "hello virtio_tcp";


static int __init init(void)
{
    void *rbuffer = NULL;
    int rsize = 0;
    printk(KERN_ALERT "virtio_tpcm test send\n");

    //dev_xmit_tpcm("eth0", pkt, 20);
    tpcm_request((void *)pkt, 20, &rbuffer, &rsize);

    return 0;
}

static void __exit fini(void)
{
    printk(KERN_ALERT "virtio_tpcm  test send exit\n");
}

module_init(init);
module_exit(fini);
