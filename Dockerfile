# Laboratorio autocontido: compila nginx e modulo juntos, evitando
# incompatibilidade binaria entre builds diferentes do nginx.
ARG NGINX_VERSION=1.28.0
FROM debian:bookworm-slim AS build
ARG NGINX_VERSION

RUN apt-get update && apt-get install -y --no-install-recommends \
    ca-certificates curl build-essential libpcre2-dev zlib1g-dev \
    && rm -rf /var/lib/apt/lists/*

COPY module /src/module
WORKDIR /build
RUN curl -fsSL "https://nginx.org/download/nginx-${NGINX_VERSION}.tar.gz" \
        -o nginx.tar.gz \
    && tar -xzf nginx.tar.gz \
    && cd "nginx-${NGINX_VERSION}" \
    && ./configure --prefix=/opt/nginx --with-compat \
         --add-dynamic-module=/src/module \
    && make -j2 \
    && make install \
    && mkdir -p /opt/nginx/modules \
    && cp objs/ngx_http_guard_module.so /opt/nginx/modules/

FROM debian:bookworm-slim
RUN apt-get update && apt-get install -y --no-install-recommends \
    libpcre2-8-0 zlib1g \
    && rm -rf /var/lib/apt/lists/*
COPY --from=build /opt/nginx /opt/nginx
COPY sandbox/nginx.conf /opt/nginx/conf/nginx.conf
COPY sandbox/site/ /opt/nginx/html/
EXPOSE 8080
CMD ["/opt/nginx/sbin/nginx", "-g", "daemon off;"]
