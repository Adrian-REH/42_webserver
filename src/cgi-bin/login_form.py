#!/usr/bin/env python3

import cgi
import cgitb

# Habilita el modo de depuración para mostrar errores en el navegador
cgitb.enable()

def loginForm():
    # Obtener los datos del formulario
    form = cgi.FieldStorage()

    # Recuperar los valores enviados (si existen)
    username = form.getvalue("username", "No proporcionado")
    password = form.getvalue("password", "No proporcionada")

    # Verificación simple para asegurarnos de que los campos no estén vacíos
    if username == "No proporcionado" and password == "No proporcionada":
        mensaje = "Por favor, ingresa tu username y password."
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
        <form method="post" action="/cgi-bin/login.py">
            <label for="username">Username:</label><br>
            <input type="text" id="username" name="username" value="{username}"><br><br>
            <label for="password">Password:</label><br>
            <input type="password" id="password" name="password" value="{password}"><br><br>
            <input type="submit" value="Enviar">
        </form>
        <hr>
        <h2>{mensaje}</h2>
        <p><strong>Username:</strong> {username}</p>
        <p><strong>Password:</strong> {password}</p>
    </body>
    </html>
    """)
