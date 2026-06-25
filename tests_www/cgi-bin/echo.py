#!/usr/bin/env python3
import sys
body = sys.stdin.read()
print("Content-Type: text/plain\r\n")
print("\r\n")
print("CGI echo OK")
print(body)
