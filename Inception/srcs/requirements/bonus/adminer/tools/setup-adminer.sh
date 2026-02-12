#!/bin/bash
set -e

# Download Adminer if not present
if [ ! -f "/var/www/html/index.php" ]; then
    echo "Downloading Adminer..."
    wget -q "https://github.com/vrana/adminer/releases/download/v4.8.1/adminer-4.8.1.php" -O /var/www/html/index.php
    chown -R www-data:www-data /var/www/html
    echo "Adminer downloaded successfully."
else
    echo "Adminer already installed."
fi

# Start PHP-FPM
exec "$@"
