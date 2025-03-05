
#CMD: pytest test_api.py -v

import requests
import os
from requests_toolbelt.multipart.encoder import MultipartEncoder

BASE_URL = "http://localhost:8080/"

def test_uri_file_extension():
    response = requests.get(BASE_URL + "cgi-bin/login.py", data=data)
    assert response.status_code == 200
    assert "Upload a file" in response.text
