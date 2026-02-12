# User Documentation

This guide explains how to use and administer the Inception infrastructure. It is written for end users and administrators who need to access the services, manage credentials, and verify that everything is running correctly.

---

## What Services Are Provided

The Inception stack deploys a complete web infrastructure composed of the following services:

| Service | What It Does | How to Access |
|---------|-------------|---------------|
| **Nginx** | Reverse proxy; the sole HTTPS entry point for all web traffic | Port 443 on the host |
| **WordPress** | Blog and content management system (CMS) | `https://viceda-s.42.fr` |
| **MariaDB** | Relational database storing all WordPress content | Internal only (port 3306) |
| **Redis** | In-memory cache that speeds up WordPress | Internal only (port 6379) |
| **FTP** | File transfer server for uploading/downloading WordPress files | `ftp://viceda-s.42.fr:21` |
| **Adminer** | Lightweight web UI for managing the database | `https://viceda-s.42.fr/adminer` |
| **Static Site** | A simple HTML/CSS/JS portfolio page | `https://viceda-s.42.fr/portfolio` |
| **File Browser** | Web-based file manager for browsing WordPress volumes | `http://localhost:8080` |

Only Nginx (port 443) and the FTP server (port 21) are exposed to the host. All other services communicate internally over the Docker network.

---

## Starting and Stopping the Project

### Start the Infrastructure

From the project root directory:

```bash
make
```

This builds all Docker images (if not already built) and starts every container in the background. The first run takes 2–5 minutes; subsequent starts are much faster.

### Stop the Infrastructure

```bash
make down
```

This stops and removes all containers. **Your data is preserved** — volumes are not deleted.

### Pause and Resume

To stop containers without removing them:

```bash
make stop
```

To resume stopped containers:

```bash
make start
```

### Full Reset

To remove everything (containers, images, volumes, and stored data) and start fresh:

```bash
make fclean
make
```

> **Warning:** `make fclean` deletes all persistent data (database content, WordPress files). Use this only if you want a clean slate.

---

## Accessing the Website and Administration Panel

### WordPress Website

**URL:** `https://viceda-s.42.fr`

When you visit this URL, your browser will show a security warning because the TLS certificate is self-signed. This is normal for a development environment.

- **Chrome / Edge:** Click "Advanced" then "Proceed to viceda-s.42.fr (unsafe)"
- **Firefox:** Click "Advanced" then "Accept the Risk and Continue"
- **Safari:** Click "Show Details" then "visit this website"

### WordPress Admin Panel

**URL:** `https://viceda-s.42.fr/wp-admin`

Log in with the administrator credentials configured during setup:

- **Username:** The value of `WP_ADMIN_USER` in `srcs/.env`
- **Password:** The value of `WP_ADMIN_PASSWORD` in `srcs/.env`

From the admin panel you can:

- Write, edit, and publish posts and pages
- Manage users and their roles
- Install and configure themes and plugins
- Adjust site settings (title, tagline, permalinks, etc.)

### Adminer (Database Management)

**URL:** `https://viceda-s.42.fr/adminer`

Fill in the login form:

| Field | Value |
|-------|-------|
| System | MySQL |
| Server | `mariadb` |
| Username | Value of `MYSQL_USER` in `srcs/.env` |
| Password | Value of `MYSQL_PASSWORD` in `srcs/.env` (or contents of `secrets/db_password.txt`) |
| Database | `wordpress` |

> **Caution:** Adminer gives direct access to the database. Modifying or deleting rows can break the WordPress installation.

### Static Site (Portfolio)

**URL:** `https://viceda-s.42.fr/portfolio`

A static HTML/CSS/JS page served via Nginx. To edit it, modify the files under `srcs/requirements/bonus/static-site/site/` and restart the container:

```bash
sudo docker-compose -f srcs/docker-compose.yml restart static-site
```

### File Browser

**URL:** `http://localhost:8080`

A web-based file manager that provides read-only access to the WordPress volume. Log in with:

- **Username:** Value of `FB_ADMIN_USER` in `srcs/.env`
- **Password:** Value of `FB_ADMIN_PASSWORD` in `srcs/.env`

### FTP Server

**Access:** `ftp://viceda-s.42.fr:21`

- **Username:** Value of `FTP_USER` in `srcs/.env`
- **Password:** Value of `FTP_PASSWORD` in `srcs/.env`

Modern browsers no longer support FTP. Use command-line tools instead:

```bash
# List files
curl ftp://viceda-s.42.fr:21/ --user <FTP_USER>:<FTP_PASSWORD>

# Download a file
curl ftp://viceda-s.42.fr:21/filename --user <FTP_USER>:<FTP_PASSWORD> -o filename

# Upload a file
curl -T localfile.txt ftp://viceda-s.42.fr:21/ --user <FTP_USER>:<FTP_PASSWORD>
```

