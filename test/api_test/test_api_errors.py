
#CMD: pytest test_api.py -v

import requests
import os
from requests_toolbelt.multipart.encoder import MultipartEncoder

BASE_URL = "http://localhost:8080/"

def test_uri_400():
    data = {"username": "admin", "password": "123"}
    response = requests.post(BASE_URL + "cgi-bin/login.py", data=data)
    assert response.status_code == 400
    assert "Login Form" in response.text
    
def test_uri_502():
    response = requests.get(BASE_URL + "cgi-bin/error/error.py")
    assert response.status_code == 502
    assert "Bad Gateway" in response.text

def test_uri_408():
    response = requests.get(BASE_URL + "cgi-bin/error/whiletrue.py")
    assert response.status_code == 408
    assert "Request Timeout" in response.text

def test_uri_404():
    response = requests.get(BASE_URL + "cgi-bin/error/hola!")
    assert response.status_code == 404
    assert "Not Found" in response.text

def test_uri_204():
    response = requests.get(BASE_URL + "cgi-bin/auth.py")
    assert response.status_code == 204
    assert not response.text