# CGI Server Testing

## Descripción
Este documento proporciona una guía detallada para probar el funcionamiento del CGI en el servidor, asegurando que maneja correctamente los métodos `GET` y `POST`, así como la gestión de errores.

## Requisitos
- Servidor web configurado para ejecutar CGI.
- Un script CGI funcional (`script.py`).
- Herramientas como `curl` o `siege` para realizar pruebas.

---

## 1. Verificación del funcionamiento del CGI
### **GET Request**
Para probar una solicitud GET al script CGI, ejecuta:
```sh
curl -X GET "http://localhost/cgi-bin/script.py"
```
El servidor debe ejecutar el script y devolver una respuesta válida.

### **POST Request**
Para probar una solicitud POST con datos enviados al script:
```sh
curl -X POST "http://localhost/cgi-bin/script.py" -d "username=test&password=1234"
```
El CGI debe procesar la entrada y devolver una respuesta adecuada.

---

## 2. Pruebas con archivos erróneos
### **Bucle infinito**
Prueba con un script que genera un bucle infinito:
```python
while True:
    pass
```
El servidor debe manejar el timeout y no bloquearse.

### **Error en el código**
Prueba un script con un error sintáctico, por ejemplo:
```python
print("Hello" # Falta un paréntesis
```
El servidor debe devolver un error en lugar de colapsar.

---

## 3. Manejo de errores
- **Si el CGI no existe**, debe devolver `404 Not Found`.
- **Si el script tiene errores**, debe devolver `500 Internal Server Error`.
- **Si el CGI se ejecuta correctamente**, debe devolver `200 OK`.

---

## 4. Prueba de carga
Ejecuta `siege` para probar múltiples solicitudes simultáneas:
```sh
siege -c 10 -t 30S http://localhost/cgi-bin/script.py
```
Esto enviará 10 solicitudes concurrentes por 30 segundos, verificando la estabilidad del servidor.

---

## 5. Consideraciones Finales
- El servidor nunca debe **caerse** o bloquearse.
- El manejo de errores debe ser claro y comprensible.
- Las pruebas deben realizarse con la ayuda del equipo para garantizar un entorno de prueba completo.

¡Buena suerte probando el CGI! 🚀

