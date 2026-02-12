# Developer Documentation

This document describes how to set up the development environment from scratch, build and run the project, manage containers and volumes, and understand where data is stored and how it persists.

---

## Setting Up the Environment from Scratch

### Prerequisites

- **OS:** Linux (Debian/Ubuntu recommended), macOS with Docker Desktop, or WSL2 on Windows
- **Docker Engine** 20.10+ and **Docker Compose** 2.0+
- **make** utility
- **git** for version control
- A text editor (VS Code, Vim, nano, etc.)

#### Installing Docker on Debian/Ubuntu

```bash
sudo apt-get update
sudo apt-get install -y docker.io docker-compose make curl git
sudo usermod -aG docker $(whoami)
# Log out and log back in for the group change to take effect
```

#### Installing Docker on macOS

```bash
brew install docker docker-compose make
# Start Docker Desktop manually
```

### Cloning and Initial Setup

```bash
git clone <repository-url> Inception
cd Inception
```

Verify the directory structure:

```
Inception/
├── Makefile
├── README.md
├── USER_DOC.md
├── DEV_DOC.md
├── secrets/
└── srcs/
    ├── docker-compose.yml
    └── requirements/
```

### Creating the Data Directories

The Makefile creates these automatically on `make up`, but you can create them manually:

```bash
mkdir -p /home/viceda-s/data/mariadb
mkdir -p /home/viceda-s/data/wordpress
```

### Configuration Files

#### The `.env` File

Create `srcs/.env` with all required environment variables:

```env
# Domain
DOMAIN_NAME=viceda-s.42.fr

# Data directory (used by volume driver_opts in docker-compose)
DATA_DIR=/home/viceda-s/data

# MariaDB
MYSQL_ROOT_PASSWORD=your_root_password
MYSQL_DATABASE=wordpress
MYSQL_USER=wp_user
MYSQL_PASSWORD=your_db_password
MYSQL_HOST=mariadb

# WordPress
WP_TITLE=My WordPress Site
WP_ADMIN_USER=myadmin
WP_ADMIN_PASSWORD=admin_password_here
WP_ADMIN_EMAIL=admin@example.com
WP_USER=regular_user
WP_PASSWORD=user_password_here
WP_EMAIL=user@example.com

# Redis
REDIS_HOST=redis
REDIS_PORT=6379

# FTP
FTP_USER=ftpuser
FTP_PASSWORD=ftp_password_here

# File Browser
FB_ADMIN_USER=admin
FB_ADMIN_PASSWORD=fb_password_here
```

> **Important:** Never commit this file. It should be listed in `.gitignore`.

#### The Secrets Directory

Create the `secrets/` directory with credential files:

```bash
mkdir -p secrets
echo "your_root_password"        > secrets/db_root_password.txt
echo "your_db_password"          > secrets/db_password.txt
echo "myadmin:admin_password"    > secrets/credentials.txt
```

Each file contains a single value (no trailing newline is ideal, but scripts typically handle it).

> **Important:** Never commit these files. Add `secrets/` to `.gitignore`.

### Domain Configuration

Add the project domain to your hosts file:

```bash
sudo sh -c 'echo "127.0.0.1 viceda-s.42.fr" >> /etc/hosts'
```

---

## Building and Launching with the Makefile and Docker Compose

### Makefile Targets

The Makefile wraps Docker Compose commands for convenience:

| Target | Command Executed | Description |
|--------|-----------------|-------------|
| `make` / `make up` | `docker-compose ... up -d --build` | Build images and start containers in detached mode |
| `make down` | `docker-compose ... down` | Stop and remove containers (volumes preserved) |
| `make stop` | `docker-compose ... stop` | Stop containers without removing them |
| `make start` | `docker-compose ... start` | Start previously stopped containers |
| `make logs` | `docker-compose ... logs -f` | Follow real-time logs |
| `make clean` | `docker-compose ... down` + `docker system prune -af` | Remove containers, images, and dangling resources |
| `make fclean` | clean + remove volumes + delete data dirs | Full reset including persistent data |
| `make re` | `fclean` then `up` | Rebuild everything from scratch |

### Building

```bash
make
```

On first run, Docker builds each image from its Dockerfile. Expect 5–15 minutes depending on network speed. Subsequent builds use cached layers and are much faster (1–3 minutes).

### Manual Docker Compose Commands

If you prefer not to use the Makefile:

```bash
# Build all images
sudo docker-compose -f srcs/docker-compose.yml --env-file srcs/.env build

# Start all services
sudo docker-compose -f srcs/docker-compose.yml --env-file srcs/.env up -d

# Stop all services
sudo docker-compose -f srcs/docker-compose.yml down

# View status
sudo docker-compose -f srcs/docker-compose.yml ps

# Logs for a specific service
sudo docker-compose -f srcs/docker-compose.yml logs mariadb
sudo docker-compose -f srcs/docker-compose.yml logs -f wordpress
```

### Build Troubleshooting

