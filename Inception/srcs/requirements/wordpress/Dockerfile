FROM debian:bullseye

RUN apt-get update && apt-get install -y \
    php7.4-fpm \
    php7.4-mysql \
    php7.4-curl \
    php7.4-gd \
    php7.4-xml \
    php7.4-xmlrpc \
    php7.4-zip \
    php7.4-redis \
    wget \
    curl \
    mariadb-client \
    && rm -rf /var/lib/apt/lists/*
    
RUN mkdir -p /run/php && \
    sed -i 's/listen = \/run\/php\/php7.4-fpm.sock/listen = 9000/g' /etc/php/7.4/fpm/pool.d/www.conf
    
COPY tools/setup-wordpress.sh /usr/local/bin/
RUN chmod +x /usr/local/bin/setup-wordpress.sh

WORKDIR /var/www/html

EXPOSE 9000

ENTRYPOINT ["setup-wordpress.sh"]
CMD ["php-fpm7.4", "-F"]
    
    
