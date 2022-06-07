#!/bin/sh

iptables --flush
iptables -t nat -A POSTROUTING -o eth0 -j MASQUERADE
iptables -A FORWARD -i eth1 -m state --state NEW,INVALID -j DROP
echo "Success"