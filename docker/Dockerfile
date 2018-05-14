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
	perl \
	protobuf \
	protobuf-c \
	sudo \
	wget \
	yajl

# add user and give user permissions
RUN useradd -m user \
	&& chown user:user /build \
	&& groupadd sudo \
	&& usermod -aG sudo user \
	&& chmod 0640 /etc/sudoers \
	&& echo -en "\n%sudo	ALL=(ALL) NOPASSWD: ALL\n" >> /etc/sudoers \
	&& chmod 0440 /etc/sudoers

# switch to user
USER user
ENV PATH="$PATH:/usr/bin/core_perl"

# install dumb-init
RUN wget -O dumb-init https://github.com/Yelp/dumb-init/releases/download/v1.2.1/dumb-init_1.2.1_amd64 \
	&& sudo mv dumb-init /usr/local/bin/dumb-init \
	&& chmod 0755 /usr/local/bin/dumb-init

# get keys
RUN gpg --recv-keys --keyserver hkp://pgp.mit.edu 1EB2638FF56C0C53

# get aur packages
RUN git clone https://aur.archlinux.org/cower.git \
	&& cd cower \
	&& makepkg -Acs \
	&& yes | sudo pacman -U cower*.tar.xz \
	&& cd ../ \
	&& cower -dd spdlog-git \
	&& cd spdlog-git \
	&& makepkg -Acs \
	&& yes | sudo pacman -U spdlog-git*.tar.xz

WORKDIR /wd
ENTRYPOINT ["/usr/local/bin/dumb-init", "--"]
CMD ["/wd/ssyncd"]
