FROM debian:buster
RUN apt-get update
RUN apt-get -y install gcc make libreadline-dev
RUN mkdir /home/tsh-testing
COPY code/ /home/tsh-testing/
