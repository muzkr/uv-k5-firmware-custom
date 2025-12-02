# syntax=docker/dockerfile:1.6

# Parametric Alpine tag. Override at build time with:
#   docker build --build-arg ALPINE_TAG=3.21 -t uvk5 .
# Examples: 3.22, 3.21, 3.19, edge
ARG ALPINE_TAG=3.21
FROM alpine:${ALPINE_TAG}

# Toolchain and utilities needed to build the firmware
RUN apk add --no-cache \
      bash \
      build-base \
      gcc-arm-none-eabi \
      newlib-arm-none-eabi \
      python3 \
      py3-crcmod \
      py3-pip \
      git

# Project workspace
WORKDIR /app

# Copy sources into the image (the script mounts the repo and runs builds)
COPY . .
