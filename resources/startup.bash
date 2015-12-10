#!/bin/bash
service tor start
cd /root/server
nohup ./cserver 2>&1 > /var/log/cserver.log &
tail -f /var/log/tor/log /var/log/cserver.log
