# auth.pyimport cgi
import cgi
import http.cookies
import os
import json

def list_files():
	real = os.path.abspath(__file__)
	root_dir = os.path.join(os.path.dirname(real), "..")
	path_f = os.path.join(root_dir, "files")
	filenames = []
	file_names = [f for f in os.listdir(path_f) if os.path.isfile(os.path.join(path_f, f))]
	for filename in file_names:
		filenames.append({"fileId": filename})
	print(json.dumps({"files": filenames, "path_f": path_f}, indent=2))

if __name__ == "__main__":
	if os.environ.get("REQUEST_METHOD") != "GET":
		sys.exit(92)
	print("Content-Type: application/json\r\n")
	list_files()

