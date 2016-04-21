FROM ubuntu:16.04

MAINTAINER Tsuzu(Twitter: @Wp120_3238, Github: cs3238-tsuzu)

RUN apt-get update
RUN apt-get -y upgrade
RUN apt-get -y dist-upgrade
RUN apt-get install -y g++ libboost-dev git cmake vim docker.io
RUN echo 'set tabstop=4\nset autoindent\nset number' > /root/.vimrc

