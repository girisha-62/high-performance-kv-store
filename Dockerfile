FROM ubuntu:24.04 AS build
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential cmake && rm -rf /var/lib/apt/lists/*
WORKDIR /app
COPY . .
RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=OFF && \
    cmake --build build -j2

FROM ubuntu:24.04
RUN apt-get update && apt-get install -y --no-install-recommends libstdc++6 && rm -rf /var/lib/apt/lists/*
WORKDIR /app
COPY --from=build /app/build/kv_server /app/kv_server
RUN mkdir -p /app/data
EXPOSE 6379
CMD ["/app/kv_server", "6379", "4"]
