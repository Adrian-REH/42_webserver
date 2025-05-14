
#CMD: pytest test_api.py -v

import requests
import os
from requests_toolbelt.multipart.encoder import MultipartEncoder

BASE_URL = "http://localhost:8081/"


def test_get_autoindex():
    response = requests.get(BASE_URL + "cgi-bin/")
    assert response.status_code == 200
    assert "Index of cgi-bin/" in response.text

def test_download_file():
    response = requests.get(BASE_URL + "cgi-bin/login.py")
    assert response.status_code == 200
    assert "Content-Disposition" in response.headers
    assert "attachment" in response.headers["Content-Disposition"].lower()
    assert "login.py" in response.headers["Content-Disposition"]
    content = response.content
    assert content, "El archivo descargado está vacío"

def test_access_dir():
    response = requests.get(BASE_URL + "cgi-bin/files/")
    assert response.status_code == 200
    assert "Index of cgi-bin/files" in response.text

def test_download_dir():
    response = requests.get(BASE_URL + "cgi-bin/files")
    assert response.status_code == 404

""" def test_403_dir():
    response = requests.get(BASE_URL + "cgi-bin/files/403/")
    assert response.status_code == 403
    assert "Index of cgi-bin/files/" in response.text

def test_403_file():
    response = requests.get(BASE_URL + "cgi-bin/files/403.no")
    assert response.status_code == 200
    assert "Index of cgi-bin/files/" in response.text """