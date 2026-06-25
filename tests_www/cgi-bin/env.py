#!/usr/bin/env python3
import os
print("Content-Type: text/html\r\n")
print("\r\n")
print("<html><body><h1>CGI ENV</h1>")
for key, value in sorted(os.environ.items()):
    print(f"<p><b>{key}</b> = {value}</p>")
print("</body></html>")
