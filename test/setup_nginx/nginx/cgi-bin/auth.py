#!/usr/bin/env python3
import http.cookies
def verify_session():
    try:
        cookie_header = os.environ.get('HTTP_COOKIE', '')
        if not cookie_header:
            return False
        cookie = http.cookies.SimpleCookie(cookie_header)
        if 'session' in cookie:
            session = cookie['session'].value
            return session if session else False
        return False
    except Exception as e:
        print(f"Error procesando cookies: {e}")
        return False

