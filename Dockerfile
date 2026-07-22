FROM debian:12

RUN apt update && apt install -y \
    git \
    cmake \
    ninja-build \
    build-essential \
    pkg-config \
    libexiv2-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /build

COPY . .

RUN cmake --version

RUN rm -rf out

RUN cmake --preset release-shared

RUN cmake --build --preset release-shared

CMD ["bash"]