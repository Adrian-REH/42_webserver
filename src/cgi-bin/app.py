#!/usr/bin/env python3

import cgi
import cgitb

# Habilita el modo de depuración para mostrar errores en el navegador
cgitb.enable()

# Obtener los datos del formulario
form = cgi.FieldStorage()

# Recuperar los valores enviados (si existen)
nombre = form.getvalue("nombre", "No proporcionado")
edad = form.getvalue("edad", "No proporcionada")

# Verificación simple para asegurarnos de que los campos no estén vacíos
if nombre == "No proporcionado" and edad == "No proporcionada":
    mensaje = "Por favor, ingresa tu nombre y edad."
else:
    mensaje = "Los datos han sido recibidos."

# Imprimir el encabezado y el cuerpo del HTML
print("Content-Type: text/html")  # Encabezado HTTP
print()  # Línea en blanco para separar encabezados del cuerpo

# Página HTML de respuesta
print(f"""
<!DOCTYPE html>
<html lang="es">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Formulario CGI en Python</title>
</head>
<body>
    <h1>Formulario CGI en Python</h1>
    <form method="post" action="/cgi-bin/app.py">
        <label for="nombre">Nombre:</label><br>
        <input type="text" id="nombre" name="nombre" value="{nombre}"><br><br>
        <label for="edad">Edad:</label><br>
        <input type="number" id="edad" name="edad" value="{edad}"><br><br>
        <input type="submit" value="Enviar">
    </form>
    <hr>
    <h2>{mensaje}</h2>
    <p><strong>Nombre:</strong> {nombre}</p>
    <p><strong>Edad:</strong> {edad}</p>
</body>
</html>
""")
