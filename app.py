#!/usr/bin/env python3

import cgi
import cgitb

# Habilita el modo de depuración para mostrar errores en el navegador
cgitb.enable()

print("Content-Type: text/html")  # Header HTTP
print()  # Línea vacía obligatoria después del header

# Obtener los datos del formulario
form = cgi.FieldStorage()

# Recuperar los valores enviados (si existen)
nombre = form.getvalue("nombre", "No proporcionado")
edad = form.getvalue("edad", "No proporcionada")

# Página HTML de respuesta
print(f"""
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>CGI Form</title>
</head>
<body>
    <h1>Formulario CGI en Python</h1>
    <form method="post" action="/cgi-bin/app.py">
        <label for="nombre">Nombre:</label><br>
        <input type="text" id="nombre" name="nombre"><br><br>
        <label for="edad">Edad:</label><br>
        <input type="number" id="edad" name="edad"><br><br>
        <input type="submit" value="Enviar">
    </form>
    <hr>
    <h2>Resultados:</h2>
    <p><strong>Nombre:</strong> {nombre}</p>
    <p><strong>Edad:</strong> {edad}</p>
</body>
</html>
""")