| Error | Cause | Fix |
|-------|-------|-----|
| `buildx is not available` | Missing buildx plugin | `sudo apt install docker-buildx-plugin` or use older Compose syntax |
| `no such file or directory` | Missing files in requirements/ | Verify Dockerfile paths and COPY sources exist |
| `port is already allocated` | Another process on port 443 or 21 | `sudo lsof -i :443` to find and stop conflicting process |
| `permission denied` | Docker needs root or group membership | Run with `sudo` or add user to docker group |

---

## Managing Containers and Volumes

### Container Management

#### Listing Running Containers

```bash
docker ps
```

Expected output shows 8 containers: `mariadb`, `wordpress`, `nginx`, `redis`, `ftp`, `adminer`, `static-site`, `filebrowser`.

#### Executing Commands Inside a Container

```bash
# Interactive shell
docker exec -it mariadb bash
docker exec -it wordpress bash

# Run a single command
docker exec wordpress wp plugin list --allow-root
docker exec mariadb mysql -uroot -p<password> -e "SHOW DATABASES;"
docker exec nginx nginx -t
docker exec redis redis-cli ping
```

#### Inspecting a Container

```bash
# Full details
docker inspect mariadb

# Environment variables
docker exec mariadb env

# Running processes
docker exec mariadb ps aux

# Network settings
docker inspect mariadb --format '{{.NetworkSettings.Networks}}'
```

#### Restarting a Single Service

```bash
sudo docker-compose -f srcs/docker-compose.yml restart wordpress
```

#### Viewing Logs

```bash
# All services
make logs

# Specific service, last 100 lines
sudo docker-compose -f srcs/docker-compose.yml logs --tail=100 mariadb

# Follow with timestamps
sudo docker-compose -f srcs/docker-compose.yml logs -ft wordpress
```

#### Resource Usage

```bash
docker stats
docker stats --no-stream --format "table {{.Container}}\t{{.CPUPerc}}\t{{.MemUsage}}"
```

### Volume Management

#### Listing Volumes

```bash
docker volume ls
```

You should see `srcs_wp_db` and `srcs_wp_files`.

#### Inspecting a Volume

```bash
docker volume inspect srcs_wp_db
docker volume inspect srcs_wp_files
```

The `Mountpoint` field shows where Docker stores the volume, and the `Options` field shows the bind mount to the host path.

#### Checking Volume Contents on the Host

```bash
ls -la /home/viceda-s/data/mariadb/
ls -la /home/viceda-s/data/wordpress/
du -sh /home/viceda-s/data/*
```

#### Removing Volumes

```bash
# Remove specific volumes (containers must be stopped first)
docker volume rm srcs_wp_db srcs_wp_files

# Remove all dangling (unused) volumes
docker volume prune
```

> **Warning:** Removing volumes deletes all persistent data. Use `make fclean` which handles this along with cleaning data directories.

### Network Management

#### Listing Networks

```bash
docker network ls
```

The project uses a bridge network named `srcs_inception`.

#### Inspecting the Network

```bash
docker network inspect srcs_inception
```

This shows all connected containers and their IP addresses. Every service resolves to other services by container name (e.g., `mariadb`, `wordpress`, `redis`).

#### Testing Internal Connectivity

```bash
# From WordPress, ping the database
docker exec wordpress ping -c 3 mariadb

# Test database port
docker exec wordpress nc -zv mariadb 3306

# Test Redis
docker exec wordpress nc -zv redis 6379

# DNS resolution
docker exec wordpress nslookup mariadb
```

---

## Where Data Is Stored and How It Persists

### Persistent Data Locations

All persistent data lives on the host filesystem at `/home/viceda-s/data/`:

| Host Path | Container Mount | Contents |
|-----------|----------------|----------|
| `/home/viceda-s/data/mariadb/` | `mariadb:/var/lib/mysql` | MariaDB database files (tables, indexes, logs) |
| `/home/viceda-s/data/wordpress/` | `wordpress:/var/www/html` | WordPress core, themes, plugins, uploads, `wp-config.php` |

The same WordPress volume is also mounted by:
- **Nginx** at `/var/www/html` (read-only) — to serve static files
- **FTP** at `/home/ftpuser/ftp` — for file transfer access
- **File Browser** at `/srv` (read-only) — for web-based browsing

### How Persistence Works

The `docker-compose.yml` defines two named volumes using the `local` driver with bind mount options:

```yaml
volumes:
  wp_db:
    driver: local
    driver_opts:
      type: none
      device: ${DATA_DIR}/mariadb
      o: bind
  wp_files:
    driver: local
    driver_opts:
      type: none
      device: ${DATA_DIR}/wordpress
      o: bind
```

This means:
- Docker creates named volumes (`wp_db`, `wp_files`) that it tracks via `docker volume ls`.
- The actual files are stored at the specified host paths.
- Data survives `docker-compose down`, `docker-compose stop`, and container crashes.
- Data is **only removed** when volumes are explicitly deleted (e.g., `docker volume rm` or `make fclean`).

