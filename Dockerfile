FROM ubuntu
MAINTAINER Charlie

RUN apt-get update && apt-get -y install tor
ADD ./server/ /root/server
COPY ./startup.bash /root/startup.bash
RUN chmod +x /root/startup.bash
COPY ./tor /etc/tor/torrc
ADD ./hidden_service/ /root/hidden_service/
RUN service tor start && sleep 10
RUN rm -rf /var/lib/tor/hidden_service/*
RUN cp /root/hidden_service/* /var/lib/tor/hidden_service/
RUN chown -R debian-tor:debian-tor /var/lib/tor/hidden_service/
