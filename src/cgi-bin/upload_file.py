# upload_file.py
import cgi, os
import cgitb
import http.cookies

def verify_session():
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

# Generator to buffer file chunks
def fbuffer(f, chunk_size=10000):
	while True:
		chunk = f.read(chunk_size)
		if not chunk: break
		yield chunk

def main():
	
	session_id = verify_session()
	if not session_id:
		print(f"Set-Cookie: session_id={session_id}")
		print("Content-Type: text/html\r\n")
		print("<h1>Error: Sesión invalida</h1>")
		return 
	#cgitb.enable()
	form = cgi.FieldStorage()

	print(f"""Set-Cookie: session_id={session_id}""")
	if "filename" not in form:
		print("Content-Type: text/html\r\n")
		print("<H1>Error: Archivo invalido</H1>")
		return
	

	# A nested FieldStorage instance holds the file
	fileitem = form['filename']

	# Test if the file was uploaded
	if fileitem.filename:

		# strip leading path from file name
		# to avoid directory traversal attacks
		filename = os.path.basename(fileitem.filename)
		f = open('files/' + filename, 'wb')
		for chunk in fbuffer(fileitem.file):
			f.write(fileitem.file.read())
		f.close()
		message = 'The file "' + fn + '" was uploaded successfully'

	else:
		message = 'No file was uploaded'
	print("Content-Type: text/html\r\n")
	print("") 
	print(f"""
	<!DOCTYPE html>
	<html lang="es">
		<head>
			<meta charset="UTF-8">
			<meta name="viewport" content="width=device-width, initial-scale=1.0">
		</head>
		<body>
			<p>{message}</p>
		</body>
	</html>
	""")


if __name__ == "__main__":
	
	main()