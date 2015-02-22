#!/bin/bash
service tor start
cd /root/server
./cserver >> /var/log/cserver.log
