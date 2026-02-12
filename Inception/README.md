*This project has been created as part of the 42 curriculum by viceda-s.*

# Inception

## Description

**Inception** is a system administration project from the 42 curriculum that builds a complete, containerized web infrastructure using Docker. The goal is to set up multiple services — each running in its own Docker container — composed and orchestrated via Docker Compose on a personal virtual machine.

The infrastructure includes a WordPress website served by Nginx with TLS encryption, backed by a MariaDB database, along with several bonus services (Redis cache, FTP server, Adminer, a static portfolio site, and a web-based file browser). Every container is built from a custom Dockerfile based on the penultimate stable version of Debian, and no pre-built images from Docker Hub are used (except the base OS).

### Services Overview

| Service | Role |
|---------|------|
| **Nginx** | Reverse proxy and sole HTTPS entry point (port 443, TLS 1.2/1.3) |
| **WordPress + PHP-FPM** | Dynamic web application (listens on port 9000 internally) |
| **MariaDB** | Relational database for WordPress data |
| **Redis** | (Bonus) Object cache for WordPress |
| **FTP** | (Bonus) File transfer access to WordPress files |
| **Adminer** | (Bonus) Lightweight database management UI |
| **Static Site** | (Bonus) A simple HTML/CSS/JS portfolio |
| **File Browser** | (Bonus) Web-based file manager for WordPress volumes |

---

## Instructions

### Prerequisites

- A virtual machine running Linux (Debian-based recommended)
- Docker Engine 20.10+ and Docker Compose 2.0+
- `make` utility
- At least 1 GB of free disk space

### Installation

1. **Clone the repository:**
   ```bash
   git clone <repository-url> Inception
   cd Inception
   ```

2. **Set up secrets** (these files must never be committed):
   ```bash
   mkdir -p secrets
   echo "your_root_password"        > secrets/db_root_password.txt
   echo "your_wp_db_password"       > secrets/db_password.txt
   echo "admin_user:admin_password" > secrets/credentials.txt
   ```

3. **Create the environment file** at `srcs/.env` with your configuration (domain, database credentials, WordPress settings, FTP user, etc.). See [DEV_DOC.md](DEV_DOC.md) for details on all variables.

4. **Configure your domain** by adding an entry to `/etc/hosts`:
   ```bash
   sudo sh -c 'echo "127.0.0.1 viceda-s.42.fr" >> /etc/hosts'
   ```

5. **Build and launch:**
   ```bash
   make
   ```

### Makefile Commands

| Command | Description |
|---------|-------------|
| `make` / `make up` | Build images and start all containers |
| `make down` | Stop and remove containers |
| `make stop` | Stop containers without removing them |
| `make start` | Start previously stopped containers |
| `make logs` | Follow real-time logs from all services |
| `make clean` | Remove containers, images, and prune the system |
| `make fclean` | Full cleanup including volumes and persistent data |
| `make re` | Rebuild everything from scratch |

### Accessing Services

| Service | URL / Access |
|---------|-------------|
| WordPress | `https://viceda-s.42.fr` |
| WordPress Admin | `https://viceda-s.42.fr/wp-admin` |
| Adminer | `https://viceda-s.42.fr/adminer` |
| Static Portfolio | `https://viceda-s.42.fr/portfolio` |
| File Browser | `http://localhost:8080` (credentials in `.env`) |
| FTP | `ftp://viceda-s.42.fr:21` (use CLI — browsers no longer support FTP) |

> **Note:** The TLS certificate is self-signed. Your browser will show a security warning — this is expected in a development environment.

---

## Project Description

### Use of Docker

Docker is the core technology of this project. Instead of running services directly on the host OS or inside full virtual machines, each component is packaged in its own lightweight container. Docker Compose orchestrates the entire stack, defining how containers are built, connected, and persist data.

Every Dockerfile in this project is written from scratch using `debian:bullseye` as the base image. No pre-made application images (e.g., `wordpress:latest` or `nginx:latest` from Docker Hub) are pulled — all installation and configuration is done explicitly in the build process. This ensures full understanding and control over what runs inside each container.

### Sources Included in the Project

The `srcs/` directory contains everything Docker needs:

