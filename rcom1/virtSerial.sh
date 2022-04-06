#!/bin/sh
echo "Connect receiver end to /dev/ttyS11"
echo "Connect transmitter end to /dev/ttyS10"
sudo socat -d -d PTY,link=/dev/ttyS10,mode=777 PTY,link=/dev/ttyS11,mode=777
