# Multi-stage build for cashtrack.
# Builder produces a static-ish binary; runtime is a tiny image with
# just the binary and the sample CSVs.

FROM ubuntu:24.04 AS builder

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        build-essential \
        cmake \
        ninja-build \
        ca-certificates \
        git && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /src
COPY . .
RUN cmake -S . -B build -G Ninja \
        -DCMAKE_BUILD_TYPE=Release \
        -DCASHTRACK_BUILD_TESTS=OFF && \
    cmake --build build -j

FROM ubuntu:24.04 AS runtime

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        libstdc++6 \
        ca-certificates && \
    rm -rf /var/lib/apt/lists/*

COPY --from=builder /src/build/cashtrack /usr/local/bin/cashtrack
COPY --from=builder /src/samples /samples

WORKDIR /data
ENTRYPOINT ["/usr/local/bin/cashtrack"]
CMD ["--help"]
