# 🐳 Inception

*This project has been created as part of the 42 curriculum by **viceda-s**.*

## Description

**Inception** is a Docker-based infrastructure project that sets up a complete web application environment with WordPress, MariaDB, and Nginx. The project demonstrates practical system administration skills through containerization, service orchestration, and secure configuration management.

### Project Goal

The objective is to virtualize multiple Docker images running on a personal virtual machine, establishing a secure multi-container infrastructure with proper networking, persistence, and security measures. This includes setting up a WordPress website with a database backend, a reverse proxy with TLS encryption, and optional bonus services for enhanced functionality.

### What You Get

- **WordPress**: A fully functional WordPress blog with two users (admin and regular user)
- **MariaDB**: A dedicated database container for persistent data storage
- **Nginx**: A reverse proxy with TLS 1.2/1.3 encryption as the sole entry point
- **Bonus Services**: Redis caching, FTP server, Adminer DB management, and a static portfolio site

---

## Instructions

### Prerequisites

- A virtual machine with Docker and Docker Compose installed
- Sufficient disk space for volumes (~1GB recommended)
- A text editor for configuration
- Basic command-line knowledge

### Installation & Setup

1. **Clone the repository:**
   ```bash
   git clone <your-repo-url> Inception
   cd Inception
   ```

2. **Configure your login:**
    - Replace `<your-login>` with your actual 42 login throughout the project:
        - In `srcs/.env`
        - In `docker-compose.yml`
        - In Nginx configuration
        - In the Makefile
        - In `/etc/hosts` (optional, for local testing)

3. **Set up credentials (never commit these):**
   ```bash
   mkdir -p secrets
   echo "your_secure_root_password" > secrets/db_root_password.txt
   echo "your_wordpress_password" > secrets/db_password.txt
   echo "admin_user:admin_password" > secrets/credentials.txt
   ```

4. **Configure your domain:**
    - Add to your VM's `/etc/hosts` (Linux/Mac):
      ```bash
      127.0.0.1 <your-login>.42.fr
      ```
    - Or use your VM's IP address instead of 127.0.0.1

5. **Build and launch:**
   ```bash
   make
   ```
   Or:
   ```bash
   make up
   ```

### Common Commands

| Command | Purpose |
|---------|---------|
| `make` or `make up` | Build and start all containers |
| `make down` | Stop and remove containers |
| `make stop` | Stop containers without removing |
| `make start` | Start stopped containers |
| `make logs` | View real-time container logs |
| `make clean` | Remove containers and images |
| `make fclean` | Full clean including volumes |
| `make re` | Rebuild everything from scratch |

### Accessing Services

Once the infrastructure is running:

- **WordPress Website**: `https://<your-login>.42.fr`
- **WordPress Admin Panel**: `https://<your-login>.42.fr/wp-admin`
- **Adminer Database Manager**: `https://<your-login>.42.fr/adminer`
- **Portfolio (Static Site)**: `https://<your-login>.42.fr/portfolio`
- **FTP Server**: `ftp://<your-login>.42.fr:21`
    - User: `ftpuser`
    - Password: See `secrets/credentials.txt`

### Important Notes

- **HTTPS Warning**: The certificate is self-signed. Your browser will show a security warning—this is normal.
- **Credentials**: Never commit sensitive information. Use `secrets/` folder and `.env` file.
- **Volume Persistence**: All WordPress data is stored in `/home/login/data/`—this persists across container restarts.

---

## Project Description

### Docker & Architecture Overview

This project demonstrates key Docker concepts:

#### **Containerization**
Each service runs in its own isolated container:
- **mariadb**: Database service
- **wordpress**: Application server with PHP-FPM
- **nginx**: Web server and reverse proxy
- **redis**: (Bonus) Cache service
- **ftp**: (Bonus) File transfer service
- **adminer**: (Bonus) Database UI
- **static-site**: (Bonus) Portfolio website

#### **Networking**
A user-defined bridge network (`inception_net`) connects all containers. This allows:
- Service-to-service communication by hostname (e.g., `wordpress` connects to `mariadb:3306`)
- Isolation from the host network
- No need for `--link` or `network: host` (which are discouraged)

