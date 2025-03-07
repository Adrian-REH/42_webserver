# upload_file.py
import cgi, os
import cgitb
import http.cookies
import io
import sys
from auth import verify_session
import json
import errno

def parser_body():
	content_length = os.environ.get("CONTENT_LENGTH", "0")
	if content_length.isdigit() and int(content_length) > 0:
		body = sys.stdin.read(int(content_length))  # Leer el body de la request
		try:
			data = json.loads(body)
			file_id = data.get("fileId")
			if file_id:
				return file_id
			else:
				print(json.dumps({"status": "error", "message": "Falta fileId"}))
				sys.exit(1)
		except json.JSONDecodeError:
			print(json.dumps({"status": "error", "message": "JSON inválido"}))
			sys.exit(1)
	else:
		print(json.dumps({"status": "error", "message": "Cuerpo vacío"}))
		sys.exit(1)

def main():
	session = verify_session()
	if not session:
		print(f"Set-Cookie: session={session}")
		print("Content-Type: text/html\r\n")
		print("<h1>Error: Sesion invalida</h1>")
		sys.exit(errno.EACCES)  # Error: permisos denegados
	real = os.path.abspath(__file__)
	root_dir = os.path.join(os.path.dirname(real), "..")
	filename = parser_body()
	path_f = os.path.join(root_dir, "files", filename)
	try:
		os.remove(path_f)
	except FileNotFoundError:
		print("Error: No se encontró el archivo.")
		sys.exit(errno.ENOENT)
	except PermissionError:
		print("Error: No tienes permiso para eliminar el archivo.")
		sys.exit(errno.EACCES)
	except OSError as e:
		print(f"Error: {e.strerror}")
		sys.exit(e.errno)  # Error genérico con código
		
	print("Content-Type: text/html\r\n")
	print("")
	print(f"""
	<!DOCTYPE html>
	<html lang="es">
		<head>
			<meta charset="UTF-8">
			<meta name="viewport" content="width=device-width, initial-scale=1.0">
		<style>
		.button {{
			background-color: #007BFF;
			color: white;
			border: none;
			padding: 10px 20px;
			border-radius: 4px;
			font-size: 1rem;
			cursor: pointer;
			text-decoration: none;
		}}
		.button:hover {{
			background-color: #0056b3;
		}}
		</style>
		</head>
		<body>
			<h1>Archivo eliminado correctamente</h1>
		</body>
	</html>
	""")


if __name__ == "__main__":
	if os.environ.get("REQUEST_METHOD") == "DELETE":
		main()
	else:
		print("Content-Type: text/html\r\n")
		print("<h1>Error: Sesion invalida</h1>")
		sys.exit(95)
