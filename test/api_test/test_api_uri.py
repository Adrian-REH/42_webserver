
#CMD: pytest test_api.py -v

import requests
import os
from requests_toolbelt.multipart.encoder import MultipartEncoder

BASE_URL = "http://localhost:8080/"

def test_404():
    response = requests.get(BASE_URL + "noexiste")
    assert response.status_code == 404

def test_uri_file_extension():
    data = {"username": "admin", "password": "admin"}
    response = requests.post(BASE_URL + "cgi-bin/login.py", data=data)
    assert response.status_code == 200
    assert "Formulario CGI en Python" in response.text


def test_uri_file_extension_noncgi():
    response = requests.get(BASE_URL + "cgi-bin/files/val.txt")
    assert response.status_code == 415
    assert "Unsupported Media Type" in response.text

def test_uri_file_without_extension():
    response = requests.get(BASE_URL + "cgi-bin/files/ot")
    assert response.status_code == 404
    assert "Not Found" in response.text

def test_uri_dir_error():
    response = requests.get(BASE_URL + "cgi-bin/files")
    assert response.status_code == 404
    assert "Not Found" in response.text


def test_uri_dir_index():
    response = requests.get(BASE_URL + "cgi-bin/")
    assert response.status_code == 200
    assert "Formulario CGI en Python" in response.text