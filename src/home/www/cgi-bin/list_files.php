<?php
	function verify_session() {
		try {
			// Verificamos si existe la cookie 'session' en $_COOKIE
			if (isset($_COOKIE['session'])) {
				// Comprobamos si su valor es 'valid'
				if ($_COOKIE['session'] == 'valid') {
					return true; // Sesión válida
				}
			}
			return false; // No existe la cookie o su valor no es 'valid'
		} catch (Exception $e) {
			// Si ocurre un error, lo mostramos en el log o en la consola
			error_log("Error procesando cookies: " . $e->getMessage());
			return false; // Devolvemos false en caso de error
		}
	}

	// Uso de la función
	if (verify_session()) {
		echo "Content-Type: text/html\r\n\r\n";
		echo "Invalid session";
		exit(13);
	}
?>

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
				border-radius: 5px;home
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
			<h1>List Files </h1>
			<hr>
				<?php
				$real = __FILE__;
				$dir = dirname($real);
				$root_dir = dirname($dir);
				$path_f = $root_dir . "/files"; // Ajusta la ruta del directorio de archivos según sea necesario

				// Verifica si el directorio existe
				if (is_dir($path_f)) {
					// Obtiene la lista de archivos en el directorio
					$file_names = array_diff(scandir($path_f), array('.', '..')); // Elimina "." y ".."

					// Si hay archivos, genera el HTML para la lista
					echo '<div class="uploaded-files-card">';
					echo '<h2>Uploaded Files</h2>';
					echo '<ul id="fileList">';

					foreach ($file_names as $filename) {
						// Por cada archivo, genera un elemento <li> con el nombre del archivo
						echo '<li>' . htmlspecialchars($filename) . '<button onclick="deleteFile(\''. htmlspecialchars($filename) . '\')">Delete</button>' . "</li>\n";
					}

					echo '</ul>';
					echo '</div>';
				} else {
					// Si no se encuentra el directorio o no tiene permisos, muestra un mensaje de error
					echo '<div class="uploaded-files-card">';
					echo '<h2>Uploaded Files</h2>';
					echo '<p>No se pudo acceder al directorio de archivos.</p>';
					echo '</div>';
				}
				?>
		</div>
	</body>
	<script>
		function deleteFile(fileName) {
			fetch('http://localhost:8080/delete_file.js', {
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

