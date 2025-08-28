FROM ubuntu:plucky

COPY entrypoint.sh* /entrypoint.sh

WORKDIR /
RUN apt update
RUN --mount=type=cache,id=Unpadded,target=/var/cache/apt \
  apt install -y \
  git \
  software-properties-common \
  wget
RUN add-apt-repository ppa:ubuntu-toolchain-r/test
RUN wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key 2>/dev/null | tee /etc/apt/trusted.gpg.d/apt.llvm.org.asc
RUN echo 'deb http://apt.llvm.org/plucky/ llvm-toolchain-plucky-21 main' > /etc/apt/sources.list.d/llvm.list
RUN git clone https://github.com/include-what-you-use/include-what-you-use

WORKDIR /
RUN apt update
RUN --mount=type=cache,id=Unpadded,target=/var/cache/apt \
  apt install -y \
  ccache \
  clang-21 \
  clangd \
  clang-format-21 \
  clang-tidy-21 \
  cmake \
  gdb \
  g++-15 \
  libclang-21-dev \
  pipx \
  socat \
  sudo
RUN git config --global --add safe.directory /code
ENV CCACHE_DIR=/code/.ccache

WORKDIR /include-what-you-use
RUN git fetch --all
RUN git show 721024b
RUN git checkout 721024b
RUN --mount=type=cache,id=Unpadded,target=build \
  cmake -Bbuild -DCMAKE_CXX_COMPILER=clang++-21 -DCMAKE_PREFIX_PATH=/usr/lib/llvm-21 \
  && cmake --build build --target install --parallel $(nproc)
RUN ln -s /usr/local/bin/include-what-you-use /usr/bin/iwyu

WORKDIR /
RUN update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-15 0
RUN update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-21 0
RUN update-alternatives --install /usr/bin/clang-format clang-format /usr/bin/clang-format-21 0
RUN update-alternatives --install /usr/bin/clang-tidy clang-tidy /usr/bin/clang-tidy-21 0
RUN echo "ubuntu ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers

USER ubuntu
WORKDIR /home/ubuntu
RUN pipx ensurepath
RUN --mount=type=cache,id=Unpadded,target=/home/ubuntu/.cache/pip,uid=1000,gid=1000 \
  pipx install cmakelang

ENV TERM=xterm-color
