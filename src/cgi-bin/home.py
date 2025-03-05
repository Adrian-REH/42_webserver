# home.py
import cgi
import cgi
import http.cookies
import os
import sys
from auth import verify_session

def list_files():
    real = os.path.abspath(__file__)
    dir_path = os.path.dirname(real)
    
    path_f = os.path.join(dir_path, "files")
    file_names = [f for f in os.listdir(path_f) if os.path.isfile(os.path.join(path_f, f))]
    for filename in file_names:
        print(f"""
            <li>
                {filename}
                <button onclick="deleteFile('{filename}')">Delete</button>
            </li>
        """)

def home():
    print("Content-type: text/html\n")
    print("""
    <!DOCTYPE html>
    <html lang="es">
    <head>
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <title>Página de Inicio</title>
        <style>
            body {
                font-family: 'Arial', sans-serif;
                background-color: #f4f4f9;
                margin: 0;
                padding: 0;
                display: flex;
                justify-content: center;
                align-items: center;
                height: 100vh;
            }

            .container {
                background-color: #ffffff;
                border-radius: 8px;
                box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1);
                padding: 30px;
                width: 100%;
                max-width: 400px;
            }

            h1 {
                text-align: center;
                color: #333333;
            }

            label {
                font-size: 1rem;
                margin-bottom: 10px;
                display: inline-block;
                color: #555555;
            }
            input[type="submit"] {
                background-color: #007BFF;
                color: white;
                border: none;
                padding: 10px 20px;
                border-radius: 4px;
                font-size: 1rem;
                cursor: pointer;
                width: 100%;
            }

            input[type="submit"]:hover {
                background-color: #0056b3;
            }

            .message {
                text-align: center;
                font-size: 1.2rem;
                color: #333333;
                margin-top: 20px;
            }

            .message.error {
                color: #dc3545;
            }

            .message.success {
                color: #28a745;
            }
            #myFile {
                display: none;
            }
            .custom-file-button {
                background-color: #3d9fd4;
                color: white;
                padding: 10px 20px;
                border: none;
                cursor: pointer;
                border-radius: 5px;
                font-size: 16px;
                display: inline-block;
            }
            .custom-file-button:hover {
                background-color: #1d7caf;
            }
            #fileNameDisplay {
                display: inline-block;
                margin-left: 10px;
            }
            p {
                display: inline-block;
            }
            .signout-button {
                position: absolute;
                top: 10px; /* Ajusta la distancia desde la parte superior */
                right: 10px; /* Mueve el botón a la derecha */
                padding: 10px 15px;
                background-color: red;
                color: white;
                text-decoration: none;
                border-radius: 5px;
            }
                /* Estilos para el contenedor de los archivos subidos */
            .uploaded-files-card {
                border: 1px solid #ddd;
                padding: 20px;
                margin-top: 20px;
                border-radius: 8px;
                box-shadow: 0 2px 5px rgba(0, 0, 0, 0.1);
                background-color: #f9f9f9;
            }

            .uploaded-files-card h2 {
                margin-bottom: 15px;
            }

            .uploaded-files-card ul {
                list-style-type: none;
                padding: 0;
            }

            .uploaded-files-card ul li {
                background-color: #fff;
                border: 1px solid #ddd;
                margin-bottom: 10px;
                padding: 10px;
                border-radius: 5px;
                display: flex;
                justify-content: space-between;
                align-items: center;
            }

            /* Estilos para el botón de borrar */
            .uploaded-files-card ul li button {
                background-color: red;
                color: white;
                border: none;
                padding: 5px 10px;
                cursor: pointer;
                border-radius: 5px;
            }

            .uploaded-files-card ul li button:hover {
                background-color: darkred;
            }
        </style>
    </head>
    <body>
        <a href="signout.py" class="signout-button">Sign Out</a>
        <div class="container">
            <h1>Upload a file </h1>
            
            <form enctype="multipart/form-data" action="/cgi-bin/upload_file.py" method = "post" >
                <input type="file" id="myFile" name="file">
                <p>
                    <button type="button" class="custom-file-button" onclick="document.getElementById('myFile').click();">
                        Select File
                    </button>

                    <div id="fileNameDisplay">No file selected</div>
                </p>
                <input type="submit" value="Upload">
            </form>
            <hr>
            
            <div class="uploaded-files-card">
                <h2>Uploaded Files</h2>
                <ul id="fileList">""")
    list_files()
    print("""
                </ul>
            </div>
        </div>
    </body>
    <script>
        function deleteFile(fileName) {
            fetch('http://localhost:8080/cgi-bin/delete_file.py', {
                method: 'DELETE',
                headers: {
                    'Content-Type': 'application/json',
                },
                body: JSON.stringify({
                    fileId: fileName,
                }),
            })
            .then(response => {
                if (response.ok) {
                    alert('Archivo eliminado con éxito');
                    const fileList = document.getElementById('fileList');
                    const listItems = fileList.getElementsByTagName('li');
                    for (let i = 0; i < listItems.length; i++) {
                        if (listItems[i].textContent.includes(fileName)) {
                            listItems[i].remove(); // Eliminar el <li> correspondiente
                        }
                    }
                } else {
                    alert('Error al eliminar el archivo');
                }
            })
            .catch(error => {
                alert('Error en la solicitud: ' + error);
            });
        }
        document.getElementById('myFile').addEventListener('change', function() {
            const fileName = this.files[0] ? this.files[0].name : 'No file selected';
            document.getElementById('fileNameDisplay').textContent = fileName;
        });
    </script>
    </html>
    """)

def verify_home():
    mensaje = "INFO"
    mensaje_class = "info"
    if not verify_session():
        print(f"Set-Cookie: session=invalid;")
        print("Content-Type: text/html\r\n")
        print(f"""<!DOCTYPE html>
        <html lang="es">
        <head>
            <meta charset="UTF-8">
            <meta name="viewport" content="width=device-width, initial-scale=1.0">
            <title>Prohibido</title>
            <style>
                body {{
                    font-family: Arial, sans-serif;
                    display: flex;
                    justify-content: center;
                    align-items: center;
                    height: 100vh;
                    background-color: #f8f9fa;
                    margin: 0;
                }}
                .container {{
                    text-align: center;
                    background: white;
                    padding: 20px;
                    border-radius: 10px;
                    box-shadow: 0px 4px 6px rgba(0, 0, 0, 0.1);
                }}
                .message {{
                    font-size: 24px;
                    color: #333;
                    margin-bottom: 20px;
                }}
                .button {{
                    display: inline-block;
                    padding: 10px 20px;
                    font-size: 18px;
                    color: white;
                    background-color: #007bff;
                    text-decoration: none;
                    border-radius: 5px;
                    transition: background 0.3s ease;
                }}
                .button:hover {{
                    background-color: #0056b3;
                }}
            </style>
        </head>
        <body>
            <div class="container">
                <a class="button" href="/cgi-bin/login.py" >Volver a login</a>
            </div>
        </body>
        </html>""")
        sys.exit(112)
    home()


if __name__ == "__main__":
    if os.environ.get("REQUEST_METHOD") == "DELETE":
        sys.exit(92)
    verify_home()