#!/bin/bash

FTP_DIR="/home/$FTP_USER/ftp"

# Create FTP user if it doesn't exist
if ! id "$FTP_USER" &>/dev/null; then
    useradd -m -d "$FTP_DIR" -s /bin/bash "$FTP_USER"
    usermod -a -G www-data "$FTP_USER"
fi

# Set FTP user password from environment variable
echo "$FTP_USER:$FTP_PASSWORD" | chpasswd

# Create the chroot list file
echo "$FTP_USER" > /etc/vsftpd.chroot_list

# Create user list file
echo "$FTP_USER" > /etc/vsftpd.userlist

# Ensure FTP directory exists and has correct permissions
mkdir -p "$FTP_DIR"
chown -R www-data:www-data "$FTP_DIR"
chmod -R 775 "$FTP_DIR"

# Make sure the user can write to the directory
if [ ! -f "$FTP_DIR/.write_test" ]; then
    touch "$FTP_DIR/.write_test" 2>/dev/null || true
fi

echo "Starting vsftpd..."
exec vsftpd /etc/vsftpd.conf