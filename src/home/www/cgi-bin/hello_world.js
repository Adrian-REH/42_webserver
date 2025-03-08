#!/usr/bin/env node

// Cabeceras para indicar que la salida será en formato HTML
console.log("Content-Type: text/html");
console.log("");  // Línea en blanco que indica el final de las cabeceras

// Respuesta que el servidor CGI enviará
console.log("<html><body><h1>Hello, World from CGI!</h1></body></html>");
