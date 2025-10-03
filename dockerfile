# Use the official Alpine Linux image as the base image
FROM --platform="linux/${TARGETARCH}" alpine:latest AS BSDLike-Ash

RUN mkdir -p /build/ash

# Install necessary dependencies and build tools
RUN apk update && \
    apk add --no-cache \
    clang \
    llvm \
    cmd:lld

# remove unnecessary dependencies and build tools
RUN apk del --no-cache \
    gcc

# Copy the project files into the container
COPY build-ash.sh /build/build-ash.sh
COPY ash/* /build/ash/
# TODO
#COPY LICENSE /build/LICENSE

# Set the working directory inside the container
WORKDIR /build

# Build the project using the provided build script
RUN chmod +x build-CinderBool.sh && \
    sh ./build-ash.sh

WORKDIR /

# Set the entry point to run the compiled binary
ENTRYPOINT ["/build/obj/ash/sh"]

# set inherited values
LABEL version="1.0"
LABEL org.opencontainers.image.title="BSDLike Ash"
LABEL org.opencontainers.image.description="BSDLike Ash on alpine linux"
LABEL org.opencontainers.image.vendor="individual"
LABEL org.opencontainers.image.licenses="0BSD"

