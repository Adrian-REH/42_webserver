# upload_file.py
import cgi, os
import cgitb
import http.cookies
import io
import sys
from auth import verify_session

# Generator to buffer file chunks
def fbuffer(f, chunk_size=10000):
	while True:
		chunk = f.read(chunk_size)
		if not chunk: break
		yield chunk

def main():
	session_id = verify_session()
	if not session_id:
		print(f"Set-Cookie: session={session_id}")
		print("Content-Type: text/html\r\n")
		print("<h1>Error: Sesion invalida</h1>")
		return
	form = cgi.FieldStorage()
	
	print("Content-Type: text/html\r\n")
	print("")
	# A nested FieldStorage instance holds the file
	fileitem = form['file']
	
	# Test if the file was uploaded
	if fileitem.filename:
		# strip leading path from file name
		# to avoid directory traversal attacks
		filename = os.path.basename(fileitem.filename)
		print(f"<b> {filename}</b>")
		
		real = os.path.abspath(__file__)
		root_dir = os.path.join(os.path.dirname(real), "..")
		path_f = os.path.join(root_dir, "files", filename)
		try:
			f = open(path_f, 'wb', 10000)
		except IOError as exc:
			sys.exit(exc.errno)
		for chunk in fbuffer(fileitem.file):
			f.write(chunk)
		f.close()
		message = 'The file "' + filename + '" was uploaded successfully'
	else:
		message = 'No file was uploaded'
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
			<p>{message}</p>
			<a href="#" id="deleteButton" class="button">Delete {filename}</a>
			<script>
				document.getElementById('deleteButton').addEventListener('click', function(event) {{
					event.preventDefault();  // Evita que el enlace navegue a la URL

					// Realiza una solicitud DELETE al servidor
					fetch('http://localhost:8080/cgi-bin/delete_file.py', {{
						method: 'DELETE',
						headers: {{
							'Content-Type': 'application/json',
						}},
						body: JSON.stringify({{
							fileId: '{filename}',
						}}),
					}})
					.then(response => {{
						if (response.ok) {{
							alert('Archivo eliminado con éxito');
						}} else {{
							alert('Error al eliminar el archivo');
						}}
					}})
					.catch(error => {{
						alert('Error en la solicitud: ' + error);
					}});
				}});
			</script>
		</body>
	</html>
	""")



if __name__ == "__main__":
	if os.environ.get("REQUEST_METHOD") != "POST":
		sys.exit(92)
	main()