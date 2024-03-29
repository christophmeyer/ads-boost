FROM ubuntu:20.04

ENV DEBIAN_FRONTEND noninteractive

RUN apt-get update \
    && apt-get install -y --no-install-recommends cmake \ 
    build-essential \
    zlib1g-dev \
    librtlsdr-dev \
    libgtest-dev

COPY backend /backend
RUN rm -rf /backend/build && mkdir build
WORKDIR /backend/build

RUN cmake .. && cmake --build . 