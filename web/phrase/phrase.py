#!/usr/bin/env python3
import cgi
import json
import urllib.request
import os


def caesar_cipher(text, shift):
    encrypted_text = ""
    for char in text:
        if char.isalpha():
            ascii_offset = 65 if char.isupper() else 97
            encrypted_text += chr((ord(char) - ascii_offset + shift) % 26 + ascii_offset)
        else:
            encrypted_text += char
    return encrypted_text

def main():
    query_string = os.environ.get('QUERY_STRING', '')
    arguments = urllib.parse.parse_qs(query_string)
    if "phrase" in arguments:
        try:
            sentence = arguments["phrase"][0]
            encrypted_sentence = caesar_cipher(sentence, 3)  # Use a shift of 3 for Caesar cipher
        except ValueError:
            print("<html><body><p style=\"color: green; font-size: 25px;\">Error: The value must be a number.</p></body></html>")
            return
        print(f"<html><body><p style=\"color: green; font-size: 25px;\">{encrypted_sentence}</p></body></html>")
    else:
        print("<html><body><p style=\"color: green; font-size: 25px;\">Error: Parameter 'phrase' is required.</p></body></html>")

if __name__ == "__main__":
    main()
