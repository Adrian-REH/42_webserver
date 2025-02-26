#!/bin/bash

chmod 777 /usr/lib/cgi-bin/*.py
chmod 755 /usr/lib/cgi-bin

chown -R www-data:www-data /usr/lib/cgi-bin

fcgiwrap -s tcp:0.0.0.0:9000 &

nginx -g 'daemon off;'