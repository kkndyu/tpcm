#!/bin/bash

virsh destroy vm_amd64_virtio
sleep 6
rmmod vhost_net
insmod ./vhost_net.ko
virsh start vm_amd64_virtio
