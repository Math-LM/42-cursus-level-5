#!/bin/bash
set -e

# Initialize database if it doesn't exist
if [ ! -d "/var/lib/mysql/mysql" ]; then
    echo "Initializing MariaDB data directory..."
    mysql_install_db --user=mysql --datadir=/var/lib/mysql
fi

# Check if WordPress database already exists
DB_EXISTS=0
if [ -d "/var/lib/mysql/${MYSQL_DATABASE}" ]; then
    DB_EXISTS=1
    echo "WordPress database already exists, skipping initialization."
fi

# Start MariaDB in background
mysqld --user=mysql --datadir=/var/lib/mysql &
pid="$!"

# Wait for MariaDB to start
echo "Waiting for MariaDB to start..."
for i in {30..0}; do
    if mysqladmin ping --silent; then
        break
    fi
    sleep 1
done

if [ "$i" = 0 ]; then
    echo "MariaDB failed to start"
    exit 1
fi

# Create database and user only on first run
if [ "$DB_EXISTS" = "0" ]; then
    echo "Creating WordPress database and user..."
    mysql << EOF
ALTER USER 'root'@'localhost' IDENTIFIED BY '${MYSQL_ROOT_PASSWORD}';
DELETE FROM mysql.user WHERE User='';
DELETE FROM mysql.user WHERE User='root' AND Host NOT IN ('localhost', '127.0.0.1', '::1');
CREATE DATABASE IF NOT EXISTS \`${MYSQL_DATABASE}\`;
CREATE USER IF NOT EXISTS '${MYSQL_USER}'@'%' IDENTIFIED BY '${MYSQL_PASSWORD}';
GRANT ALL PRIVILEGES ON \`${MYSQL_DATABASE}\`.* TO '${MYSQL_USER}'@'%';
FLUSH PRIVILEGES;
EOF
    echo "MariaDB initialized successfully."
fi

# Keep MariaDB running
wait "$pid"
