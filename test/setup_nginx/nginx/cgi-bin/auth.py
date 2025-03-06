# auth.pyimport cgi
import cgi
import http.cookies
import os

def verify_session():
    try:
        cookie_header = os.environ.get('HTTP_COOKIE', '')
        if not cookie_header:
            return False
        cookie = http.cookies.SimpleCookie(cookie_header)
        if 'session' in cookie:
            return cookie['session'].value == 'valid'
        return False
    except Exception as e:
        print(f"Error procesando cookies: {e}")
        return False