Or use an FTP client such as FileZilla or `lftp`.

---

## Locating and Managing Credentials

### Where Credentials Are Stored

| File | Contents |
|------|----------|
| `secrets/db_root_password.txt` | MariaDB root password |
| `secrets/db_password.txt` | WordPress database user password |
| `secrets/credentials.txt` | Application-level credentials (format: `user:password`) |
| `srcs/.env` | All environment variables including usernames, non-secret config, and some passwords |

### Viewing Credentials

```bash
cat secrets/db_root_password.txt
cat secrets/db_password.txt
cat secrets/credentials.txt
cat srcs/.env
```

### Changing Credentials

Changing credentials after the initial setup requires a full rebuild because the database is initialized with the original values.

1. Stop and clean everything:
   ```bash
   make fclean
   ```
2. Edit the credential files and/or `srcs/.env` with the new values.
3. Rebuild:
   ```bash
   make
   ```

### Security Notes

- The `secrets/` directory and `srcs/.env` should be listed in `.gitignore` and must **never** be committed to version control.
- Use strong passwords (12+ characters, mixed case, numbers, symbols).
- The self-signed TLS certificate is suitable for development only — not for production.

---

## Checking That Services Are Running Correctly

### Quick Health Check

```bash
docker ps
```

You should see **8 containers** (3 mandatory + 5 bonus), all with `STATUS: Up`:

| Container | Expected Status |
|-----------|----------------|
| mariadb | Up |
| wordpress | Up |
| nginx | Up |
| redis | Up |
| ftp | Up |
| adminer | Up |
| static-site | Up |
| filebrowser | Up |

### View Logs

Real-time logs from all services:

```bash
make logs
```

Logs from a specific service:

```bash
sudo docker-compose -f srcs/docker-compose.yml logs wordpress
sudo docker-compose -f srcs/docker-compose.yml logs mariadb
sudo docker-compose -f srcs/docker-compose.yml logs nginx
```

Press `Ctrl+C` to stop following logs.

### Test HTTPS Connectivity

```bash
curl -k https://viceda-s.42.fr
```

A successful response returns the WordPress HTML page. The `-k` flag skips certificate verification (needed for self-signed certs).

### Check Resource Usage

```bash
docker stats
```

Shows live CPU, memory, and network usage per container. Press `Ctrl+C` to exit.

---

## Troubleshooting

### Services Won't Start

```bash
make logs
```

Look for error messages. Common causes:

- **Port 443 already in use:** Another service is occupying the port. Check with `sudo lsof -i :443`.
- **Docker daemon not running:** Start it with `sudo systemctl start docker`.
- **Missing environment variables:** Ensure `srcs/.env` exists and contains all required values.

### Cannot Access WordPress

1. **Browser shows "connection refused"** — Nginx may not be running. Check `docker ps`.
2. **Blank page or loading indefinitely** — WordPress may still be initializing. Wait 1–2 minutes and try again.
3. **404 error** — Database may not be ready. Check MariaDB logs: `sudo docker-compose -f srcs/docker-compose.yml logs mariadb`.

### Cannot Resolve `viceda-s.42.fr`

Verify your `/etc/hosts` file contains:

```
127.0.0.1 viceda-s.42.fr
```

### Database Errors

MariaDB may take a moment to initialize on first boot. Re-check after 30 seconds. If the problem persists:

```bash
sudo docker-compose -f srcs/docker-compose.yml logs mariadb
```

### High Disk Usage

Check how much space volumes are consuming:

```bash
du -sh /home/viceda-s/data/*
```

To reclaim space with a full reset:

```bash
make fclean
make
```

---

## Data Persistence

All persistent data is stored on the host at:

```
/home/viceda-s/data/
├── mariadb/      # Database files
└── wordpress/    # WordPress files (themes, plugins, uploads, wp-config.php)
```

This data survives container stops, restarts, and removals (unless you run `make fclean`).

### Backing Up

```bash
# Full backup
tar -czf ~/inception_backup.tar.gz /home/viceda-s/data/

# Database dump
sudo docker-compose -f srcs/docker-compose.yml exec mariadb \
  mysqldump -uroot -p<root_password> --all-databases > ~/db_backup.sql
```

### Restoring

```bash
# Restore files
tar -xzf ~/inception_backup.tar.gz -C /

# Restore database
sudo docker-compose -f srcs/docker-compose.yml exec mariadb \
  mysql -uroot -p<root_password> < ~/db_backup.sql
```