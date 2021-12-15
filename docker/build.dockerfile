FROM ubuntu:latest
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && \
    apt-get -y upgrade && \
    apt-get install -y git wget flex bison gperf python3 python3-pip python3-setuptools cmake ninja-build ccache libffi-dev libssl-dev dfu-util libusb-1.0-0

RUN mkdir -p /esp
WORKDIR /esp
#RUN git clone --recursive https://github.com/espressif/esp-idf.git
RUN git clone https://github.com/espressif/esp-idf.git
WORKDIR /esp/esp-idf
RUN git checkout 220590d599e134d7a5e7f1e683cc4550349ffbf8
RUN git submodule init && git submodule update
RUN mkdir /root/.espressif
RUN ./install.sh esp32
ENV IDF_PATH=/esp/esp-idf/
WORKDIR /src
