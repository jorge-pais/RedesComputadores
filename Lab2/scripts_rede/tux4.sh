#!/bin/bash

echo -n "Restarting network service"
sudo /etc/init.d/networking restart
#sudo systemctl restart networking

ifconfig eth0 up 172.16.20.254/24
ifconfig eth1 up 172.16.21.253/24
echo -n "IP address set"

