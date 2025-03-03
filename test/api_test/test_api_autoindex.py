
#CMD: pytest test_api.py -v

import requests
import os
from requests_toolbelt.multipart.encoder import MultipartEncoder

BASE_URL = "http://localhost:8081/"

def test_404():
    response = requests.get(BASE_URL + "noexiste")
    assert response.status_code == 404

def test_get_autoindex():
    response = requests.get(BASE_URL + "cgi-bin")
    assert response.status_code == 200
    assert "Index of cgi-bin/" in response.text

def test_download_file():
    response = requests.get(BASE_URL + "cgi-bin/login.py")
    assert response.status_code == 200
    assert "Content-Disposition" in response.headers
    assert "login.py" in response.headers

def test_access_dir():
    response = requests.get(BASE_URL + "cgi-bin/files/")
    assert response.status_code == 200
    assert "Index of cgi-bin/files" in response.text

def test_download_dir():
    response = requests.get(BASE_URL + "cgi-bin/files")
    assert response.status_code == 200

""" def test_403_dir():
    response = requests.get(BASE_URL + "cgi-bin/files/403/")
    assert response.status_code == 403
    assert "Index of cgi-bin/files/" in response.text

def test_403_file():
    response = requests.get(BASE_URL + "cgi-bin/files/403.no")
    assert response.status_code == 200
    assert "Index of cgi-bin/files/" in response.text """