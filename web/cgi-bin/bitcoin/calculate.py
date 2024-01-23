#!/usr/bin/env python3
import cgi
import json
import urllib.request


def main():
    print("Content-Type: text/html")  # HTTP header
    print()  # End of headers

    arguments = cgi.FieldStorage()
    if "btc" in arguments:
        try:
            btc = float(arguments["btc"].value)
        except ValueError:
            print("<html><body><p style=\"color: red; font-size: 25px;\">Error: The value must be a number.</p></body></html>")
            return
        try:
            with urllib.request.urlopen("https://api.coindesk.com/v1/bpi/currentprice.json") as url:
                response = json.loads(url.read().decode())
        except urllib.error.URLError:
            print("<html><body><p style=\"color: red; font-size: 25px;\">Error: Unable to fetch data from CoinDesk API.</p></body></html>")
            return
        rate = response["bpi"]["USD"]["rate"]
        float_rate = float(rate.replace(',', ''))
        amount = float_rate * btc
        print(f"<html><body><p style=\"color: red; font-size: 25px;\">{btc} BTC is now valued at ${amount:,.4f}</p></body></html>")
    else:
        print("<html><body><p style=\"color: red; font-size: 25px;\">Error: Parameter 'btc' is required.</p></body></html>")


if __name__ == "__main__":
    main()
