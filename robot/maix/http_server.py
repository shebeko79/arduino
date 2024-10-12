from maix import camera, time, app, image
from flask import Flask, request, send_file
import io
import threading
import states
import json

server: Flask = Flask(__name__)
last_img: image.Image = None
main_th_lock = None
web_lock = None
current_state = {}


def thread_task(lc):
    global web_lock

    web_lock = lc
    server.run(host="0.0.0.0", port=8000)


def init():
    global server
    global main_th_lock

    main_th_lock = threading.Lock()
    t1 = threading.Thread(target=thread_task, args=(main_th_lock,))
    t1.start()


def set_img(im: image.Image):
    global last_img

    main_th_lock.acquire()
    last_img = im
    main_th_lock.release()


def set_state():
    global current_state

    st = states.current_state
    cur_state = {}

    if st is not None:
        cur_state['state_name'] = st.state_name
        cur_state['accept_click'] = st.accept_click
        cur_state['accept_rectangle'] = st.accept_rectangle

        buttons = []
        for b in st.buttons:
            buttons.append({'caption': b.caption, 'enabled': b.enabled})
        cur_state['buttons'] = buttons

    main_th_lock.acquire()
    current_state = cur_state
    main_th_lock.release()



@server.route("/", methods=["GET", "POST"])
def root():
    file = open("assets/index.html", 'r')
    file_content = file.read()
    return file_content


@server.route("/img")
def img():
    global last_img

    bts = None

    web_lock.acquire()
    if last_img:
        bts = last_img.to_jpeg().to_bytes()
    web_lock.release()

    if bts is None:
        return ""

    fp = io.BytesIO()
    fp.write(bts)
    fp.seek(0)

    return send_file(fp,mimetype="image/jpeg")


@server.route("/state")
def state():
    global current_state

    d = '{}'

    web_lock.acquire()
    if current_state:
        d = json.dumps(current_state)
    web_lock.release()

    return d
