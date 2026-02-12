#!/bin/bash
set -e

if [ ! -f "/var/www/html/wp-config.php" ]; then
    echo "Setting up WordPress..."
    
    # Download WordPress CLI
    curl -O https://raw.githubusercontent.com/wp-cli/builds/gh-pages/phar/wp-cli.phar
    chmod +x wp-cli.phar
    mv wp-cli.phar /usr/local/bin/wp
    
    # Download WordPress core
    wp core download --allow-root
    
    # Wait for database
    echo "Waiting for database..."
    until mysql -h"${MYSQL_HOST}" -u"${MYSQL_USER}" -p"${MYSQL_PASSWORD}" -e "SELECT 1" &>/dev/null; do
        sleep 2
    done
    
    # Create wp-config.php
    wp config create \
        --dbname="${MYSQL_DATABASE}" \
        --dbuser="${MYSQL_USER}" \
        --dbpass="${MYSQL_PASSWORD}" \
        --dbhost="${MYSQL_HOST}" \
        --allow-root
    
    # Install WordPress
    wp core install \
        --url="https://${DOMAIN_NAME}" \
        --title="${WP_TITLE}" \
        --admin_user="${WP_ADMIN_USER}" \
        --admin_password="${WP_ADMIN_PASSWORD}" \
        --admin_email="${WP_ADMIN_EMAIL}" \
        --skip-email \
        --allow-root
    
    # Create additional user
    wp user create "${WP_USER}" "${WP_EMAIL}" \
        --user_pass="${WP_PASSWORD}" \
        --role=author \
        --allow-root
    
    # Install and configure Redis cache plugin
    wp plugin install redis-cache --activate --allow-root
    wp config set WP_REDIS_HOST "${REDIS_HOST}" --allow-root
    wp config set WP_REDIS_PORT "${REDIS_PORT}" --raw --allow-root
    wp redis enable --allow-root
    
    echo "WordPress setup complete."
fi

exec "$@"

