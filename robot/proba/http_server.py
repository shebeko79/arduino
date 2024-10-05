from maix import camera, time, app, image
from flask import Flask, request, send_file
import io

app = Flask(__name__)

cam = camera.Camera(640, 480)

@app.route("/", methods=["GET", "POST"])
def root():
    print("========")
    print(request.remote_addr)
    print(f'headers:\n{request.headers}')
    print(f'data: {request.data}')
    print("========")
    return 'hello world<br><img src="/img" style="background-color: black">'

@app.route("/img")
def img():
    img = cam.read()

    fp = io.BytesIO()
    fp.write(img.to_jpeg().to_bytes())
    fp.seek(0)

    return send_file(fp,mimetype="image/jpeg")

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=8000)