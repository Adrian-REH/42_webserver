import cgi, os, cgit
import http.cookies

# Generator to buffer file chunks
def fbuffer(f, chunk_size=10000):
    while True:
        chunk = f.read(chunk_size)
        if not chunk: break
        yield chunk

def main():

	form = cgi.FieldStorage()
	
	# A nested FieldStorage instance holds the file
	fileitem = form['file']

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

	print """\
	Content-Type: text/html\n
	<html><body>
	<p>%s</p>
	</body></html>
	""" % (message,)


if __name__ == "__main__":
	cgitb.enable()
	main()