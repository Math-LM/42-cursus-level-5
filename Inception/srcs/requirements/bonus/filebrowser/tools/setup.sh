#!/bin/bash

echo "Starting File Browser..."

# Fix permissions for database directory
if [ -f "/database/filebrowser.db" ]; then
    # If database exists but has wrong permissions, remove it to recreate
    if ! touch /database/test_write 2>/dev/null; then
        echo "Database has wrong permissions, removing to recreate..."
        rm -f /database/filebrowser.db 2>/dev/null || true
    else
        rm -f /database/test_write
    fi
fi

# Initialize the database and create admin user if it doesn't exist
if [ ! -f "/database/filebrowser.db" ]; then
    echo "Creating File Browser database and admin user..."
    /usr/local/bin/filebrowser config init --config /config/filebrowser.json
    /usr/local/bin/filebrowser users add "$FB_ADMIN_USER" "$FB_ADMIN_PASSWORD" --perm.admin --config /config/filebrowser.json
    
    echo "File Browser initialized with admin credentials:"
    echo "  Username: $FB_ADMIN_USER"
    echo "  Password: [hidden for security]"
else
    echo "File Browser database already exists"
fi

# Fix all permissions before starting
chown -R www-data:www-data /srv /config /database 2>/dev/null || true

# Start File Browser as www-data user for WordPress file access
echo "File Browser will be available for WordPress file management"
exec su www-data -s /bin/sh -c "/usr/local/bin/filebrowser --config /config/filebrowser.json"