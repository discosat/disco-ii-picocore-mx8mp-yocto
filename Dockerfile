FROM ubuntu:22.04
RUN rm /bin/sh && ln -s /bin/bash /bin/sh

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    git \
    curl \
    expect \
    python3 \
    python3-distutils \
    gnupg \
    bzip2 \
    chrpath \
    cpio \
    cpp \
    diffstat \
    file \
    g++ \
    gawk \
    gcc \
    lz4 \
    make \
    binutils \
    zstd \
    wget \
    locales \
    sudo \
    gcc-multilib \
    build-essential \
    socat \
    python3-pip \
    python3-pexpect \
    xz-utils \
    debianutils \
    iputils-ping \
    python3-git \
    python3-jinja2 \
    libegl1-mesa \
    libsdl1.2-dev \
    xterm \
    python3-subunit \
    mesa-common-dev \
    libarchive-zip-perl \
    ca-certificates  \
 && pip3 install pylint \
 && ln -s /usr/bin/python3 /usr/bin/python \
 && locale-gen en_US.UTF-8

ENV LANG en_US.UTF-8
ENV LANGUAGE en_US:en
ENV LC_ALL en_US.UTF-8

# Create the yocto user before making directories
RUN useradd -ms /bin/bash yocto
RUN echo 'yocto ALL=(ALL) NOPASSWD:ALL' >> /etc/sudoers

# Ensure essential directories exist, now owned by yocto
RUN mkdir -p /build /deploy /custom 
RUN chown -R yocto:yocto /build /deploy /custom

# Copy necessary directories from the host (if they contain required files)
COPY --chown=yocto:yocto custom/conf /custom/conf
COPY --chown=yocto:yocto custom/layers /custom/layers
COPY --chown=yocto:yocto custom/patch /custom/patch

#  Switch to yocto user after preparing the system
USER yocto

WORKDIR /prep-build

#  Copy setup files to /prep-build
COPY --chown=yocto:yocto setup-yocto fs-release-manifest.xml ./

RUN echo -e "\n\ny\n" | ./setup-yocto disco-ii-yocto

WORKDIR /prep-build/disco-ii-yocto/yocto-fus

# Expect script to accept EULA
RUN echo -e '#!/usr/bin/expect\n\
set env(DISTRO) "fus-imx-wayland"\n\
set env(MACHINE) "fsimx8mp"\n\
spawn /bin/bash -c "source /prep-build/disco-ii-yocto/yocto-fus/fus-setup-release.sh"\n\
expect "Do you accept the EULA you just read? (y/n)"\n\
send "y\r"\n\
expect eof' > setup.exp
RUN chmod +x setup.exp && ./setup.exp

#  Copy start.sh (Already belongs to yocto due to --chown)
COPY --chown=yocto:yocto custom/start.sh /custom/start.sh
RUN chmod +x /custom/start.sh

# Set start.sh as entrypoint
CMD /custom/start.sh

