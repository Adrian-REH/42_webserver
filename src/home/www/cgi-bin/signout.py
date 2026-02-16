# login.py
import cgi
import http.cookies
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

def main():
	session_id = fetch_srv_session_id()
	print(f"Set-Cookie: session=invalid;")
	print(f"Set-Cookie: session_id={session_id};")
	print("Content-Type: text/html\r\n\r\n")
	print(f"""<!DOCTYPE html>
	<html lang="es">
	<head>
		<meta charset="UTF-8">
		<meta name="viewport" content="width=device-width, initial-scale=1.0">
		<title>Sesión Finalizada</title>
		<style>
			body {{
				font-family: Arial, sans-serif;
				display: flex;
				justify-content: center;
				align-items: center;
				height: 100vh;
				background-color: #f8f9fa;
				margin: 0;
			}}
			.container {{
				text-align: center;
				background: white;
				padding: 20px;
				border-radius: 10px;
				box-shadow: 0px 4px 6px rgba(0, 0, 0, 0.1);
			}}
			.message {{
				font-size: 24px;
				color: #333;
				margin-bottom: 20px;
			}}
			.button {{
				display: inline-block;
				padding: 10px 20px;
				font-size: 18px;
				color: white;
				background-color: #007bff;
				text-decoration: none;
				border-radius: 5px;
				transition: background 0.3s ease;
			}}
			.button:hover {{
				background-color: #0056b3;
			}}
		</style>
	</head>
	<body>
		<div class="container">
			<div class="message">Sesión Finalizada</div>
			<a class="button" href="/login.py" >Volver a login</a>
		</div>
	</body>
	</html>""")


if __name__ == "__main__":
	main()