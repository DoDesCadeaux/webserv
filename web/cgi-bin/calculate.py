#!/usr/bin/env python3
import cgi

print("Content-Type: text/html")  # En-tête HTTP
print()                          # Ligne vide, fin des en-têtes

arguments = cgi.FieldStorage()
if "a" in arguments and "b" in arguments:
    try:
        a = float(arguments["a"].value)
        b = float(arguments["b"].value)
        resultat = a + b
        print(f"<html><body><p>{a} + {b} = {resultat}</p></body></html>")
    except ValueError:
        print("<html><body><p>Erreur : Les valeurs doivent être des nombres.</p></body></html>")
else:
    print("<html><body><p>Erreur : Paramètres 'a' et 'b' requis.</p></body></html>")
