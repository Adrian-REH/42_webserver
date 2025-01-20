# auth.py
import http.cookies

def verify_session():
	cookie = http.cookies.SimpleCookie(os.environ.get('HTTP_COOKIE'))
	return 'session' in cookie and cookie['session'].value == 'valid'

