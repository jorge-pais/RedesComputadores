#!/bin/bash

echo -n "Restarting network service"
sudo /etc/init.d/networking restart
#sudo systemctl restart networking

echo 1 > /proc/sys/net/ipv4/ip_forward
echo 0 > /proc/sys/net/ipv4/icmp_echo_ignore_broadcasts

ifconfig eth0 up 172.16.20.254/24
ifconfig eth1 up 172.16.21.253/24
echo -n "IP address set"

route add default gw 172.16.21.254
echo -n "Default router set"