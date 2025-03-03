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

def handle_request_file(fileitem):
	filename = os.path.basename(fileitem.filename)
	print(f"<b> {filename}</b>")
	
	real = os.path.abspath(__file__)
	dir_path = os.path.dirname(real)
	

	path_f = os.path.join(dir_path, "files", filename)
	try:
		f = open(path_f, 'wb', 10000)
	except IOError as exc:
		""" tb = sys.exc_info()[-1]
		lineno = tb.tb_lineno
		filename = tb.tb_frame.f_code.co_filename """
		print('<b>{} <b>.'.format(exc.strerror))
		sys.exit(exc.errno)
	for chunk in fbuffer(fileitem.file):
		f.write(chunk)
	f.close()
	message = 'The file "' + filename + '" was uploaded successfully'

def main():
	session_id = verify_session()
	if not session_id:
		print(f"Set-Cookie: session_id={session_id}")
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
		handle_request_file(fileitem)
	else:
		message = 'No file was uploaded'
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