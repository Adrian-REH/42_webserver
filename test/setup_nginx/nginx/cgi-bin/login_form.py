#!/usr/bin/env python3

import cgi
import cgitb
import os

# Habilita el modo de depuración para mostrar errores en el navegador
cgitb.enable()

def loginForm():
	# Obtener los datos del formulario
 
	form = cgi.FieldStorage()
	mensaje_class = "success"  # O "error" según corresponda
	mensaje = ""  # Variable para el mensaje a mostrar
	# Recuperar los valores enviados (si existen)
	username = form.getvalue("username", "")
	password = form.getvalue("password", "")

	if os.environ.get("REQUEST_METHOD") == "POST":
		# Verificación simple para asegurarnos de que los campos no estén vacíos
		if username == "" or password == "":
			mensaje = "Por favor, ingresa tu username y password."
			mensaje_class = "error"
		else:
			mensaje = "Los datos han sido recibidos."

	# Imprimir el encabezado y el cuerpo del HTML
	print("Content-Type: text/html\r\n")  # Encabezado HTTP
	print("")  # Línea en blanco para separar encabezados del cuerpo

	# Página HTML de respuesta
	print(f"""
	<!DOCTYPE html>
	<html lang="es">
	<head>
		<meta charset="UTF-8">
		<meta name="viewport" content="width=device-width, initial-scale=1.0">
		<title>Login Form</title>
		<style>
			body {{
				font-family: 'Arial', sans-serif;
				background-color: #f4f4f9;
				margin: 0;
				padding: 0;
				display: flex;
				justify-content: center;
				align-items: center;
				height: 100vh;
			}}

			.container {{
				background-color: #ffffff;
				border-radius: 8px;
				box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1);
				padding: 30px;
				width: 100%;
				max-width: 400px;
			}}

			h1 {{
				text-align: center;
				color: #333333;
			}}

			label {{
				font-size: 1rem;
				margin-bottom: 10px;
				display: inline-block;
				color: #555555;
			}}

			input[type="text"], input[type="password"] {{
				width: 100%;
				padding: 10px;
				margin-bottom: 20px;
				border: 1px solid #ddd;
				border-radius: 4px;
				font-size: 1rem;
				box-sizing: border-box;
			}}

			input[type="text"]:focus, input[type="password"]:focus {{
				border-color: #007BFF;
				outline: none;
			}}

			input[type="submit"] {{
				background-color: #007BFF;
				color: white;
				border: none;
				padding: 10px 20px;
				border-radius: 4px;
				font-size: 1rem;
				cursor: pointer;
				width: 100%;
			}}

			input[type="submit"]:hover {{
				background-color: #0056b3;
			}}

			.message {{
				text-align: center;
				font-size: 1.2rem;
				color: #333333;
				margin-top: 20px;
			}}

			.message.error {{
				color: #dc3545;
			}}

			.message.success {{
				color: #28a745;
			}}
		</style>
	</head>
	<body>
		<div class="container">
			<h1>Login Form</h1>
			<form method="post" action="/cgi-bin/login.py">
				<label for="username">Username:</label>
				<input type="text" id="username" name="username" value="{username}">
				
				<label for="password">Password:</label>
				<input type="password" id="password" name="password" value="{password}">
				
				<input type="submit" value="Enviar">
			</form>
			<hr>
			<div class="message {mensaje_class}">{mensaje}</div>
		</div>
	</body>
	</html>
	""")


if __name__ == "__main__":
	loginForm()