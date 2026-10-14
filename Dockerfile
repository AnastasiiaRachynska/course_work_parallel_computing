FROM  ubuntu:22.04 AS builder

RUN apt-get update && apt-get install -y\
     build-essential cmake git && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .
RUN mkdir build && cd build && cmake .. && make

FROM  ubuntu:22.04
WORKDIR /app
COPY --from=builder /app/build/course_work_parallel_computing /app/main
COPY --from=builder /app/data /app/data

EXPOSE 8080

CMD ["./main"]