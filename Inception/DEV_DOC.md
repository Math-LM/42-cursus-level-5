# DEV_DOC.md - Developer Documentation

## Table of Contents
1. [Project Setup](#project-setup)
2. [Environment Configuration](#environment-configuration)
3. [Building and Launching](#building-and-launching)
4. [Docker Compose Architecture](#docker-compose-architecture)
5. [Service Details](#service-details)
6. [Dockerfile Best Practices](#dockerfile-best-practices)
7. [Volume Management](#volume-management)
8. [Network Management](#network-management)
9. [Common Development Tasks](#common-development-tasks)
10. [Debugging and Testing](#debugging-and-testing)
11. [Project Structure](#project-structure)

---

## Project Setup

### Prerequisites

- Linux/Mac/WSL2 system (Docker Desktop on Mac/Windows with WSL2)
- Docker 20.10+ and Docker Compose 2.0+
- `make` utility
- Text editor (VS Code, Vim, nano, etc.)
- Git for version control

### Initial Clone and Setup

```bash
# Clone the repository
git clone <repo-url> Inception
cd Inception

# Verify structure
ls -la
# Should show: Makefile, README.md, USER_DOC.md, DEV_DOC.md, secrets/, srcs/

# Create necessary directories
mkdir -p secrets
mkdir -p /home/$(whoami)/data/mariadb
mkdir -p /home/$(whoami)/data/wordpress

# Set correct permissions
chmod 755 /home/$(whoami)/data/mariadb
chmod 755 /home/$(whoami)/data/wordpress
```

### Dependency Installation

**On Debian/Ubuntu:**
```bash
sudo apt-get update
sudo apt-get install -y docker.io docker-compose make curl
sudo usermod -aG docker $(whoami)
```

**On Alpine/Arch:**
```bash
sudo apk add docker docker-compose make curl
sudo rc-service docker start
```

**On macOS (with Homebrew):**
```bash
brew install docker docker-compose make
# Start Docker Desktop manually
```

---

## Environment Configuration

### The .env File

Located at `srcs/.env`, this file stores all non-sensitive configuration:

```env
# Domain configuration
DOMAIN_NAME=<your_login>.42.fr

# Database settings
MYSQL_DATABASE=wordpress
MYSQL_USER=wp_user
MYSQL_PASSWORD=your_db_password_here
MYSQL_ROOT_PASSWORD=your_root_password_here
MYSQL_HOST=mariadb

# WordPress settings
WP_TITLE="My WordPress Site"
WP_ADMIN_USER=my_admin_user
WP_ADMIN_PASSWORD=admin_password_here
WP_ADMIN_EMAIL=admin@example.com
WP_USER=regular_user
WP_USER_EMAIL=user@example.com
WP_USER_PASSWORD=user_password_here

# Redis configuration
REDIS_HOST=redis
REDIS_PORT=6379

# FTP configuration
FTP_USER=ftpuser
FTP_PASSWORD=ftp_password_here
```

**Important Notes:**
- Never commit this file with real passwords
- Use `.gitignore` to exclude it from version control
- Passwords should be at least 12 characters, with mixed case and symbols

### Secrets Management

The `secrets/` folder stores sensitive data referenced by Docker:

```
secrets/
├── db_root_password.txt    # One line: password only
├── db_password.txt         # One line: password only
└── credentials.txt         # Format: username:password (one per line)
```

**Create these files:**
```bash
echo "secure_root_password_123" > secrets/db_root_password.txt
echo "secure_db_password_456" > secrets/db_password.txt
echo "admin_user:admin_password" >> secrets/credentials.txt
echo "regular_user:user_password" >> secrets/credentials.txt
```

**Never commit these files!** Add to `.gitignore`:
```bash
echo "secrets/" >> .gitignore
echo "srcs/.env" >> .gitignore
```

---

## Building and Launching

### Using the Makefile

```bash
# Build and start everything
make

# Explicit start
make up

# Stop containers
make down

# Stop without removing
make stop

# Start stopped containers
make start

# View logs
make logs

# Clean images/containers
make clean

# Full reset (removes volumes!)
make fclean

# Rebuild from scratch
make re
```

### Manual Docker Compose Commands

If not using Make:

```bash
# Build images
docker-compose -f srcs/docker-compose.yml build

# Start services
docker-compose -f srcs/docker-compose.yml up -d

# Stop services
docker-compose -f srcs/docker-compose.yml down

# View status
docker-compose -f srcs/docker-compose.yml ps

# Check specific service logs
docker-compose -f srcs/docker-compose.yml logs mariadb
docker-compose -f srcs/docker-compose.yml logs wordpress
docker-compose -f srcs/docker-compose.yml logs nginx

# Follow logs in real-time
docker-compose -f srcs/docker-compose.yml logs -f
```

### Build Output Interpretation

**First build (expect 5-15 minutes):**
```
Building mariadb
[+] Building 45.2s (10/10) FINISHED
...
```

**Subsequent builds (expect 1-3 minutes):**
- Only changed services rebuild
- Cached layers speed up the process

**Common build errors:**
```
ERROR: buildx is not available
# Install docker buildx or use older compose syntax

ERROR: no such file or directory
# Check that files exist in requirements/*/tools/ and requirements/*/conf/
```

---

## Docker Compose Architecture

### Service Definition Structure

```yaml
services:
  mariadb:
    container_name: mariadb
    build:
      context: ./requirements/mariadb
      dockerfile: Dockerfile
    image: mariadb:inception
    volumes:
      - wp_db:/var/lib/mysql
    networks:
      - inception_net
    environment:
      MYSQL_ROOT_PASSWORD: ${MYSQL_ROOT_PASSWORD}
      MYSQL_DATABASE: ${MYSQL_DATABASE}
      # ... more env vars
    restart: always
    depends_on:
      - other_service  # Optional: explicit dependency
```

**Key fields:**
- `container_name`: Name of running container (must match service name)
- `build`: Path to Dockerfile and build context
- `image`: Tag for built image (should include `:inception`)
- `volumes`: Mount points and volume references
- `networks`: Which user-defined network to join
- `environment`: Environment variables for the container
- `restart`: auto-restart policy (`always`, `on-failure`, `no`)
- `depends_on`: Service ordering (not guaranteed ordering, use health checks)

### Volume Configuration

```yaml
volumes:
  wp_db:
    driver: local
    driver_opts:
      type: none
      o: bind
      device: ${HOME}/data/mariadb

  wp_files:
    driver: local
    driver_opts:
      type: none
      o: bind
      device: ${HOME}/data/wordpress
```

This creates a local volume with a bind mount to the host directory.

### Network Configuration

```yaml
networks:
  inception_net:
    driver: bridge
```

A user-defined bridge network enables:
- Service-to-service DNS resolution (e.g., `mariadb` resolves to the MariaDB container's IP)
- Automatic service discovery
- Complete isolation from the host network

---

## Service Details

### MariaDB Service

**Purpose**: Store WordPress data persistently

**Dockerfile location**: `srcs/requirements/mariadb/Dockerfile`

**Key files**:
- `conf/50-server.cnf`: MariaDB configuration
- `tools/init-db.sh`: Initialization script

**Environment variables consumed**:
- `MYSQL_ROOT_PASSWORD`: Root user password
- `MYSQL_DATABASE`: Initial database name
- `MYSQL_USER`: Database user
- `MYSQL_PASSWORD`: Database user password

**What happens on first run**:
1. Docker builds the image
2. `init-db.sh` initializes the data directory
3. MariaDB starts and listens on port 3306
4. WordPress database and user are created

**Testing**:
```bash
# Connect to MariaDB
docker exec -it mariadb mysql -uroot -p${MYSQL_ROOT_PASSWORD}

# List databases
mysql> SHOW DATABASES;

# Connect to WordPress database
mysql> USE wordpress;
mysql> SHOW TABLES;
```

### WordPress Service

**Purpose**: Run PHP application and serve dynamic content

**Dockerfile location**: `srcs/requirements/wordpress/Dockerfile`

**Key dependencies**:
- PHP 7.4 with FPM
- MySQL client for database connectivity
- curl, wget for downloads
- WordPress CLI for setup

**What happens on first run**:
1. Docker builds the image with PHP-FPM
2. `setup-wordpress.sh` waits for MariaDB to be ready
3. Downloads WordPress core using WP-CLI
4. Generates `wp-config.php` with database credentials
5. Installs WordPress core
6. Creates admin and regular user
7. Configures Redis cache plugin
8. PHP-FPM starts listening on port 9000

**PHP-FPM Configuration**:
```bash
# Located in Dockerfile
sed -i 's/listen = \/run\/php\/php7.4-fpm.sock/listen = 9000/g' /etc/php/7.4/fpm/pool.d/www.conf
```

Changes PHP-FPM from Unix socket to TCP listening (for Nginx proxy_pass).

**Environment variables consumed**:
- Database credentials (MYSQL_*)
- WordPress setup (WP_*)
- Domain name (DOMAIN_NAME)

**Testing**:
```bash
# Check if PHP-FPM is listening
docker exec wordpress netstat -tlnp | grep 9000

# Check if WordPress files exist
docker exec wordpress ls -la /var/www/html/

# Test PHP execution
docker exec wordpress php -r "phpinfo();"
```

### Nginx Service

**Purpose**: Serve static files, reverse proxy to PHP-FPM, handle HTTPS

**Dockerfile location**: `srcs/requirements/nginx/Dockerfile`

**TLS Certificate Generation**:
```bash
# Self-signed certificate (in Dockerfile)
openssl req -x509 -nodes -days 365 -newkey rsa:2048 \
  -keyout /etc/nginx/ssl/nginx.key \
  -out /etc/nginx/ssl/nginx.crt \
  -subj "/C=LU/ST=Luxembourg/L=Luxembourg/O=42/OU=42/CN=<your_login>.42.fr"
```

**Key configuration** (`conf/default.conf`):
```nginx
server {
    listen 443 ssl http2;
    server_name <your_login>.42.fr;
    root /var/www/html;

    ssl_certificate /etc/nginx/ssl/nginx.crt;
    ssl_certificate_key /etc/nginx/ssl/nginx.key;
    ssl_protocols TLSv1.2 TLSv1.3;

    location ~ \.php$ {
        fastcgi_pass wordpress:9000;
        fastcgi_param SCRIPT_FILENAME $document_root$fastcgi_script_name;
        # ... fastcgi_params
    }
}
```

**What happens**:
1. Nginx starts on port 443 (HTTPS)
2. Listens for requests to `<your_login>.42.fr`
3. Static files (JS, CSS, images) served directly
4. PHP files forwarded to WordPress PHP-FPM on port 9000
5. Reverse proxy for bonus services (/adminer, /portfolio)

**Testing**:
```bash
# Check certificate
docker exec nginx openssl x509 -in /etc/nginx/ssl/nginx.crt -text -noout

# Test configuration
docker exec nginx nginx -t

# Check ports
docker exec nginx netstat -tlnp
```

### Bonus Services

#### Redis
```bash
# Location: srcs/requirements/bonus/redis/

# Test connection
docker exec redis redis-cli ping
# Should return: PONG
```

#### FTP
```bash
# Location: srcs/requirements/bonus/ftp/

# Create user on first run (in setup-ftp.sh)
# Available at port 21 and 21000-21010 (passive mode)

# Test from host
ftp localhost
# Enter credentials from FTP_USER / FTP_PASSWORD
```

#### Adminer
```bash
# Location: srcs/requirements/bonus/adminer/

# Access at: https://<your_login>.42.fr/adminer
# Requires reverse proxy configuration in Nginx
# Runs PHP on port 8080 internally
```

#### Static Site
```bash
# Location: srcs/requirements/bonus/static-site/

# Serves HTML/CSS from Python HTTP server
# Access at: https://<your_login>.42.fr/portfolio
# Edit: srcs/requirements/bonus/static-site/site/
```

---

## Dockerfile Best Practices

### Structure Template

```dockerfile
# Stage 1: Base image (specific version, not latest)
FROM debian:bullseye

# Install dependencies
RUN apt-get update && apt-get install -y \
    package1 \
    package2 \
    && rm -rf /var/lib/apt/lists/*

# Create required directories
RUN mkdir -p /var/log/service && \
    chown -R www-data:www-data /var/log/service

# Copy configuration files
COPY conf/config.conf /etc/service/
COPY tools/entrypoint.sh /usr/local/bin/
RUN chmod +x /usr/local/bin/entrypoint.sh

# Expose required ports
EXPOSE 80 443

# Define entrypoint and default command
ENTRYPOINT ["/usr/local/bin/entrypoint.sh"]
CMD ["service", "-foreground"]
```

### Key Principles

**1. No `latest` tag**
```dockerfile
# ❌ WRONG
FROM nginx:latest

# ✅ CORRECT
FROM nginx:1.25-alpine
```

**2. PID 1 Awareness**
```dockerfile
# Use exec form, not shell form
ENTRYPOINT ["/usr/local/bin/entrypoint.sh"]
CMD ["service", "-foreground"]

# Inside entrypoint.sh
#!/bin/bash
exec "$@"  # Replace PID 1 with actual service
```

**3. No Infinite Loops**
```bash
# ❌ WRONG
CMD ["bash", "-c", "while true; do sleep 1; done"]

# ✅ CORRECT
CMD ["service_binary", "-foreground"]  # Service handles signals
```

**4. Clean Layer Caching**
```dockerfile
# Combine RUN commands to reduce layers
RUN apt-get update && apt-get install -y \
    package1 \
    package2 \
    && rm -rf /var/lib/apt/lists/*
```

**5. Use .dockerignore**
```
.git
.gitignore
__pycache__
*.pyc
node_modules
```

---

## Volume Management

### Checking Volume Status

```bash
# List all volumes
docker volume ls

# Inspect a specific volume
docker volume inspect srcs_wp_db

# See physical location
docker volume inspect srcs_wp_db | grep Mountpoint
```

### Volume Operations

```bash
# View volume contents (from host)
ls /home/$(whoami)/data/mariadb/
ls /home/$(whoami)/data/wordpress/

# Check disk usage
du -sh /home/$(whoami)/data/*

# Backup a volume
tar -czf /tmp/wp_backup.tar.gz /home/$(whoami)/data/wordpress/

# Restore from backup
tar -xzf /tmp/wp_backup.tar.gz -C /
```

### Troubleshooting Volume Issues

```bash
# Permission denied errors
ls -la /home/$(whoami)/data/
# Should be rwx for current user

# Fix permissions if needed
sudo chown -R $(whoami):$(whoami) /home/$(whoami)/data/
chmod 755 /home/$(whoami)/data/*

# Volume not mounting
docker-compose -f srcs/docker-compose.yml logs mariadb | grep "volume\|mount"

# Orphaned volumes
docker volume rm $(docker volume ls -f dangling=true -q)
```

---

## Network Management

### Verifying Network Setup

```bash
# List networks
docker network ls

# Inspect the inception network
docker network inspect srcs_inception_net

# See connected containers
docker network inspect srcs_inception_net | grep -A 5 "Containers"
```

### Testing Internal Connectivity

```bash
# Test service-to-service connectivity
docker exec wordpress ping mariadb
# Should resolve and return PONG

# Test port accessibility
docker exec wordpress nc -zv mariadb 3306
# Should succeed (connection refused if service down, but DNS works)

# Check DNS resolution
docker exec wordpress nslookup mariadb
# Should show the container's IP address
```

### Network Troubleshooting

```bash
# Services can't find each other
# Solution: Verify they're on the same network
docker network inspect srcs_inception_net | grep -i "mariadb\|wordpress"

# Can't access from host
# Solution: Check if port is exposed (only 443 for Nginx should be)
docker-compose -f srcs/docker-compose.yml ps
# Look at "PORTS" column

# Port binding issues
# Find what's using a port
sudo lsof -i :443
sudo netstat -tlnp | grep 443
```

---

## Common Development Tasks

### Adding a New Service

1. **Create directory structure:**
   ```bash
   mkdir -p srcs/requirements/bonus/myservice/{conf,tools}
   ```

2. **Write Dockerfile:**
   ```dockerfile
   FROM debian:bullseye
   # ... follow best practices
   ```

3. **Add to docker-compose.yml:**
   ```yaml
   myservice:
     container_name: myservice
     build:
       context: ./requirements/bonus/myservice
       dockerfile: Dockerfile
     image: myservice:inception
     networks:
       - inception_net
     restart: always
   ```

4. **Rebuild:**
   ```bash
   make fclean
   make
   ```

### Modifying WordPress Configuration

```bash
# Edit WordPress config inside container
docker exec -it wordpress nano /var/www/html/wp-config.php

# Or copy to host, edit, and copy back
docker cp wordpress:/var/www/html/wp-config.php ~/wp-config.php
# Edit locally
docker cp ~/wp-config.php wordpress:/var/www/html/

# Restart service
docker-compose -f srcs/docker-compose.yml restart wordpress
```

### Adding Environment Variables

1. **Add to `srcs/.env`:**
   ```env
   NEW_VAR=new_value
   ```

2. **Reference in `docker-compose.yml`:**
   ```yaml
   environment:
     NEW_VAR: ${NEW_VAR}
   ```

3. **Access in container:**
   ```bash
   docker exec wordpress echo $NEW_VAR
   ```

### Installing WordPress Plugins Programmatically

```bash
# WordPress already has WP-CLI, use it
docker exec wordpress wp plugin install redis-cache --activate --allow-root
```

### Custom MySQL Initialization

Edit `srcs/requirements/mariadb/tools/init-db.sh` to add more:

```bash
mysql -u${MYSQL_USER} -p${MYSQL_PASSWORD} ${MYSQL_DATABASE} << EOF
  CREATE TABLE custom_table (id INT AUTO_INCREMENT PRIMARY KEY, data TEXT);
EOF
```

---

## Debugging and Testing

### Container Inspection

```bash
# Execute command inside container
docker exec container_name command

# Get interactive shell
docker exec -it container_name bash
docker exec -it container_name sh

# View environment variables
docker exec container_name env

# Check running processes
docker exec container_name ps aux
```

### Log Analysis

```bash
# All logs for a service
docker-compose -f srcs/docker-compose.yml logs mariadb

# Follow logs in real-time
docker-compose -f srcs/docker-compose.yml logs -f wordpress

# Last 100 lines
docker-compose -f srcs/docker-compose.yml logs --tail=100

# Timestamp-prefixed logs
docker-compose -f srcs/docker-compose.yml logs -t
```

### Health Checks

```bash
# MariaDB connectivity test
docker exec wordpress mysql -hmariadb -u${MYSQL_USER} -p${MYSQL_PASSWORD} \
  ${MYSQL_DATABASE} -e "SELECT 1;"

# WordPress installation check
docker exec wordpress test -f /var/www/html/wp-config.php && echo "Installed" || echo "Not installed"

# Nginx configuration validation
docker exec nginx nginx -t

# FTP service check
docker exec ftp netstat -tlnp | grep ftp
```

### Performance Testing

```bash
# Monitor resource usage
docker stats

# Memory usage per service
docker stats --no-stream --format "{{.Container}}\t{{.MemUsage}}"

# CPU usage
docker stats --no-stream --format "{{.Container}}\t{{.CPUPerc}}"
```

### HTTP Testing

```bash
# Test Nginx from host
curl -k https://127.0.0.1

# Test with headers
curl -k -v https://127.0.0.1

# Test with custom domain (requires /etc/hosts entry)
curl -k https://<your_login>.42.fr

# Test PHP endpoint
curl -k https://127.0.0.1/index.php
```

---

## Project Structure

```
Inception/
├── Makefile                          # Build automation
├── README.md                         # Project overview
├── USER_DOC.md                       # User guide
├── DEV_DOC.md                        # This file
│
├── secrets/                          # Git-ignored credentials
│   ├── db_root_password.txt
│   ├── db_password.txt
│   └── credentials.txt
│
└── srcs/
    ├── .env                          # Environment variables (git-ignored)
    ├── docker-compose.yml            # Service orchestration
    │
    └── requirements/
        ├── mariadb/
        │   ├── Dockerfile
        │   ├── .dockerignore
        │   ├── conf/
        │   │   └── 50-server.cnf
        │   └── tools/
        │       └── init-db.sh
        │
        ├── wordpress/
        │   ├── Dockerfile
        │   ├── .dockerignore
        │   ├── conf/
        │   └── tools/
        │       └── setup-wordpress.sh
        │
        ├── nginx/
        │   ├── Dockerfile
        │   ├── .dockerignore
        │   ├── conf/
        │   │   ├── nginx.conf
        │   │   └── default.conf
        │   └── tools/
        │
        └── bonus/
            ├── redis/
            │   └── Dockerfile
            ├── ftp/
            │   ├── Dockerfile
            │   ├── conf/
            │   │   └── vsftpd.conf
            │   └── tools/
            │       └── setup-ftp.sh
            ├── adminer/
            │   └── Dockerfile
            └── static-site/
                ├── Dockerfile
                └── site/
                    ├── index.html
                    └── style.css
```

### .gitignore Configuration

```bash
# Required to protect credentials
secrets/
srcs/.env
.DS_Store
__pycache__
*.log
.idea/
.vscode/

# Optional: Docker-related
.dockerignore
docker-compose.override.yml
```

---

## Quick Reference

### Most Common Commands

```bash
# Start project
make

# View logs
make logs

# Stop project
make down

# Full reset
make fclean

# SSH into a service
docker exec -it service_name bash

# View environment
docker exec service_name env | sort

# Execute command
docker exec service_name command arg1 arg2
```

### Health Check Endpoints

```bash
# Test Nginx
curl -k https://<your_login>.42.fr

# Test WordPress
curl -k https://<your_login>.42.fr/wp-admin

# Test database
docker exec wordpress mysql -hmariadb -uroot -p -e "SELECT 1;"

# Test Redis
docker exec redis redis-cli ping
```

### File Locations in Containers

| Service | Config | Data |
|---------|--------|------|
| **Nginx** | `/etc/nginx/` | `/var/www/html/` |
| **WordPress** | `/var/www/html/wp-config.php` | `/var/www/html/` |
| **MariaDB** | `/etc/mysql/` | `/var/lib/mysql/` |
| **Redis** | `/etc/redis/` | `/data/` |
| **FTP** | `/etc/vsftpd.conf` | `/home/ftpuser/` |

---

## References

- **Docker Documentation**: https://docs.docker.com/
- **Docker Compose Spec**: https://github.com/compose-spec/compose-spec/blob/master/spec.md
- **Dockerfile Best Practices**: https://docs.docker.com/develop/develop-images/dockerfile_best-practices/
- **Nginx Documentation**: https://nginx.org/en/docs/
- **MariaDB Documentation**: https://mariadb.com/docs/
- **WordPress Developer**: https://developer.wordpress.org/