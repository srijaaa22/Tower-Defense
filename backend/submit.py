import json
import requests

data = json.load(open("../stats.json"))
data["player"] = input("Enter your name: ")

response = requests.post("http://localhost:8000/score", json=data)
print(response.json())