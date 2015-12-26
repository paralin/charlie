#!/bin/bash
service tor start
tail -f /var/log/tor/log
