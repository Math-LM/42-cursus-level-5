FROM debian:bullseye

# Install dependencies
RUN apt-get update && apt-get install -y \
    wget \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# Use www-data user (same as WordPress) for file access compatibility
# www-data user already exists in the base image

# Download and install filebrowser
RUN wget -O /usr/local/bin/filebrowser.tar.gz https://github.com/filebrowser/filebrowser/releases/download/v2.23.0/linux-amd64-filebrowser.tar.gz \
    && tar -xzf /usr/local/bin/filebrowser.tar.gz -C /usr/local/bin/ \
    && rm /usr/local/bin/filebrowser.tar.gz \
    && chmod +x /usr/local/bin/filebrowser

# Create necessary directories
RUN mkdir -p /srv /config /database

# Copy configuration and initialization script
COPY conf/filebrowser.json /config/
COPY tools/setup.sh /usr/local/bin/
RUN chmod +x /usr/local/bin/setup.sh

# Set permissions for www-data user
RUN chown -R www-data:www-data /srv /config /database

# Expose port
EXPOSE 8082

ENTRYPOINT ["/usr/local/bin/setup.sh"]