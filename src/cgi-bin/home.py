# home.py
import cgi
from auth import verify_session

def main():
    if verify_session():
        print("Content-type: text/html\n")
        print("<html><body><h1>Welcome, logged in user!</h1></body></html>")
    else:
        print("Location: login.py\n")

if __name__ == "__main__":
	main()
