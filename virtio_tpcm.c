#include <linux/netdevice.h>
#include <linux/module.h>
#include <linux/semaphore.h>
MODULE_LICENSE("GPL");

static char *eth = "eth0";
module_param(eth, charp, S_IRUGO|S_IWUSR);

extern struct semaphore virtio_tpcm_sem;
extern void *virtio_tpcm_rbuffer;
extern int virtio_tpcm_rsize;
extern int16_t virtio_tpcm_series;
struct semaphore request_sem;

int dev_xmit_tpcm(char * eth, u_char* pkt, int pkt_len);
void HexDump(char *buf,int len,int addr) {
    int i,j,k;
    char binstr[80];

    for (i=0;i<len;i++) {
        if (0==(i%16)) {
            sprintf(binstr,"%08x -",i+addr);
            sprintf(binstr,"%s %02x",binstr,(unsigned char)buf[i]);
        } else if (15==(i%16)) {
            sprintf(binstr,"%s %02x",binstr,(unsigned char)buf[i]);
            sprintf(binstr,"%s  ",binstr);
            for (j=i-15;j<=i;j++) {
                sprintf(binstr,"%s%c",binstr,('!'<buf[j]&&buf[j]<='~')?buf[j]:'.');
            }
            printk("%s\n",binstr);
        } else {
            sprintf(binstr,"%s %02x",binstr,(unsigned char)buf[i]);
        }
    }
    if (0!=(i%16)) {
        k=16-(i%16);
        for (j=0;j<k;j++) {
            sprintf(binstr,"%s   ",binstr);
        }
        sprintf(binstr,"%s  ",binstr);
        k=16-k;
        for (j=i-k;j<i;j++) {
            sprintf(binstr,"%s%c",binstr,('!'<buf[j]&&buf[j]<='~')?buf[j]:'.');
        }
        printk("%s\n",binstr);
    }
}

static int __dev_xmit_tpcm(u_char* pkt, int pkt_len, int16_t series, int16_t ordinal, int16_t total)
{
    struct net_device * dev = NULL;
    struct sk_buff * skb = NULL;
    int nret = 1;
    struct ethhdr * ethdr = NULL;
    u_char * pdata = NULL;
    int16_t tmp = 0;

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
    memcpy(pdata, pkt, pkt_len);

    ethdr = (struct ethhdr *) skb_push(skb, 14);
    //using mac header dest field as series/current/total  2 Bytes/2 Bytes/2Bytes
    pdata = (u_char *)ethdr;
    tmp = __constant_htons(series);
    memcpy(pdata, &tmp, 2);
    tmp = __constant_htons(ordinal);
    memcpy(pdata+2, &tmp, 2);
    tmp = __constant_htons(total);
    memcpy(pdata+4, &tmp, 2);
    memcpy(ethdr->h_source, dev->dev_addr, ETH_ALEN);
    ethdr->h_proto = __constant_htons(0xBEEF);

    HexDump((char *)skb->data, pkt_len+14, (int)(skb->data));

    printk("before xmit\n");
    if (0 > dev_queue_xmit(skb)) goto out;
    printk("after xmit\n");
    nret = 0;

out:
    //if (0 != nret && NULL != skb) {dev_put(dev); kfree_skb(skb);}
    if (0 != nret && NULL != skb) { kfree_skb(skb); }
    /* pair with dev_get_by_name() */
    dev_put(dev);

    return 0;
}

#define VIRTIO_TPCM_MTU 1500

int tpcm_request(void *sbuffer, int ssize, void **rbuffer, int *rsize)
{
    char* cur_buf = NULL;
    int div=0, mal=0, i;
    int16_t series = -1;
    
    //make sure only one request at a time
    down(&request_sem);
    if(sbuffer==NULL || ssize==0) return 1;

    div = ssize / VIRTIO_TPCM_MTU;
    mal = ssize % VIRTIO_TPCM_MTU;
    do{
        get_random_bytes(&series, sizeof(series));
    }while(series == -1);
    printk("===series random %d\n", series);
    virtio_tpcm_series = series;

    for(i=0,cur_buf=(char*)sbuffer;i<div;i++,cur_buf+=1500){
        __dev_xmit_tpcm(cur_buf,VIRTIO_TPCM_MTU,series,i+1,div+1);
    }
    __dev_xmit_tpcm(cur_buf,mal,series,i+1,div+1);

    printk("wait for recv by sema == %d\n", virtio_tpcm_sem.count);
    down(&virtio_tpcm_sem);
    printk("sema arrived\n");

    *rbuffer = virtio_tpcm_rbuffer;
    *rsize = virtio_tpcm_rsize;
    HexDump((char *)*rbuffer, *rsize, (int)(*rbuffer));

    virtio_tpcm_rsize = 0;


    up(&request_sem);
    return 0;
}

int tpcm_request_1(void *sbuffer, int ssize, void **rbuffer, int *rsize)
{
    //make sure only one request at a time
    down(&request_sem);
    dev_xmit_tpcm("eth0",(u_char*)sbuffer, ssize);

    printk("wait for recv by sema == %d\n", virtio_tpcm_sem.count);
    down(&virtio_tpcm_sem);
    printk("sema arrived\n");

    *rbuffer = virtio_tpcm_rbuffer;
    *rsize = virtio_tpcm_rsize;
    HexDump((char *)*rbuffer, *rsize, (int)(*rbuffer));


    up(&request_sem);
    return 0;
}
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
    //unsigned char src[] = "\x12\x34\x56\xab\xcd\xee";
    memcpy(ethdr->h_source, dev->dev_addr, ETH_ALEN);
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
    /* pair with dev_get_by_name() */
    dev_put(dev);

    return 0;
}

static int __init init(void)
{
    printk(KERN_ALERT "virtio_tpcm init\n");
    sema_init(&request_sem,1);
    virtio_tpcm_rbuffer = kmalloc(4194304, GFP_KERNEL);
    return 0;
}

static void __exit fini(void)
{
    printk(KERN_ALERT "virtio_tpcm exit\n");
    if(virtio_tpcm_rbuffer) kfree(virtio_tpcm_rbuffer);
}

int dev_xmit_tpcm_host(char * eth, u_char* pkt, int pkt_len)
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
    unsigned char dest[] = "\x52\x54\x00\xc0\x26\x6c";
    memcpy(ethdr->h_dest, dest, ETH_ALEN);
    printk("%p\n", ethdr->h_dest);
    unsigned char src[] = "\x12\x34\x56\xab\xcd\xee";
    memcpy(ethdr->h_source, src, ETH_ALEN);
    //memcpy(ethdr->h_source, dev->dev_addr, ETH_ALEN);
    printk("%p\n", ethdr->h_source);
    ethdr->h_proto = __constant_htons(0xBEEF);

    printk("before xmit\n");
    printk("ref==%d==\n", netdev_refcnt_read(dev));

    if (0 > dev_queue_xmit(skb)) goto out;

    printk("after xmit\n");
    printk("ref==%d==\n", netdev_refcnt_read(dev));
    nret = 0;

out:
    //if (0 != nret && NULL != skb) {dev_put(dev); kfree_skb(skb);}
    if (0 != nret && NULL != skb) { kfree_skb(skb);}
    /* pair with dev_get_by_name() */
    dev_put(dev);

    return 0;
}
module_init(init);
module_exit(fini);
EXPORT_SYMBOL_GPL(dev_xmit_tpcm);
EXPORT_SYMBOL_GPL(dev_xmit_tpcm_host);
EXPORT_SYMBOL_GPL(tpcm_request);
