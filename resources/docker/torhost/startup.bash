#!/bin/bash
if [ -d /etc/hidden_service ]; then
  rsync -rav /etc/hidden_service /var/lib/tor/hidden_service
  if [ -f /var/lib/tor/hidden_service/privatekey ]; then
    mv /var/lib/tor/hidden_service/privatekey /var/lib/tor/hidden_service/private_key
  fi
fi

chown -R debian-tor:debian-tor /var/lib/tor/hidden_service/
chmod 700 /var/lib/tor/hidden_service

service tor start
tail -f /var/log/tor/log
