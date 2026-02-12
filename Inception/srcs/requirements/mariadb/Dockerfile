FROM debian:bullseye

RUN apt-get update && apt-get install -y \
    mariadb-server \
    mariadb-client \
    && rm -rf /var/lib/apt/lists/*
  
COPY tools/init-db.sh /usr/local/bin/
COPY conf/50-server.cnf /etc/mysql/mariadb.conf.d/
RUN chmod +x /usr/local/bin/init-db.sh

RUN mkdir -p /var/run/mysqld && \
    chown -R mysql:mysql /var/run/mysqld && \
    chmod 777 /var/run/mysqld

EXPOSE 3306

ENTRYPOINT ["init-db.sh"]
CMD ["mysqld"]
