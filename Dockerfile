FROM ubuntu
MAINTAINER Charlie

RUN echo "deb http://deb.torproject.org/torproject.org trusty main" > /etc/apt/sources.list.d/tor.list
RUN echo "deb-src http://deb.torproject.org/torproject.org trusty main" >> /etc/apt/sources.list.d/tor.list
RUN gpg --keyserver keys.gnupg.net --recv 886DDD89 && gpg --export A3C4F0F979CAA22CDBA8F512EE8CBC9E886DDD89 | sudo apt-key add -
RUN apt-get update && apt-get -y install tor deb.torproject.org-keyring
ADD ./server/ /root/server
COPY ./startup.bash /root/startup.bash
RUN chmod +x /root/startup.bash
COPY ./tor /etc/tor/torrc
ADD ./hidden_service/ /root/hidden_service/
RUN su -c "timeout 10s tor; exit 0" -s /bin/bash debian-tor
RUN rm -rf /var/lib/tor/hidden_service/*
RUN cp /root/hidden_service/* /var/lib/tor/hidden_service/
RUN chown -R debian-tor:debian-tor /var/lib/tor/hidden_service/
