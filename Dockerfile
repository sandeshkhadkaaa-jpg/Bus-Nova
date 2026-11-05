FROM gcc:14 AS builder
WORKDIR /build
COPY server.c .
RUN gcc -O2 -Wall -Wextra server.c -o busnova

FROM debian:bookworm-slim
WORKDIR /app
COPY --from=builder /build/busnova ./busnova
COPY index.html admin.html ./
ENV PORT=10000
EXPOSE 10000
CMD ["./busnova"]
