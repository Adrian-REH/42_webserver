

## **1️⃣ Configuración con múltiples puertos y diferentes sitios web**
Aquí configuramos dos sitios web, uno en el puerto **8080** y otro en el **9090**.

📌 **Objetivo**: Servir diferentes sitios en diferentes puertos.

```nginx
server {
    listen 8080;
    server_name mysite1.com;

    root /var/www/mysite1;
    index index.html;

    location / {
        try_files $uri $uri/ =404;
    }
}

server {
    listen 9090;
    server_name mysite2.com;

    root /var/www/mysite2;
    index index.html;

    location / {
        try_files $uri $uri/ =404;
    }
}
```

🔹 **Prueba en el navegador**:
- Abre `http://localhost:8080` y verifica que muestra el contenido de `mysite1`.
- Abre `http://localhost:9090` y verifica que muestra el contenido de `mysite2`.

---

## **2️⃣ Configuración incorrecta: Mismo puerto en varios servidores con `server_name` distinto**
📌 **Objetivo**: Nginx elegirá el primer bloque que coincida con la solicitud.

```nginx
server {
    listen 8080;
    server_name mysite1.com;

    root /var/www/mysite1;
    index index.html;
}

server {
    listen 8080;
    server_name mysite2.com;

    root /var/www/mysite2;
    index index.html;
}
```

🚨 **Resultado esperado**:
- Si accedes a `http://localhost:8080`, el **primer bloque** será el que responda si no se especifica un `Host`.
- Si accedes con `mysite1.com` o `mysite2.com`, funcionará correctamente si el `Host` está bien configurado en `/etc/hosts`.

---

## **3️⃣ Configuración incorrecta: Intento de usar el mismo puerto dos veces en la misma configuración**
📌 **Objetivo**: Ver qué pasa si intentamos definir el mismo puerto dos veces en distintos bloques.

```nginx
server {
    listen 8080;
    server_name mysite1.com;

    root /var/www/mysite1;
    index index.html;
}

server {
    listen 8080;
    server_name mysite2.com;

    root /var/www/mysite2;
    index index.html;
}
```

🚨 **Error esperado**:
```
nginx: [emerg] bind() to 0.0.0.0:8080 failed (98: Address already in use)
```
- Nginx no permitirá que el mismo puerto escuche dos veces sin una configuración adecuada de `server_name`.

---

## **4️⃣ Lanzar múltiples servidores con configuraciones diferentes pero con puertos comunes**
📌 **Objetivo**: Lanzar múltiples instancias de Nginx con diferentes configuraciones.

1️⃣ **Servidor 1 (`/etc/nginx/nginx1.conf`)**
```nginx
worker_processes 1;

events {
    worker_connections 1024;
}

http {
    server {
        listen 8080;
        server_name mysite1.com;
        root /var/www/mysite1;
        index index.html;
    }
}
```

2️⃣ **Servidor 2 (`/etc/nginx/nginx2.conf`)**
```nginx
worker_processes 1;

events {
    worker_connections 1024;
}

http {
    server {
        listen 8080;
        server_name mysite2.com;
        root /var/www/mysite2;
        index index.html;
    }
}
```

🔹 **Cómo lanzarlos por separado**:
```bash
nginx -c /etc/nginx/nginx1.conf
nginx -c /etc/nginx/nginx2.conf
```
🚨 **Error esperado**:
- Si ambos servidores intentan escuchar el mismo puerto, **uno de ellos fallará** porque el puerto ya está en uso.

