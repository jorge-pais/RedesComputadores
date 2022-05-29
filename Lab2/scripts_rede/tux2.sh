#!/bin/bash

echo -n "Restarting network service"
sudo /etc/init.d/networking restart
#sudo systemctl restart networkingorce Sensing Re

ifconfig eth0 up 172.16.21.1/24
echo -n "IP address set"

route add -net 172.16.20.0/24 gw 172.16.21.253
echo -n "route to vlan1 added"

route add default gw 172.16.21.254
echo -n "Default router set"

#delete route Q4
#route del -net 172.16.20.0 gw 172.16.21.253 netmask 255.255.255.0 dev eth0