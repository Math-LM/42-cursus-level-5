COMPOSE_FILE	= srcs/docker-compose.yml
ENV_FILE		= srcs/.env

all: up

up:
	@mkdir -p /home/viceda-s/data/mariadb
	@mkdir -p /home/viceda-s/data/wordpress
	@sudo docker-compose -f $(COMPOSE_FILE) --env-file $(ENV_FILE) up -d --build

down:
	@sudo docker-compose -f $(COMPOSE_FILE) down

start:
	@sudo docker-compose -f $(COMPOSE_FILE) start

stop:
	@sudo docker-compose -f $(COMPOSE_FILE) stop

clean: down
	sudo docker system prune -af

fclean: clean
	sudo docker volume rm srcs_wp_db srcs_wp_files 2>/dev/null || true
	sudo rm -rf /home/viceda-s/data/mariadb/*
	sudo rm -rf /home/viceda-s/data/wordpress/*

re: fclean up

logs:
	sudo docker-compose -f $(COMPOSE_FILE) logs -f

.PHONY: all up down start stop clean fclean re logs
