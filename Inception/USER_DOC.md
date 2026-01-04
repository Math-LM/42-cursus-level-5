# USER_DOC.md - User and Administrator Guide

## Table of Contents
1. [Overview](#overview)
2. [Getting Started](#getting-started)
3. [Accessing Services](#accessing-services)
4. [Managing Credentials](#managing-credentials)
5. [Starting and Stopping](#starting-and-stopping)
6. [Troubleshooting](#troubleshooting)
7. [Data Management](#data-management)

---

## Overview

### What is This?

The Inception project is a complete web infrastructure powered by Docker. It provides:

- **WordPress Blog**: A ready-to-use content management system for blogging and web publishing
- **Database**: A MariaDB server that stores all WordPress content securely
- **Web Server**: An Nginx server with HTTPS encryption protecting all traffic
- **Bonus Tools**: File management (FTP), database admin panel (Adminer), caching (Redis), and a portfolio website

All services run in isolated containers that communicate securely with each other.

### Services at a Glance

| Service | Purpose | Port |
|---------|---------|------|
| Nginx | Web server & reverse proxy | 443 (HTTPS) |
| WordPress | Blog platform with PHP | Port 9000 (internal) |
| MariaDB | Database | Port 3306 (internal) |
| Redis | Cache for WordPress | Port 6379 (internal) |
| FTP | File transfer server | Port 21 + 21000-21010 |
| Adminer | Database UI | Accessible via Nginx |
| Static Site | Portfolio website | Accessible via Nginx |

**Note**: Only Nginx port 443 is exposed externally. All other services communicate internally.

---

## Getting Started

### Prerequisites

- Docker and Docker Compose installed on your system
- At least 1GB of free disk space
- Access to your system's hosts file or IP configuration

### First-Time Setup

#### Step 1: Configure Your Domain

On your development machine, add this line to `/etc/hosts`:

```
127.0.0.1 <your_login>.42.fr
```

Replace `<your_login>` with your actual 42 login (e.g., `27.0.0.1 vicente.42.fr`).

**On Linux/Mac:**
```bash
sudo nano /etc/hosts
# Add the line above, save, and exit
```

**On Windows:**
```
C:\Windows\System32\drivers\etc\hosts
```
(Open as Administrator)

#### Step 2: Start the Infrastructure

```bash
cd ~/Inception
make
```

The first run will take 2-5 minutes as Docker builds and initializes everything.

#### Step 3: Wait for Initialization

Monitor the startup:
```bash
make logs
```

Look for messages like:
- `MariaDB initialized successfully`
- `WordPress setup complete`
- `Nginx started`

Press `Ctrl+C` to exit log view.

#### Step 4: Verify Everything is Running

```bash
docker ps
```

You should see 7 containers (3 mandatory + 4 bonus):
- mariadb
- wordpress
- nginx
- redis
- ftp
- adminer
- static-site

All should show `STATUS: Up`

---

## Accessing Services

### WordPress Website

**URL:** `https://<your_login>.42.fr`

**HTTPS Warning**: You'll see a browser security warning because the certificate is self-signed (not from a trusted authority). This is normal for development. You can:

- **Chrome/Edge**: Click "Advanced" → "Proceed to..."
- **Firefox**: Click "Advanced" → "Accept the Risk"
- **Safari**: Click "Show Details" → "Visit this website"

### WordPress Admin Panel

**URL:** `https://<your_login>.42.fr/wp-admin`

Log in with:
- **Username**: The admin user created during setup (not containing "admin")
- **Password**: Set in your environment or `secrets/credentials.txt`

From here, you can:
- Write and publish posts
- Manage users and permissions
- Install themes and plugins
- Configure site settings

### Database Admin (Adminer)

**URL:** `https://<your_login>.42.fr/adminer`

Adminer allows you to manage your database directly:

1. **Server**: `mariadb`
2. **Username**: `wp_user` (as configured)
3. **Password**: Found in your environment variables or `secrets/db_password.txt`
4. **Database**: `wordpress`

**Be careful** - you can modify or delete data here!

### Portfolio Website (Static Site)

**URL:** `https://<your_login>.42.fr/portfolio`

A simple HTML/CSS showcase. Edit the files at:
```
srcs/requirements/bonus/static-site/site/
```

Changes take effect after container restart:
```bash
docker-compose -f srcs/docker-compose.yml restart static-site
```

### FTP Server

**Access:** `ftp://<your_login>.42.fr:21`

**Credentials:**
- **Username**: `ftpuser`
- **Password**: Found in `secrets/credentials.txt`

**Tools:**
- **Command line**: `ftp <your_login>.42.fr`
- **Windows**: Windows Explorer (File → New → FTP Location)
- **Mac**: Finder → Go → Connect to Server
- **Linux**: FileZilla, Nautilus, or `ftp` command

**What you can do:**
- Upload, download, and edit WordPress files
- Useful for developers managing content

---

## Managing Credentials

### Where Credentials Are Stored

All sensitive information is stored in the `secrets/` folder at the project root:

```
secrets/
├── db_root_password.txt       # MariaDB root password
├── db_password.txt             # WordPress database user password
└── credentials.txt             # User account credentials
```

### Viewing Credentials

To see your WordPress admin password:
```bash
cat secrets/credentials.txt
```

To see the database password:
```bash
cat secrets/db_password.txt
```

### Changing Credentials

**Important**: Changing credentials after the first run requires rebuilding the database.

1. **Stop the infrastructure:**
   ```bash
   make down
   ```

2. **Update credential files:**
   ```bash
   echo "new_password" > secrets/db_password.txt
   ```

3. **Clean volumes to reset the database:**
   ```bash
   make fclean
   ```

4. **Update `srcs/.env` if needed:**
   ```bash
   nano srcs/.env
   # Change MYSQL_PASSWORD, etc.
   ```

5. **Rebuild:**
   ```bash
   make
   ```

### Security Notes

- **Never commit** credential files to Git
- The `secrets/` folder should be in `.gitignore`
- Use strong passwords (at least 12 characters, mix of uppercase, numbers, symbols)
- Change passwords periodically in production environments

---

## Starting and Stopping

### Start Everything

```bash
make
```

Or explicitly:

```bash
make up
```

### Stop Everything (containers keep their data)

```bash
make down
```

### Pause and Resume

Stop without removing containers:
```bash
make stop
```

Resume stopped containers:
```bash
make start
```

### View Logs

Real-time logs from all services:
```bash
make logs
```

Logs from a specific service:
```bash
docker-compose -f srcs/docker-compose.yml logs wordpress
docker-compose -f srcs/docker-compose.yml logs mariadb
docker-compose -f srcs/docker-compose.yml logs nginx
```

Follow specific service logs:
```bash
docker-compose -f srcs/docker-compose.yml logs -f wordpress
```

---

## Troubleshooting

### Services Won't Start

**Check logs:**
```bash
make logs
```

**Common issues:**

1. **Port 443 already in use**
   ```bash
   # Find what's using port 443
   sudo lsof -i :443
   # Stop the conflicting service or change Nginx port in docker-compose.yml
   ```

2. **DNS resolution issues**
   - Verify `/etc/hosts` entry is correct
   - Try using IP address instead: `https://127.0.0.1`

3. **Docker daemon not running**
   ```bash
   # On Linux
   sudo systemctl start docker
   # On Mac/Windows
   # Restart Docker Desktop
   ```

### Can't Access WordPress

1. **Certificate warning persists** - This is normal for self-signed certs
2. **Connection refused** - Nginx might not be running. Check: `docker ps`
3. **Blank page** - Wait a bit longer (initialization can take 2-5 minutes)
4. **404 error** - Check database is initialized: `docker logs mariadb`

### Database Connection Errors

```bash
# Check if MariaDB is running
docker-compose -f srcs/docker-compose.yml ps mariadb

# Test database connection
docker-compose -f srcs/docker-compose.yml exec wordpress \
  mysql -h mariadb -u wp_user -p${MYSQL_PASSWORD} -e "SELECT 1"
```

### FTP Not Working

```bash
# Check FTP container
docker-compose -f srcs/docker-compose.yml ps ftp

# Check logs
docker-compose -f srcs/docker-compose.yml logs ftp

# Verify ports are open (if behind firewall)
netstat -tlnp | grep 21
```

### High Disk Usage

Volumes might be consuming space:

```bash
# Check volume sizes
docker volume ls

# See which volume is large
du -sh /home/$(whoami)/data/*

# Clean up old WordPress files if needed
make fclean
```

---

## Data Management

### Where Is My Data?

All persistent data is stored on your host machine:

```
/home/<your_login>/data/
├── mariadb/          # Database files
└── wordpress/        # WordPress files (themes, plugins, uploads)
```

This means your data survives even if containers crash or are deleted.

### Backing Up Your Data

**Full backup:**
```bash
tar -czf ~/inception_backup.tar.gz /home/$(whoami)/data/
```

**Database dump only:**
```bash
docker-compose -f srcs/docker-compose.yml exec mariadb \
  mysqldump -u root -p${MYSQL_ROOT_PASSWORD} --all-databases > backup.sql
```

**WordPress files backup:**
```bash
tar -czf ~/wordpress_backup.tar.gz /home/$(whoami)/data/wordpress/
```

### Restoring from Backup

**Full restore:**
```bash
tar -xzf ~/inception_backup.tar.gz -C /
make fclean
make
```

**Database restore:**
```bash
docker-compose -f srcs/docker-compose.yml exec mariadb \
  mysql -u root -p${MYSQL_ROOT_PASSWORD} < backup.sql
```

### Resetting Everything

**Complete reset (warning: deletes all data):**
```bash
make fclean
make
```

---

## Common Tasks

### Add a New WordPress User

1. Go to `https://<your_login>.42.fr/wp-admin`
2. Navigate to **Users** → **Add New**
3. Enter username, email, password, and role
4. Click **Add New User**

### Install a WordPress Plugin

1. Go to `https://<your_login>.42.fr/wp-admin`
2. Navigate to **Plugins** → **Add New**
3. Search for a plugin and click **Install Now**
4. Click **Activate**

### Upload Files via FTP

1. Connect to `ftp://<your_login>.42.fr:21`
2. Navigate to `/wordpress/wp-content/uploads/`
3. Upload your files
4. Files will be accessible from your WordPress site

### Check Database Tables

1. Go to `https://<your_login>.42.fr/adminer`
2. Select the `wordpress` database
3. Browse tables to see posts, users, options, etc.

### View Server Resource Usage

```bash
# CPU and memory usage
docker stats

# Disk usage
du -sh /home/$(whoami)/data/

# Network activity
docker-compose -f srcs/docker-compose.yml stats
```

---

## Getting Help

### Check Service Health

```bash
docker-compose -f srcs/docker-compose.yml ps
```

All services should show `Up` status and healthy status if configured.

### View Recent Error Messages

```bash
docker-compose -f srcs/docker-compose.yml logs --tail=50
```

### Reset and Try Again

If things are broken:

```bash
# Stop everything
make down

# Remove all data and containers
make fclean

# Start fresh
make
```

This gives you a clean slate.

---

## Additional Resources

- **WordPress Help**: https://wordpress.org/support/
- **Docker Documentation**: https://docs.docker.com/
- **Nginx Configuration**: https://nginx.org/en/docs/

For more technical information, see **DEV_DOC.md** if you need to modify the infrastructure.