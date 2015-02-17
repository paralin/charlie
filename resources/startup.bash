#!/bin/bash
service tor start
/root/server/cserver >> /var/log/cserver.log
