# login.py
import cgi
import http.cookies

def main():
	form = cgi.FieldStorage()
	if form.getvalue('username') == 'admin' and form.getvalue('password') == 'password':
		cookie = http.cookies.SimpleCookie()
		cookie['session'] = 'valid'
		print(cookie.output())
		print("Location: home.py\n")
	else:
		print("Content-type: text/html\n")
		print("<html><body><h1>Login Failed</h1></body></html>")

if __name__ == "__main__":
	main()