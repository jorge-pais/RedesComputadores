#!/bin/bash

echo "Restarting network service"
sudo /etc/init.d/networking restart
#sudo systemctl restart networkingorce Sensing Re

ifconfig eth0 up 172.16.21.1/24
echo "IP address set"

route add -net 172.16.20.0/24 gw 172.16.21.253
echo "route to vlan1 added"

route add default gw 172.16.21.254
echo "Default router set"

#delete route Q4
#route del -net 172.16.20.0 gw 172.16.21.253 netmask 255.255.255.0 dev eth0

echo $'search netlab.fe.up.pt\nnameserver 172.16.2.1' > /etc/resolv.conf
echo "DNS set"