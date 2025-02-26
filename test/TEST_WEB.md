# Pruebas del Servidor con un Navegador

## Introducción

Este documento describe las pruebas que deben realizarse en el servidor utilizando un navegador web. El objetivo es verificar que el servidor maneja correctamente las solicitudes HTTP y que responde adecuadamente a diferentes situaciones.

## Pasos para las Pruebas

### 1. Verificar la Conectividad

- Abrir el navegador de referencia del equipo.
- Acceder al "Inspector de Red" o "Herramientas para Desarrolladores" (F12 en la mayoría de los navegadores).
- Intentar conectarse al servidor introduciendo la URL en la barra de direcciones.
- Comprobar los encabezados de la solicitud (Request Headers) y la respuesta (Response Headers).

### 2. Servir un Sitio Web Estático

- Verificar que el servidor es capaz de servir archivos HTML, CSS, JavaScript e imágenes sin errores.
- Asegurar que los recursos estáticos se cargan correctamente sin generar errores 404 o 500.

### 3. Manejo de URLs Incorrectas

- Introducir una URL incorrecta o inexistente en el navegador (por ejemplo, `http://servidor/noexiste`).
- Verificar que el servidor devuelve un código de estado HTTP adecuado (por ejemplo, `404 Not Found`).
- Si hay una página de error personalizada, comprobar que se muestra correctamente.

### 4. Intento de Listar Directorios

- Intentar acceder a una ruta de directorio en el servidor (por ejemplo, `http://servidor/directorio/`).
- Si la lista de archivos no debería mostrarse, verificar que el servidor devuelve un error o redirige correctamente.
- Si el servidor permite la visualización de directorios, comprobar que los archivos listados sean los esperados.

### 5. Manejo de Redirecciones

- Acceder a una URL que el servidor debería redirigir (por ejemplo, `http://servidor/antigua-ruta` que redirige a `http://servidor/nueva-ruta`).
- Verificar en los encabezados de la respuesta que el código HTTP es `301 Moved Permanently` o `302 Found`.
- Comprobar que el navegador sigue la redirección correctamente.

### 6. Pruebas Adicionales

- Probar diferentes métodos HTTP (`GET`, `POST`) y verificar que el servidor maneja cada uno correctamente.
- Intentar enviar datos en un formulario y comprobar que se procesan correctamente.
- Simular múltiples conexiones concurrentes para evaluar el rendimiento del servidor.

## Conclusión

Al finalizar estas pruebas, se debe asegurar que el servidor responde adecuadamente a todas las solicitudes y maneja errores de manera controlada. Si se detectan problemas, se deben analizar los registros del servidor y corregir la configuración o el código según sea necesario.

