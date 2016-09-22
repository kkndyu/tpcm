#include <linux/netdevice.h>
#include <linux/module.h>
MODULE_LICENSE("GPL");

/*
 * Maybe need a config file to
 * identify which net interface
 * will be used.
 *
 * */
 int dev_xmit_tpcm(char * eth)
{
    struct net_device * dev = NULL;
    struct sk_buff * skb = NULL;
    int nret = 1;

    printk("using tpcm");

    dev = dev_get_by_name (&init_net, eth);
    skb = alloc_skb (1024 + LL_RESERVED_SPACE (dev), GFP_ATOMIC);

    skb_reserve (skb, LL_RESERVED_SPACE (dev));
    skb->dev = dev;
    /*
     * Add fake ethernet header
     * need a proto_type for tpcm
     *
     * */

    if (0 > dev_queue_xmit(skb)) goto out;
    nret = 0;

out:
    if (0 != nret && NULL != skb) {dev_put (dev); kfree_skb (skb);}

    return 0;
}

static int __init init(void)
{
    printk(KERN_ALERT "virtio_tpcm init\n");
    return 0;
}

static void __exit fini(void)
{
    printk(KERN_ALERT "virtio_tpcm exit\n");
}

module_init(init);
module_exit(fini);
EXPORT_SYMBOL_GPL(dev_xmit_tpcm);
