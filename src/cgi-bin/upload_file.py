# upload_file.py
import cgi, os
import cgitb
import http.cookies
import io
import sys

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
	return 1
	while True:
		return f"<H3>BEFORE chunk</H3>"
		print(f"<H3>BEFORE chunk</H3>")
		
		chunk = f.read(chunk_size)
		if not chunk: break
		yield chunk

def main():
	
	'''
	session_id = verify_session()
	if not session_id:
		print(f"Set-Cookie: session_id={session_id}")
		print("Content-Type: text/html\r\n")
		print("<h1>Error: Sesión invalida</h1>")
		return '''
	#cgitb.enable()
	headers = {u'content-disposition': 'form-data; name="file"; filename="text_to_upload.txt"\r\n',
		u'content-type': os.environ["CONTENT_TYPE"],
		u'content-length': str(len(sys.argv[1]))}
	form = cgi.FieldStorage(fp=io.BytesIO(sys.argv[1].encode("utf-8")), environ=os.environ)
	#form = cgi.FieldStorage()
	#
	#form.type = "multipart/form-data"
	#print(f"""Set-Cookie: session_id={session_id}""")
	#if "filename" not in form:
	#	print("Content-Type: text/html\r\n")
	#	print("<H1>Error: Archivo invalido</H1>")
	#	return
	
	print("Content-Type: text/html\r\n")
	print("")
	print(f"<H1>Error: Archivo {form}</H1>")
	print ('argument list', sys.argv)
	print(f"<H3>{cgi.print_environ()}</H3>")
	print(f"<H1>HEADERS: {form.headers}</H1>")
	print(f"<H1>{form.keys()}</H1>")
	
	# A nested FieldStorage instance holds the file
	fileitem = form['file']
	
	print(f"<H1>{fileitem}</H1>")
	print(f"<H1>{fileitem.file}</H1>")
	print(f"<H1>{fileitem.filename}</H1>")
	
	# Test if the file was uploaded
	if fileitem.filename:

		# strip leading path from file name
		# to avoid directory traversal attacks
		filename = os.path.basename(fileitem.filename)
		print(f"<b> {filename}</b>")
		
		real = os.path.abspath(__file__)
		dir_path = os.path.dirname(real)
		
		print(f"<H3> {dir_path}</H3>")
		print(f"<H3> {real}</H3>")
		
		path_f = os.path.join(dir_path, "files",filename)
		print(f"<H3> {path_f}</H3>")
		try:
			
			f = open(path_f, 'wb')
		except IOError as exc:
			""" tb = sys.exc_info()[-1]
			lineno = tb.tb_lineno
			filename = tb.tb_frame.f_code.co_filename """
			print('<b>{} <b>.'.format(exc.strerror))
			sys.exit(exc.errno)
		print(f"<H3> EEEEEE</H3>")

		print(fbuffer(f))
		return
		for chunk in fbuffer(f):
			f.write(fileitem.file.read())
		f.close()
		message = 'The file "' + fn + '" was uploaded successfully'

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