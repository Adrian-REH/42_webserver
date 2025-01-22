# login.py
import cgi
import http.cookies
from login_form import loginForm  # Importa la función loginForm
from home import home

def main():
	form = cgi.FieldStorage()
	if form.getvalue('username') == 'admin' and form.getvalue('password') == 'admin':
		print("Content-type: text/html\n")
		print("<html><body><h1>Welcome, logged in user!</h1></body></html>")
	else:
		loginForm()

if __name__ == "__main__":
	main()