#### **Persistence**
Two volumes store persistent data:
- **wp_db**: Contains MariaDB's data directory, bound to `/home/login/data/mariadb`
- **wp_files**: Contains WordPress files, bound to `/home/login/data/wordpress`

Data survives container restarts and recreation.

#### **Security**
- Nginx uses TLS 1.2/1.3 only (no insecure protocols)
- Nginx is the sole external entry point (port 443)
- No passwords hardcoded in Dockerfiles
- Secrets stored in `secrets/` folder (git-ignored)
- Environment variables for configuration

---

## Design Choices & Comparisons

### Virtual Machines vs Docker

| Aspect | Virtual Machines | Docker |
|--------|------------------|--------|
| **Size** | Several GB per VM | Tens to hundreds of MB per image |
| **Boot Time** | Minutes | Seconds |
| **Resource Overhead** | High (full OS) | Low (shares kernel) |
| **Isolation** | Complete OS-level isolation | Process-level isolation |
| **Use Case** | Full OS simulation, legacy apps | Microservices, modern apps |

**Why Docker for this project?** Lightweight, portable, and ideal for multi-container applications. Docker excels at running application stacks without the overhead of full VMs.

### Secrets vs Environment Variables

| Feature | Secrets | Environment Variables |
|---------|---------|----------------------|
| **Storage** | `secrets/` files, mounted at runtime | `.env` file or compose file |
| **Security** | Better isolation, not visible in ps/logs | Visible in process environment |
| **Use For** | Passwords, API keys, sensitive data | Configuration, non-sensitive settings |
| **Docker Support** | First-class in Compose | Standard environment |

**Our approach:** Database passwords use Docker secrets; domain names and service hosts use environment variables.

### Docker Network vs Host Network

| Feature | Docker Network | Host Network |
|---------|---|---|
| **Isolation** | Containers isolated, can't access host ports directly | Containers share host's network namespace |
| **Port Binding** | Explicit mapping required | Direct access to host ports |
| **Service Discovery** | DNS-based (e.g., `mariadb:3306`) | Use `localhost` or `127.0.0.1` |
| **Security** | Better isolation, network policies possible | Potential security risks |
| **Portability** | Highly portable across machines | Less portable |

**Why Docker Network?** The subject requires it. Docker's user-defined networks provide cleaner isolation and built-in DNS service discovery, allowing containers to communicate by name.

### Docker Volumes vs Bind Mounts

| Feature | Docker Volumes | Bind Mounts |
|---------|---|---|
| **Management** | Managed by Docker | User manages the directory |
| **Driver Support** | Can use custom drivers | Only local filesystem |
| **Performance** | Good on all platforms | Issues on Docker Desktop (Mac/Windows) |
| **Persistence Location** | Docker's data directory | Specified directory on host |
| **Use For** | General persistent data | Development, debugging, specific mount points |

**Our approach:** We use Docker volumes with bind mount options (`driver_opts`), giving us Docker's management plus explicit host-side persistence at `/home/login/data/`.

---

## Resources

