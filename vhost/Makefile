obj-$(CONFIG_VHOST_NET) += vhost_net.o
vhost_net-y := net.o

obj-$(CONFIG_VHOST)	+= vhost.o

all:
	$(MAKE) -C /lib/modules/`uname -r`/build M=$(PWD) modules

clean:
	$(MAKE) -C /lib/modules/`uname -r`/build M=$(PWD) clean
	$(RM) Module.markers modules.order
