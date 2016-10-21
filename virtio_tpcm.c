#include <linux/netdevice.h>
#include <linux/module.h>
MODULE_LICENSE("GPL");

/*
 * Maybe need a config file to
 * identify which net interface
 * will be used.
 *
 * */
 int dev_xmit_tpcm(char * eth, u_char* pkt, int pkt_len)
{
    struct net_device * dev = NULL;
    struct sk_buff * skb = NULL;
    int nret = 1;
    struct ethhdr * ethdr = NULL;
    u_char * pdata = NULL;

    printk("using tpcm");

    dev = dev_get_by_name (&init_net, eth);

    skb = alloc_skb (pkt_len + 14 , GFP_ATOMIC);
    if (NULL == skb) goto out;

    skb_reserve (skb, 14);
    skb->dev = dev;
    skb->pkt_type = PACKET_KERNEL;

    pdata = skb_put (skb, pkt_len);
    if (NULL != pkt) memcpy (pdata, pkt, pkt_len);

    /*
     * Add fake ethernet header
     * need a proto_type for tpcm
     *
     * BEEF
     *
     * */
    skb->mac_header = skb_push (skb, 14);
    ethdr = (struct ethhdr *) skb->mac_header;
    memcpy (ethdr->h_dest, "94de80faa236", ETH_ALEN);
    memcpy (ethdr->h_source, "123456abcdee", ETH_ALEN);
    ethdr->h_proto = 0xBEEF;

    if (0 > dev_queue_xmit(skb)) goto out;
    nret = 0;

out:
    if (0 != nret && NULL != skb) {dev_put (dev); kfree_skb (skb);}
    printk("something error");

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
