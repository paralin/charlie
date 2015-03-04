#!/bin/bash
service tor start
cd /root/server
./cserver 2>&1 | tee /var/log/cserver.log
