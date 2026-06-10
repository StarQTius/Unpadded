FROM alpine:3.24.0

COPY entrypoint.sh* /entrypoint.sh

WORKDIR /
RUN --mount=type=cache,id=Unpadded,target=/var/cache/apk \
  apk add --no-cache \
  git \
  wget
RUN git clone https://github.com/include-what-you-use/include-what-you-use
RUN git clone https://github.com/aras-p/ClangBuildAnalyzer

WORKDIR /
RUN --mount=type=cache,id=Unpadded,target=/var/cache/apk \
  apk add --no-cache \
  bash \
  bloaty \
  build-base \
  ccache \
  clang21 \
  clang21-dev \
  clang21-extra-tools \
  clang21-static \
  cmake \
  colordiff \
  curl-dev \
  gdb \
  g++ \
  libxml2-dev \
  linux-headers \
  llvm21-dev \
  llvm21-gtest \
  llvm21-libs \
  llvm21-static \
  pipx \
  socat \
  sudo
RUN git config --global --add safe.directory /code
ENV CCACHE_DIR=/code/.ccache

WORKDIR /include-what-you-use
RUN git fetch --all
RUN git show 721024b
RUN git checkout 721024b
RUN --mount=type=cache,id=Unpadded_iwyu,target=/include-what-you-use/build \
  cmake -Bbuild -DCMAKE_CXX_COMPILER=clang++-21 -DCMAKE_PREFIX_PATH=/usr/lib/llvm-21 \
  && cmake --build build --target install --parallel $(nproc)
RUN ln -s /usr/local/bin/include-what-you-use /usr/bin/iwyu

WORKDIR /ClangBuildAnalyzer
RUN git checkout 55447756ff8af2f87e4a315f2ba637b9380363ea
RUN --mount=type=cache,id=Unpadded_ClangBuildAnalyzer,target=/ClangBuildAnalyzer/build \
  cmake -Bbuild -DCMAKE_CXX_COMPILER=clang++-21 \
  && make install --directory build --jobs $(nproc) --ignore-errors \
  && cmake --build build --target install
RUN ln -s /usr/local/bin/ClangBuildAnalyzer /usr/bin/ClangBuildAnalyzer

WORKDIR /
RUN ln -sf /usr/bin/clang++-21 /usr/bin/clang++
RUN ln -sf /usr/lib/llvm21/bin/clang-format /usr/bin/clang-format
RUN ln -sf /usr/lib/llvm21/bin/clang-format /usr/bin/clang-format-21
RUN ln -sf /usr/lib/llvm21/bin/clang-tidy /usr/bin/clang-tidy
RUN ln -sf /usr/lib/llvm21/bin/clang-tidy /usr/bin/clang-tidy-21
RUN adduser -D -u 1000 alpine
RUN echo "alpine ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers

USER alpine
WORKDIR /home/alpine
RUN pipx ensurepath
RUN --mount=type=cache,id=Unpadded,target=/home/alpine/.cache/pip,uid=1000,gid=1000 \
  pipx install cmakelang

ENV PS1='\[\033[01;32m\]\u@\h\[\033[01;34m\] \w\$\[\033[00m\] '
