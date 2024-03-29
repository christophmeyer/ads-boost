# ads-boost

ads-boost is a decoder for ADS-B messages using software defined radio. This repository consists of two components:

- ads-boost: C++ based decoder for reading raw I/Q data from either the SDR, raw I/Q files or decoded message files. The decoded messages can be displayed in the terminal as a table of contacts and be streamed via websocket for use in e.g. the included frontend.

- frontend: react-based webapp to display the contact table in the browser as well as displaying the corresponding aircraft on a map.

![](.media/ads_boost_map.png)

![](.media/ads_boost_contact_table.png)

Features:

- Streaming of current contacts via websocket using [uWebSockets](https://github.com/uNetworking/uWebSockets)

- Configurable tile server for the custom maps, such as OpenStreetMap

- Easy to extend to new SDRs, currently only supports [RTL-SDR](https://www.rtl-sdr.com/about-rtl-sdr/)

- Read/Write raw I/Q data and/or demodulated ADS-B messages

## Getting started

First, you need to clone this repository

```
git clone https://github.com/christophmeyer/ads-boost.git
```

```
cd ads-boost
```

and update the submodules with

```
git submodule update --init --recursive
```

### Build and run decoder from source

First, the dependencies need to be installed, as an example here is how to install them on ubuntu:

```
apt-get update \
    && apt-get install -y --no-install-recommends cmake \
    build-essential \
    zlib1g-dev \
    librtlsdr-dev \
    libgtest-dev
```

Then, to compile the decoder run the following

```
mkdir backend/build
cd backend/build
cmake .. && cmake --build .
```

The build can be verified by running the tests

```
./test_runner
```

To run the decoder and display the current contacts in a table in the terminal, run

```
./ads-boost -c
```

If you want to run the frontend against the decoder, you need to start the decoder with the `-n` to broadcast the contacts via websocket. The port can be specified with `-p`, e.g.

```
./ads-boost -n -p 9001
```

For an overview of all options use

```
./ads-boost --help
```

### Build and run webserver from source

Either run the webserver in development mode with

```
cd frontend/app
yarn install
yarn start
```

or build the app with

```
cd frontend/app
yarn build
```

Then it can be served with e.g. nginx. The tile server (for the map) and ads-boost decoder URLs are specified in `./frontend/app/.env.development.local` (when run with `yarn start`) or in `./frontend/app/.env.production.local` (when build with `yarn build`, also used for docker image, see below).

### Docker setup

For the docker setup, the URLs of the tile server and ads-boost decoder are baked into the frontend image. They can be changed in `./frontend/app/.env.production.local`.

Build the docker images with

```
docker-compose build
```

For correctly decoding ground position messages, the coordinates of the decoder need to be set in `./.env`. Then the services can be started with

```
docker-compose up
```

which runs the both the decoder and webserver which per default gets exposed on port 80.

## Limitations / Todos:

- Only supports for RTL-SDR for now, but should be easy to extend to others
- No error correction (invalid messages get discarded)
- No support for downlink formats other than the ADS-B downlink formats 17 and 18
- Messages with type-code 28 and 31 not implemented yet
- Only tested on Ubuntu 20.04

## Credits/Resources:

- [THE 1090 MEGAHERTZ RIDDLE](https://mode-s.org/decode/) by Junzi Sun is a good resource to get started on ADS-B decoding
- The demodulation algorithm in this repo is somewhat inspired by the classic [dump1090](https://github.com/antirez/dump1090)
