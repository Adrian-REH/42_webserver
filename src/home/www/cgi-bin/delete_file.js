const readline = require('readline');
const path = require('path');
const fs = require('fs');

function verifySession() {
	try {
		const cookieHeader = process.env.HTTP_COOKIE || '';
		if (!cookieHeader) {
			return false;
		}

		// Buscamos la cookie 'session' en las cookies
		const sessionCookie = cookieHeader.split('; ').find(cookie => cookie.startsWith('session='));

		if (sessionCookie) {
		const sessionValue = sessionCookie.split('=')[1];
		return sessionValue === 'valid';
		}

		return false;
	} catch (e) {
		console.error("Error procesando cookies:", e);
		return false;
	}
};

function parser_body() {
	return new Promise((resolve, reject) => {
	  try {
		// Crea la interfaz de lectura
		const rl = readline.createInterface({
		  input: process.stdin,
		  output: process.stdout
		});
  
		const content_length = 1000;  // Longitud de contenido deseada
		let body = '';
		let size = 0;
  
		// Leemos línea por línea
		rl.on('line', (input) => {
		  body += input;  // Agregamos el input a body
		  size += input.length;  // Actualizamos el tamaño de las líneas leídas
  
		  // Si alcanzamos el contenido total o más, cerramos
		  if (size >= content_length) {
			rl.close();
			console.log('Contenido leído hasta el límite:');
			console.log(body);  // Imprime el contenido leído
		  }
		});
  
		// Cuando el input se haya completado
		rl.on('close', () => {
		  try {
			// Parseamos el JSON cuando se termina de leer todo el contenido
			var personObject = JSON.parse(body);
			resolve(personObject.fileId);  // Resolvemos la promesa con fileId
		  } catch (e) {
			reject('Error al procesar JSON: ' + e.message);  // Rechazamos si hay un error al parsear el JSON
		  }
		});
  
	  } catch (e) {
		reject('Error en la lectura: ' + e.message);
	  }
	});
  }


function getFilePath() {
	try {
	  // Obtener la ruta absoluta del archivo actual
	  const real = __filename;
  
	  // Obtener el directorio raíz
	  const rootDir = path.join(path.dirname(real), '..');
	var filename;
	  // Obtener el nombre del archivo de alguna función (similar a parser_body en Python)
	  parser_body()
	  .then(fileId => {
		console.log('File ID:', fileId);  // Imprime el fileId extraído
		filename = fileId;	  // Unir las partes de la ruta del archivo
		const pathF = path.join(rootDir, 'files', filename);
		console.log('Path del archivo:', pathF);
		fs.unlinkSync(pathF);
  
	  })
	  .catch(error => {
		console.error(error);  // Imprime cualquier error que ocurra
		process.exit(1);  // Sale con error si algo falla
	  });
	  return filename;
	} catch (e) {
	  console.error('Error:', e);
	}
};



var session = verifySession();
if (!session) {
	console.log(`Set-Cookie: session=${session}`);
	console.log("Content-Type: text/html\r\n\r\n");
	console.log("<h1>Error: Sesion invalida</h1>");
	process.exit(13);
};

getFilePath();

