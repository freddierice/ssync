FROM base/archlinux
MAINTAINER Freddie Rice

# set workdir to build to keep clean
WORKDIR /build

# upgrade and install required things
RUN yes | pacman -Syu
RUN echo -e "y\n\n\n" | pacman -Sy \
	base-devel \
	cmake \
	git \
	libcurl-compat \
	libssh2 \
	wget

# install dumb-init
RUN wget -O /usr/local/bin/dumb-init https://github.com/Yelp/dumb-init/releases/download/v1.2.1/dumb-init_1.2.1_amd64 \
	&& chmod 0755 /usr/local/bin/dumb-init

RUN useradd -m user
USER user
WORKDIR /wd
ENTRYPOINT ["/usr/local/bin/dumb-init", "--"]
CMD ["/code/ssyncd"]
