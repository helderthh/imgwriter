import base64
from io import BytesIO
import json

import requests
from PIL import Image


image = Image.open("orig-image.png")
buffered = BytesIO()
image.save(buffered, format="PNG")
encoded_string = base64.b64encode(buffered.getvalue())
img_str = encoded_string.decode("utf-8")

data = {
    "x_pos": 280,
    "y_pos": 490,
    "image": img_str,
    "text": "Hello!"
}
resp = requests.post('http://127.0.0.1:8009', json=data)
if not resp.ok:
    print(resp.content)
else:
    print('saving')
    data = resp.json()
    im = Image.open(BytesIO(base64.b64decode(data.get('image'))))
    im.save('result.jpg', format='PNG')

