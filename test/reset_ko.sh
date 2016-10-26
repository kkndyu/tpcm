#!/bin/bash
rmmod ./test_send.ko
rmmod ./virtio_tpcm.ko
insmod ./virtio_tpcm.ko
insmod ./test_send.ko
