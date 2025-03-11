# SETUP Proxy with Nginx


Si se necesita hacer un proxy inverso hacia nginx/webserver
```c
 mitmproxy --mode reverse:http://localhost:8080 --listen-port 9000
```

en la terminal se podra ver las req/res de cada peticion y testear diferentes casos.