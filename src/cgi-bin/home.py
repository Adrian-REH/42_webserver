# home.py
import cgi
from auth import verify_session

def home():
    if verify_session():
        print("Content-type: text/html\n")
        print("<html><body><h1>Welcome, logged in user!</h1></body></html>")
    else:
        print("Content-type: text/html\n")
        print("<html><body><h1>Error User!</h1></body></html>")

