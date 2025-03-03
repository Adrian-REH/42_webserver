# login.py
import cgi
import http.cookies
from login_form import loginForm
import os
from auth import verify_session
from home import home

def fetch_srv_session_id():
	try:
		cookie_header = os.environ.get('HTTP_COOKIE', '')
		if not cookie_header:
			return False
		cookie = http.cookies.SimpleCookie(cookie_header)
		if 'session_id' in cookie:
			session_id = cookie['session_id'].value
			return session_id if session_id else False
		return False
	except Exception as e:
		print(f"Error procesando cookies: {e}")
		return False

def success():
	mensaje = "¡Bienvenido, usuario logueado!"
	mensaje_class = "success"
	session_id = fetch_srv_session_id()
	if not session_id:
		print(f"Set-Cookie: session=invalid;")
		print(f"Set-Cookie: session_id={session_id};")
		print("Content-Type: text/html\r\n")
		print(f"""<!DOCTYPE html>
	<html lang="es">
	<head>
		<meta charset="UTF-8">
		<meta name="viewport" content="width=device-width, initial-scale=1.0">
		<title>Error: Sesión inválida</title>
	</head>
	<body>
		<div class="container">
			<div class="message Sesión inválida">
				Sesión inválida
			</div>
		</div>
	</body>
	</html>""")
		return False
	print(f"Set-Cookie: session=valid;")
	print(f"Set-Cookie: session_id={session_id};")
	print("Content-Type: text/html\r\n")
	print(f"""
	<!DOCTYPE html>
	<html lang="es">
	<head>
		<meta charset="UTF-8">
		<meta name="viewport" content="width=device-width, initial-scale=1.0">
		<title>Página de Inicio</title>
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
				padding: 40px;
				width: 100%;
				max-width: 600px;
				text-align: center;
			}}

			h1 {{
				color: #333333;
				font-size: 2rem;
			}}

			.message {{
				font-size: 1.2rem;
				padding: 20px;
				border-radius: 4px;
				margin-top: 20px;
			}}

			.message.success {{
				background-color: #28a745;
				color: white;
			}}

			.message.error {{
				background-color: #dc3545;
				color: white;
			}}

			.message.info {{
				background-color: #007bff;
				color: white;
			}}
		</style>
	</head>
	<body>
		<div class="container">
			<h1>Página de Inicio</h1>
			<div class="message {mensaje_class}">
				{mensaje}
			</div>
		</div>
	</body>
	</html>
	""")
	return 0

def main():
	form = cgi.FieldStorage()
	print(form.getvalue('username'))
	print(form.getvalue('password'))
	if (form.getvalue('username') == 'admin' and form.getvalue('password') == 'admin'):
		success()
	elif verify_session():
		home()
	else:
		loginForm()

if __name__ == "__main__":
	main()