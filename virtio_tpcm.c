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

    printk("in dev_xmit_tpcm\n");

    dev = dev_get_by_name(&init_net, eth);

    //skb = alloc_skb (pkt_len + LL_RESERVED_SPACE(dev) , GFP_ATOMIC);
    skb = alloc_skb (pkt_len + 12 + 14 , GFP_ATOMIC);
    if (NULL == skb) goto out;
    printk("alloc_skb success\n");

    //skb_reserve(skb, LL_RESERVED_SPACE(dev));
    skb_reserve(skb, 12+14);
    skb->dev = dev;
    skb->pkt_type = PACKET_OTHERHOST;

    if(pkt == NULL) {
        printk("pkt pointer is null\n");
        goto out;
    }

    pdata = skb_put(skb, pkt_len);
    printk("%p\n", pdata);
    printk("before memcpy pkt\n");
    if (NULL != pkt) memcpy(pdata, pkt, pkt_len);
    printk("after memcpy pkt\n");

    /*
     * Add fake ethernet header
     * need a proto_type for tpcm
     *
     * BEEF
     *
     * */
    //skb->mac_header = skb_push(skb, 14);
    //printk("%p\n",skb_push(skb, 14));
    ethdr = (struct ethhdr *) skb_push(skb, 14);
    unsigned char dest[] = "\x94\xde\x80\xfa\xa2\x36";
    memcpy(ethdr->h_dest, dest, ETH_ALEN);
    printk("%p\n", ethdr->h_dest);
    unsigned char src[] = "\x12\x34\x56\xab\xcd\xee";
    memcpy(ethdr->h_source, src, ETH_ALEN);
    //memcpy(ethdr->h_source, dev->dev_addr, ETH_ALEN);
    printk("%p\n", ethdr->h_source);
    ethdr->h_proto = __constant_htons(0xBEEF);

    printk("before xmit\n");
    if (0 > dev_queue_xmit(skb)) goto out;
    printk("after xmit\n");
    nret = 0;

out:
    //if (0 != nret && NULL != skb) {dev_put(dev); kfree_skb(skb);}
    if (0 != nret && NULL != skb) { kfree_skb(skb); }
    dev_put(dev);
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
EXPORT_SYMBOL_GPL(dev_xmit_tpcm_host);
