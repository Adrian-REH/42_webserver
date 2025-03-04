
#CMD: pytest test_api_cookie.py -v

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
    assert "Set-Cookie" in response.headers  # Verifica que haya una cookie de sesión

def test_cookie_session_home():
    # Datos de inicio de sesión
    data = {"username": "admin", "password": "admin"}

    # Realizar la solicitud de inicio de sesión
    response = requests.post(BASE_URL + "cgi-bin/login.py", data=data)

    assert response.status_code == 200
    assert "Set-Cookie" in response.headers

    session_cookie = response.cookies.get('session')
    session_id_cookie = response.cookies.get('session_id')  # Nombre de la cookie de sesión puede variar

    assert session_cookie is not None, "No se recibió cookie de sesión"
    assert session_id_cookie is not None, "No se recibió cookie de sesión"

    print(session_cookie)
    print(session_id_cookie)
    cookies = {'session': session_cookie, 'session_id': session_id_cookie}
    response = requests.post(BASE_URL + "cgi-bin/home.py", data=data, cookies=cookies)
    assert response.status_code == 200
    assert "Upload a file" in response.text

def test_cookie_session_upload_file():
    # Datos de inicio de sesión
    data = {"username": "admin", "password": "admin"}

    # Realizar la solicitud de inicio de sesión
    response = requests.post(BASE_URL + "cgi-bin/login.py", data=data)

    assert response.status_code == 200
    assert "Set-Cookie" in response.headers

    session_cookie = response.cookies.get('session')
    session_id_cookie = response.cookies.get('session_id')  # Nombre de la cookie de sesión puede variar

    assert session_cookie is not None, "No se recibió cookie de sesión"
    assert session_id_cookie is not None, "No se recibió cookie de sesión"
    real = os.path.abspath(__file__)
    dir_path = os.path.dirname(real)
    filename = "valores.txt"
    path_f = os.path.join(dir_path, "files", filename)
    with open(path_f, 'w') as file:
        file.write('Test automatico: verificando si se suben archivos a WebServer')

    # Crear un MultipartEncoder, que generará automáticamente el boundary
    with open(path_f, 'rb') as file:
        multipart_data = MultipartEncoder(fields={'file': ('valores.txt', file, 'text/plain'),})
        headers = {
            'Content-Type': multipart_data.content_type,
            'Cookie': f'session={session_cookie}; session_id={session_id_cookie}',
        }
        cookies = {'session': session_cookie, 'session_id': session_id_cookie}
        response = requests.post(BASE_URL + "cgi-bin/upload_file.py", data=multipart_data, headers=headers)
        assert response.status_code == 200
        print(file.name)
        assert filename in response.text
        os.remove(path_f)

def test_cookie_session_delete_file():
    # Datos de inicio de sesión
    data = {"username": "admin", "password": "admin"}

    # Realizar la solicitud de inicio de sesión
    response = requests.post(BASE_URL + "cgi-bin/login.py", data=data)

    assert response.status_code == 200
    assert "Set-Cookie" in response.headers

    session_cookie = response.cookies.get('session')
    session_id_cookie = response.cookies.get('session_id')  # Nombre de la cookie de sesión puede variar

    assert session_cookie is not None, "No se recibió cookie de sesión"
    assert session_id_cookie is not None, "No se recibió cookie de sesión"
    real = os.path.abspath(__file__)
    dir_path = os.path.dirname(real)
    filename = "valores.txt"
    path_f = os.path.join(dir_path, "files", filename)
    with open(path_f, 'w') as file:
        file.write('Test automatico: verificando si se suben archivos a WebServer')

    # Crear un MultipartEncoder, que generará automáticamente el boundary
    with open(path_f, 'rb') as file:
        multipart_data = MultipartEncoder(fields={'file': ('valores.txt', file, 'text/plain'),})
        headers = {
            'Content-Type': multipart_data.content_type,
            'Cookie': f'session={session_cookie}; session_id={session_id_cookie}',
        }
        cookies = {'session': session_cookie, 'session_id': session_id_cookie}
        response = requests.post(BASE_URL + "cgi-bin/upload_file.py", data=multipart_data, headers=headers)
        assert response.status_code == 200
        print(file.name)
        assert filename in response.text
        os.remove(path_f)
        headers = {
            'Content-Type': 'application/json',
            'Cookie': f'session={session_cookie}; session_id={session_id_cookie}',
        }
        data = {'fileId':filename}
        response = requests.delete(BASE_URL + "cgi-bin/delete_file.py", json=data, headers=headers)
        assert response.status_code == 200
        assert "Archivo eliminado correctamente" in response.text