### Data Lifecycle

| Action | Containers | Volumes | Data |
|--------|-----------|---------|------|
| `make stop` | Stopped | Untouched | Preserved |
| `make start` | Restarted | Untouched | Preserved |
| `make down` | Removed | Untouched | Preserved |
| `make up` | Recreated | Reattached | Preserved |
| `make clean` | Removed + images pruned | Untouched | Preserved |
| `make fclean` | Removed + images pruned | **Removed** | **Deleted** |

### Backup and Restore

#### Backup

```bash
# Full data backup
tar -czf ~/inception_backup.tar.gz /home/viceda-s/data/

# Database dump only
docker exec mariadb mysqldump -uroot -p<root_password> --all-databases > ~/db_backup.sql

# WordPress files only
tar -czf ~/wp_backup.tar.gz /home/viceda-s/data/wordpress/
```

#### Restore

```bash
# Restore files
make fclean
tar -xzf ~/inception_backup.tar.gz -C /
make

# Restore database from dump (with running containers)
docker exec -i mariadb mysql -uroot -p<root_password> < ~/db_backup.sql
```

---

## Service Architecture Reference

### Docker Compose Services

| Service | Base Image | Internal Port | Exposed Port | Volume |
|---------|-----------|---------------|-------------|--------|
| mariadb | debian:bullseye | 3306 | — | wp_db → /var/lib/mysql |
| wordpress | debian:bullseye | 9000 | — | wp_files → /var/www/html |
| nginx | debian:bullseye | 443 | 443 | wp_files → /var/www/html (ro) |
| redis | debian:bullseye | 6379 | — | — |
| ftp | debian:bullseye | 21, 21000-21010 | 21, 21000-21010 | wp_files → /home/ftpuser/ftp |
| adminer | debian:bullseye | 8080 | — | — |
| static-site | debian:bullseye | 8080 | — | — |
| filebrowser | debian:bullseye | 8082 | 8080 (host) → 8082 | wp_files → /srv (ro) |

### Service Dependencies

```
nginx
├── wordpress
│   ├── mariadb
│   └── redis
├── adminer
│   └── mariadb
└── static-site

ftp (independent, shares wp_files volume)
filebrowser (independent, shares wp_files volume)
```

### Key Files per Service

| Service | Dockerfile | Config | Entrypoint/Script |
|---------|-----------|--------|-------------------|
| mariadb | `srcs/requirements/mariadb/Dockerfile` | `conf/50-server.cnf` | `tools/init-db.sh` |
| wordpress | `srcs/requirements/wordpress/Dockerfile` | — | `tools/setup-wordpress.sh` |
| nginx | `srcs/requirements/nginx/Dockerfile` | `conf/nginx.conf`, `conf/default.conf` | — |
| redis | `srcs/requirements/bonus/redis/Dockerfile` | `conf/redis.conf` | — |
| ftp | `srcs/requirements/bonus/ftp/Dockerfile` | `conf/vsftpd.conf` | `tools/setup.sh` |
| adminer | `srcs/requirements/bonus/adminer/Dockerfile` | `conf/www.conf` | `tools/setup-adminer.sh` |
| static-site | `srcs/requirements/bonus/static-site/Dockerfile` | — | — |
| filebrowser | `srcs/requirements/bonus/filebrowser/Dockerfile` | `conf/filebrowser.json` | `tools/setup.sh` |

---

## Quick Reference

### Most Used Commands

```bash
make                  # Build and start everything
make logs             # Follow all logs
make down             # Stop and remove containers
make fclean           # Full cleanup (deletes data)
make re               # Rebuild from scratch

docker ps             # List running containers
docker stats          # Live resource usage
docker exec -it <container> bash   # Shell into a container

sudo docker-compose -f srcs/docker-compose.yml logs <service>      # Service logs
sudo docker-compose -f srcs/docker-compose.yml restart <service>   # Restart one service
```

### Health Checks

```bash
# Nginx config valid?
docker exec nginx nginx -t

# MariaDB accepting connections?
docker exec mariadb mysql -uroot -p<password> -e "SELECT 1;"

# WordPress installed?
docker exec wordpress test -f /var/www/html/wp-config.php && echo "OK" || echo "Missing"

# Redis responding?
docker exec redis redis-cli ping

# TLS certificate info
docker exec nginx openssl x509 -in /etc/nginx/ssl/nginx.crt -text -noout | head -20

# HTTP response
curl -k https://viceda-s.42.fr
```

### File Locations Inside Containers

| Service | Config Path | Data Path |
|---------|------------|-----------|
| Nginx | `/etc/nginx/` | `/var/www/html/` |
| WordPress | `/var/www/html/wp-config.php` | `/var/www/html/` |
| MariaDB | `/etc/mysql/` | `/var/lib/mysql/` |
| Redis | `/etc/redis/` | `/data/` |
| FTP | `/etc/vsftpd.conf` | `/home/ftpuser/ftp/` |