# Use the official Alpine Linux image as the base image
FROM --platform="linux/${TARGETARCH}" alpine:latest AS bsdlike-ash

RUN mkdir -p /build/ash

# Install necessary dependencies and build tools
RUN apk update && \
    apk add --no-cache \
    clang \
    llvm \
    cmd:lld \
    libedit-dev \
    libedit-static \
    editline \
    editline-dev

#use editline
ENV ASH_LINE_LIB="editline"

#use clang
ENV CC="clang"

# remove unnecessary dependencies and build tools
RUN apk del --no-cache gcc 2>/dev/null ;

# Copy the project files into the container
COPY build-ash.sh /build/build-ash.sh
COPY ash/* /build/ash/
COPY ash/bltin/* /build/ash/bltin/
COPY ash/funcs/* /build/ash/funcs/
# TODO
#COPY LICENSE /build/LICENSE
# copy libbaremusl header(s) into the container
COPY musl/baremusl/baremusl/include/sys/cdefs.h /usr/include/sys/cdefs.h
# copy SignalWright header(s) into the container
COPY musl/SignalWright/SignalWright/include/SignalWright.h /usr/include/SignalWright.h

# Set the working directory inside the container
WORKDIR /build

# Build the project using the provided build script
RUN chmod +x build-ash.sh && \
    ./build-ash.sh

WORKDIR /

# Set the entry point to run the compiled binary
ENTRYPOINT ["/build/obj/ash/sh"]

# set inherited values
LABEL version="1.3"
LABEL org.opencontainers.image.title="BSDLike Ash"
LABEL org.opencontainers.image.description="MuslLike Ash on alpine linux"
LABEL org.opencontainers.image.vendor="individual"
LABEL org.opencontainers.image.licenses="Apache-2.0 AND MIT"
