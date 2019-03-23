# coding: utf-8
from flask import Flask
from coapthon.client.helperclient import HelperClient

app = Flask(__name__)

hostArduino = "192.168.43.2"
port = 5683

@app.route("/get_values")
def get_values(path):
        client = HelperClient(server=(hostArduino, port))
        response = client.get(path)
        client.stop()
        return response._payload

@app.route("/")
def main():
	return get_values("light") + "</br>" + get_values("temp") + "</br>" + get_values("hum") + "</br>" + get_values("apptemp")

if __name__ == "__main__":
  app.run(host="192.168.43.212")
