FROM phusion/baseimage:0.9.18
MAINTAINER Charlie
CMD ["/sbin/my_init"]

RUN mkdir /etc/service/charlie
ADD ./server/ /root/server
COPY ./startup.bash /etc/service/charlie/run
RUN chmod +x /etc/service/charlie/run
