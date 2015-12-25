#!/bin/bash
service tor start
service rinetd start
tail -f /var/log/tor/log /var/log/rinetd.log
