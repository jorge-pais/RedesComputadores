#!/bin/bash

echo -n "Restarting network service"
sudo /etc/init.d/networking restart
#sudo systemctl restart networkingorce Sensing Re

ifconfig eth0 up 172.16.21.1/24
echo -n "IP address set"

route add -net 172.16.20.0/24 gw 172.16.21.253
echo -n "route added to vlan1"