- **docker-compose.yml** — defines all services, volumes, networks, and their relationships.
- **requirements/mariadb/** — Dockerfile, MariaDB config (`50-server.cnf`), and init script (`init-db.sh`).
- **requirements/wordpress/** — Dockerfile and setup script (`setup-wordpress.sh`) using WP-CLI.
- **requirements/nginx/** — Dockerfile with TLS certificate generation, and Nginx configuration files.
- **requirements/bonus/** — Dockerfiles and configs for Redis, FTP (vsftpd), Adminer, static site, and File Browser.

### Architecture

All containers are connected through a single user-defined bridge network (`inception`). Nginx is the only container with an exposed port (443) to the host. All inter-service communication happens internally over the Docker network using service hostnames as DNS names (e.g., WordPress connects to `mariadb:3306`).

Two named volumes persist data on the host at `/home/viceda-s/data/`:
- **wp_db** → `/home/viceda-s/data/mariadb` (database files)
- **wp_files** → `/home/viceda-s/data/wordpress` (WordPress files, themes, uploads)

### Design Choices

- **TLS 1.2/1.3 only** — enforced in Nginx; older protocols are disabled.
- **No hardcoded passwords** — sensitive data is stored in `secrets/` files and loaded via environment variables or Docker secrets.
- **PID 1 awareness** — entrypoint scripts use `exec` to hand off PID 1 to the actual service process, ensuring proper signal handling.
- **Restart policy `always`** — containers automatically recover from crashes.
- **Read-only volume mounts** — Nginx mounts WordPress files as read-only (`:ro`) since it only serves static assets.

---

## Design Comparisons

### Virtual Machines vs Docker

| Aspect | Virtual Machines | Docker Containers |
|--------|------------------|-------------------|
| **What it virtualizes** | Full hardware + guest OS | Application environment only (shares host kernel) |
| **Image size** | Several GB (entire OS) | Tens to hundreds of MB |
| **Startup time** | Minutes (boot full OS) | Seconds (start a process) |
| **Resource overhead** | High — each VM runs its own kernel, drivers, and OS services | Low — containers share the host kernel |
| **Isolation** | Strong — hardware-level separation via hypervisor | Process-level — uses namespaces and cgroups |
| **Portability** | Less portable; depends on hypervisor | Highly portable; runs anywhere Docker is installed |
| **Best for** | Running different OSes, legacy applications, strong security boundaries | Microservices, CI/CD, reproducible dev environments |

**Why Docker here?** The project runs multiple cooperating services (web server, app server, database, cache). Docker lets us define, build, and orchestrate them as lightweight, reproducible units without the multi-GB overhead of spinning up a VM per service.

### Secrets vs Environment Variables

| Aspect | Docker Secrets | Environment Variables |
|--------|---------------|----------------------|
| **Storage** | Files on disk, mounted into containers at runtime | Stored in `.env` or `docker-compose.yml`, injected into process environment |
| **Visibility** | Not exposed by `docker inspect`, `ps`, or logs | Visible via `docker inspect`, `/proc/<pid>/environ`, and potentially in logs |
| **Mutability** | Immutable once set for a container | Can be read and overridden at runtime |
| **Scope** | Sensitive data: passwords, API keys, tokens | Non-sensitive configuration: hostnames, ports, feature flags |
| **Docker support** | Native in Docker Swarm; in Compose, mounted as files | Native in all Docker modes |

**Our approach:** Database passwords and user credentials are stored as secret files in the `secrets/` directory and referenced by containers at runtime. Non-sensitive settings (domain name, database host, Redis port, etc.) are passed as environment variables via `srcs/.env`.

### Docker Network vs Host Network

| Aspect | User-Defined Bridge Network | Host Network |
|--------|----------------------------|-------------|
| **Isolation** | Containers are isolated from the host and from each other by default | Container shares the host's full network stack |
| **DNS** | Built-in DNS — containers resolve each other by name (e.g., `mariadb`) | No DNS; must use `localhost` or host IP |
| **Port exposure** | Ports must be explicitly published (`ports:`) | All container ports are directly accessible on the host |
| **Security** | Stronger — only explicitly exposed ports are reachable from outside | Weaker — any listening port is immediately on the host interface |
| **Multi-container** | Ideal — each service is reachable by name on the shared network | Awkward — port conflicts between services |

**Why a bridge network?** The project requires container isolation and inter-service communication by name. The `inception` bridge network provides automatic DNS resolution so WordPress can reach `mariadb:3306` and Nginx can proxy to `wordpress:9000` — without exposing internal ports to the host.

### Docker Volumes vs Bind Mounts

| Aspect | Docker Volumes | Bind Mounts |
|--------|---------------|-------------|
| **Management** | Managed by Docker (`docker volume` commands) | Managed by the user (regular directories) |
| **Location** | Docker's internal storage area (e.g., `/var/lib/docker/volumes/`) | Any path on the host filesystem |
| **Portability** | More portable — no dependency on host path structure | Tied to a specific host directory |
| **Performance** | Consistent across platforms | Can have performance issues on macOS/Windows |
| **Backup** | Via `docker volume` commands or `docker cp` | Standard file system tools (`cp`, `tar`, `rsync`) |
| **Driver support** | Supports volume drivers (NFS, cloud storage, etc.) | Local filesystem only |

**Our approach:** We use Docker named volumes with `driver_opts` set to `type: none, o: bind`, which combines Docker's management and naming with explicit host-side storage at `/home/viceda-s/data/`. This gives us the benefits of both: Docker tracks the volumes, and the data is stored at a known, accessible path on the host.

---

## Resources

### Official Documentation
- [Docker Documentation](https://docs.docker.com/)
- [Docker Compose File Reference](https://docs.docker.com/compose/compose-file/)
- [Dockerfile Best Practices](https://docs.docker.com/develop/develop-images/dockerfile_best-practices/)
- [Docker Networking Overview](https://docs.docker.com/network/)
- [Docker Volumes](https://docs.docker.com/storage/volumes/)

### WordPress & PHP
- [WordPress Configuration (wp-config.php)](https://wordpress.org/support/article/editing-wp-config-php/)
- [WP-CLI Command Reference](https://developer.wordpress.org/cli/commands/)
- [PHP-FPM Configuration](https://www.php.net/manual/en/install.fpm.configuration.php)

### Nginx & TLS
- [Nginx Documentation](https://nginx.org/en/docs/)
- [Configuring HTTPS Servers (Nginx)](https://nginx.org/en/docs/http/configuring_https_servers.html)
- [Mozilla SSL Configuration Generator](https://ssl-config.mozilla.org/)

### MariaDB
- [MariaDB Server Documentation](https://mariadb.com/docs/server/)
- [MariaDB Docker Hub Page](https://hub.docker.com/_/mariadb)

### Bonus Services
- [Redis Documentation](https://redis.io/documentation)
- [vsftpd Manual](https://security.appspot.com/vsftpd.html)
- [Adminer Project](https://www.adminer.org/)
- [File Browser Project](https://filebrowser.org/)

### Security
- [OWASP Docker Security Cheat Sheet](https://cheatsheetseries.owasp.org/cheatsheets/Docker_Security_Cheat_Sheet.html)
- [Docker and PID 1 — Best Practices](https://blog.phusion.nl/2015/01/20/docker-and-the-pid-1-zombie-reaping-problem/)

### AI Usage in This Project

AI tools (GitHub Copilot, ChatGPT) were used during development for the following tasks:

1. **Scaffolding and boilerplate** — generating initial Dockerfile structures, shell script templates, and Docker Compose service definitions. Every generated file was reviewed, modified, and tested against the actual infrastructure.

2. **Configuration research** — understanding Nginx TLS directives, PHP-FPM pool settings, MariaDB initialization, and vsftpd passive-mode configuration. AI-provided configurations were validated against official documentation.

3. **Debugging assistance** — diagnosing container startup failures, permission issues on volumes, and PHP-FPM socket vs TCP configuration. All suggested fixes were tested in the running environment.

4. **Documentation** — drafting outlines for README.md, USER_DOC.md, and DEV_DOC.md, including comparison tables and troubleshooting sections. Content was reviewed for accuracy and adapted to the actual project.

AI was **not** used to blindly generate the final project. Every component was critically reviewed, tested, and understood before being included.

---

## Troubleshooting

| Problem | Solution |
|---------|----------|
| Containers won't start | Run `make logs` to check error output; look for missing env vars or port conflicts |
| Permission denied on volumes | `sudo chown -R $USER:$USER /home/viceda-s/data/` |
| Database connection errors | Wait for MariaDB to finish initializing (check `make logs` for "ready for connections") |
| HTTPS certificate warnings | Expected with self-signed certs — accept the warning or use `curl -k` |
| Cannot resolve `viceda-s.42.fr` | Verify `/etc/hosts` contains `127.0.0.1 viceda-s.42.fr` |
| FTP not accessible in browser | Browsers dropped FTP support — use `curl` or `lftp` from the command line |

---

## Project Structure

```
Inception/
├── Makefile                  # Build automation
├── README.md                 # Project overview (this file)
├── USER_DOC.md               # End-user / administrator guide
├── DEV_DOC.md                # Developer documentation
├── secrets/                  # Credential files (git-ignored)
│   ├── db_root_password.txt
│   ├── db_password.txt
│   └── credentials.txt
└── srcs/
    ├── .env                  # Environment variables (git-ignored)
    ├── docker-compose.yml    # Service orchestration
    └── requirements/
        ├── mariadb/          # Database service
        ├── wordpress/        # WordPress + PHP-FPM
        ├── nginx/            # Reverse proxy with TLS
        └── bonus/
            ├── redis/        # Cache service
            ├── ftp/          # FTP server
            ├── adminer/      # Database management UI
            ├── static-site/  # Portfolio website
            └── filebrowser/  # Web-based file manager
```