### Docker & Containerization
- [Official Docker Documentation](https://docs.docker.com/)
- [Docker Compose Reference](https://docs.docker.com/compose/compose-file/)
- [Best Practices for Writing Dockerfiles](https://docs.docker.com/develop/dev-best-practices/)
- [Understanding Docker Networking](https://docs.docker.com/network/)
- [Docker Volumes Documentation](https://docs.docker.com/storage/volumes/)

### WordPress & PHP-FPM
- [WordPress Core Documentation](https://wordpress.org/support/article/editing-wp-config-php/)
- [PHP-FPM Configuration](https://www.php.net/manual/en/install.fpm.configuration.php)
- [WordPress CLI Reference](https://developer.wordpress.org/cli/commands/)

### Nginx & TLS
- [Nginx Documentation](https://nginx.org/en/docs/)
- [Configuring HTTPS Servers](https://nginx.org/en/docs/http/configuring_https_servers.html)
- [TLS 1.3 Configuration](https://www.digitalocean.com/community/tutorials/how-to-set-up-nginx-with-http-2-support-on-ubuntu-20-04)

### MariaDB & MySQL
- [MariaDB Official Documentation](https://mariadb.com/docs/server/)
- [Docker MariaDB Setup](https://hub.docker.com/_/mariadb)

### Security Best Practices
- [OWASP Docker Security](https://cheatsheetseries.owasp.org/cheatsheets/Docker_Security_Cheat_Sheet.html)
- [PID 1 and Docker](https://alexei-led.github.io/post/dockerfile_best-practices/)

### Bonus Services
- [Redis Cache Documentation](https://redis.io/documentation)
- [vsftpd Configuration](https://security.appspot.com/vsftpd.html)
- [Adminer Project](https://www.adminer.org/)

### AI Usage in This Project

AI was used for the following tasks to enhance productivity and learning:

1. **Code Generation & Scaffolding** (Docker Compose, Dockerfiles, shell scripts)
    - Generated boilerplate Dockerfile structures for each service
    - Created bash initialization scripts for database and WordPress setup
    - Built configuration templates for Nginx, MariaDB, and vsftpd
    - *Personal validation*: Each generated script was reviewed, tested, and customized to match project requirements

2. **Documentation & Explanations**
    - Generated initial outlines for README and documentation files
    - Provided technical explanations of Docker networking, volumes, and security
    - Created comparison tables for VMs vs Docker, Secrets vs Env Vars, etc.
    - *Personal validation*: Verified accuracy against official documentation and personal testing

3. **Troubleshooting & Problem-Solving**
    - AI assisted in debugging TLS configuration issues
    - Provided guidance on PHP-FPM socket vs TCP listening
    - Explained PID 1 and proper daemon management in containers
    - *Personal validation*: Tested all recommendations in the actual environment

4. **Configuration Best Practices**
    - AI suggested security hardening for Nginx, MariaDB, and FTP
    - Provided guidance on environment variable handling and Docker secrets
    - *Personal validation*: Aligned suggestions with 42 project requirements and security standards

**Key Takeaway**: AI accelerated development and research, but every component was critically reviewed, tested, and adapted to ensure alignment with project specifications and security requirements.

---

## Troubleshooting

### Containers won't start
```bash
docker-compose -f srcs/docker-compose.yml logs <service-name>
```
Check for missing environment variables or port conflicts.

### Permission denied on volumes
```bash
sudo chown -R $USER:$USER /home/$(whoami)/data/
```

### Database connection errors
Wait for MariaDB to fully initialize (check logs):
```bash
make logs
```

### HTTPS certificate warnings
This is expected with self-signed certificates. Use `-k` flag with curl or accept warnings in your browser.

### Can't resolve domain
Ensure your `/etc/hosts` entry is correct or use your VM's IP address directly.

---

## Project Structure

```
Inception/
├── Makefile
├── README.md
├── USER_DOC.md
├── DEV_DOC.md
├── secrets/
│   ├── db_root_password.txt
│   ├── db_password.txt
│   └── credentials.txt
└── srcs/
    ├── .env
    ├── docker-compose.yml
    └── requirements/
        ├── mariadb/
        │   ├── Dockerfile
        │   ├── .dockerignore
        │   ├── conf/
        │   │   └── 50-server.cnf
        │   └── tools/
        │       └── init-db.sh
        ├── wordpress/
        │   ├── Dockerfile
        │   ├── .dockerignore
        │   ├── conf/
        │   └── tools/
        │       └── setup-wordpress.sh
        ├── nginx/
        │   ├── Dockerfile
        │   ├── .dockerignore
        │   ├── conf/
        │   │   ├── nginx.conf
        │   │   └── default.conf
        │   └── tools/
        └── bonus/
            ├── redis/
            ├── ftp/
            ├── adminer/
            └── static-site/
```

---

## License & Attribution

This project is part of the 42 curriculum. It is for educational purposes only.

**Disclaimer**: This is a simplified stack for learning Docker and system administration. For production use, additional security measures, monitoring, and backup strategies are essential.