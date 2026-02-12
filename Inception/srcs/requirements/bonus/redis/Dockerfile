FROM debian:bullseye

RUN apt-get update && apt-get install -y \
    redis-server \
    && rm -rf /var/lib/apt/lists/*

# Copy custom Redis configuration
COPY conf/redis.conf /etc/redis/redis.conf

# Ensure Redis data directory exists
RUN mkdir -p /var/lib/redis && \
    chown -R redis:redis /var/lib/redis

EXPOSE 6379

USER redis

CMD ["redis-server", "/etc/redis/redis.conf"]
