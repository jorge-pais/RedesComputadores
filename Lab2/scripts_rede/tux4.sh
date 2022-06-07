#!/bin/bash

echo "Restarting network service"
sudo /etc/init.d/networking restart
#sudo systemctl restart networking

echo 1 > /proc/sys/net/ipv4/ip_forward
echo 0 > /proc/sys/net/ipv4/icmp_echo_ignore_broadcasts

ifconfig eth0 up 172.16.20.254/24
ifconfig eth1 up 172.16.21.253/24
echo "IP address set"

route add default gw 172.16.21.254
echo "Default router set"

# the DNS server ip for classroom I320 is 172.16.2.1 instead of 
# 
echo $'search netlab.fe.up.pt\nnameserver 172.16.2.1' > /etc/resolv.conf
echo "DNS set"