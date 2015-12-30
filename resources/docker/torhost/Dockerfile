FROM debian:jessie
MAINTAINER Charlie

RUN echo "deb http://deb.torproject.org/torproject.org jessie main" > /etc/apt/sources.list.d/tor.list && echo "deb-src http://deb.torproject.org/torproject.org jessie main" >> /etc/apt/sources.list.d/tor.list && \
    gpg --keyserver keys.gnupg.net --recv 886DDD89 && gpg --export A3C4F0F979CAA22CDBA8F512EE8CBC9E886DDD89 | apt-key add -
RUN apt-get update && apt-get -y install tor deb.torproject.org-keyring expect

COPY ./startup.bash /root/startup.bash
COPY ./setuptor.expect /root/setuptor.expect
RUN cp /root/setuptor.expect /var/lib/tor/ && chown debian-tor:debian-tor /var/lib/tor/setuptor.expect && chmod +x /root/startup.bash
ADD ./torrc /etc/tor/torrc
RUN expect /var/lib/tor/setuptor.expect && rm -rf /var/lib/tor/hidden_service/ && rm /root/setuptor.expect

VOLUME /etc/hidden_service
CMD /bin/bash /root/startup.bash
