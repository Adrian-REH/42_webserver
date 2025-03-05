
#CMD: pytest test_api.py -v

import requests
import os
from requests_toolbelt.multipart.encoder import MultipartEncoder

BASE_URL = "http://localhost:8080/"

def test_404():
    response = requests.get(BASE_URL + "noexiste")
    assert response.status_code == 404

def test_post_login():
    data = {"username": "admin", "password": "admin"}
    response = requests.post(BASE_URL + "cgi-bin/login.py", data=data)
    assert response.status_code == 200
    assert "Set-Cookie" in response.headers
    assert "Upload a file" in response.text

def test_post_login_error():
    data = {"username": "admin", "password": "123"}
    response = requests.post(BASE_URL + "cgi-bin/login.py", data=data)
    assert response.status_code == 400
    assert "Login Form" in response.text