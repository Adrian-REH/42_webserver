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

def main():
	form = cgi.FieldStorage()
	if (form.getvalue('username') == 'admin' and form.getvalue('password') == 'admin') or verify_session():
		session_id = fetch_srv_session_id()
		print(f"Set-Cookie: session=valid;")
		print(f"Set-Cookie: session_id={session_id};")
		home()
	else:
		loginForm()

if __name__ == "__main__":
	main()