FROM debian:bullseye

RUN apt-get update && apt-get install -y \
    php7.4-fpm \
    php7.4-mysql \
    wget \
    && rm -rf /var/lib/apt/lists/*

# Create directory for Adminer
RUN mkdir -p /var/www/html /run/php && \
    chown -R www-data:www-data /var/www/html

# Copy PHP-FPM configuration
COPY conf/www.conf /etc/php/7.4/fpm/pool.d/www.conf

# Copy setup script
COPY tools/setup-adminer.sh /usr/local/bin/setup-adminer.sh
RUN chmod +x /usr/local/bin/setup-adminer.sh

WORKDIR /var/www/html

EXPOSE 8080

ENTRYPOINT ["setup-adminer.sh"]
CMD ["php-fpm7.4", "-F"]
