# Usa debian:bookworm-slim como base
FROM debian:bookworm-slim

ENV DEBIAN_FRONTEND=noninteractive

# Instala Nginx y otras dependencias necesarias
RUN apt-get update && apt-get install -y \
    fcgiwrap \
    python3 \
    strace \
    nginx \
    && rm -rf /var/lib/apt/lists/* \
    && ln -sf /dev/stdout /var/log/nginx/access.log \
    && ln -sf /dev/stderr /var/log/nginx/error.log 

# Copia los archivos de configuración personalizados
COPY /nginx/tools/nginx.conf /etc/nginx/nginx.conf
COPY /nginx/tools/default.conf /etc/nginx/conf.d/default.conf
COPY /nginx/tools/entrypoint.sh /entrypoint.sh
RUN mkdir -p /usr/lib/cgi-bin

COPY /nginx/cgi-bin /usr/lib/cgi-bin

RUN chmod +x /entrypoint.sh


ENTRYPOINT ["/bin/bash", "/entrypoint.sh"]

# Expone el puerto 443
EXPOSE 8080

# Inicia Nginx en primer plano
