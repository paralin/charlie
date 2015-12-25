#!/bin/bash
cd /root/server
nohup ./cserver 2>&1 > /var/log/cserver.log &
tail -f /var/log/cserver.log
