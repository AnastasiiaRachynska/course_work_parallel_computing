FROM ubuntu:22.04 AS builder

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential cmake git ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .

RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
    && cmake --build build --config Release --target course_work_parallel_computing search time_threads
FROM ubuntu:22.04
WORKDIR /app

COPY --from=builder /app/build/course_work_parallel_computing /app/main
COPY --from=builder /app/build/search /app/search
COPY --from=builder /app/build/time_threads /app/time_threads

EXPOSE 8080 8081
CMD ["./main"]