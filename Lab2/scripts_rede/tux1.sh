#!/bin/bash

echo "Restarting network service"
sudo /etc/init.d/networking restart
#sudo systemctl restart networking

ifconfig eth0 up 172.16.20.1/24
echo "IP address set"

route add -net 172.16.21.0/24 gw 172.16.20.254
echo "route added to vlan1"

route add default gw 172.16.20.254

printf "search netlab.fe.up.pt\nnameserver 172.16.2.1" > /etc/resolv.conf
echo "DNS set"