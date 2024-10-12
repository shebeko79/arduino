from maix import image
from flask import Flask, request, send_file, jsonify
import io
import threading
import states
import json
import track_utils


server: Flask = Flask(__name__)
last_img: image.Image = None
main_th_lock = None
web_lock = None
current_state = {}
delayed_request = None


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


def process_request():
    global delayed_request

    main_th_lock.acquire()
    r = delayed_request
    delayed_request = None
    main_th_lock.release()

    if r is None:
        return

    print(r)

    st = states.current_state
    print(st)
    if st is None or r['state_name'] != st.state_name:
        return

    print(r['action'])

    if r['action'] == 'click':
        btn = st.button_by_name(r['caption'])
        print(btn)
        if btn is not None and btn.enabled:
            st.on_click_button(btn)

    elif r['action'] == 'click_point':
        if st.accept_click:
            st.on_click([r['x'], r['y']])

    elif r['action'] == 'sel_rect':
        if st.accept_rectangle and last_img:
            rc = track_utils.make_rect([r['x1'], r['y1']], [r['x2'], r['y2']])
            st.on_rectangle(last_img, rc)

    r = None


@server.route("/", methods=["GET", "POST"])
def root():
    file = open("assets/index.html", 'r')
    file_content = file.read()
    return file_content


@server.route("/img")
def img():

    bts = None

    web_lock.acquire()
    if last_img:
        jpg = last_img.to_jpeg()
        if jpg:
            bts = jpg.to_bytes()
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


@server.route('/click', methods=['POST'])
def click_handler():
    global delayed_request

    data = request.get_json()
    state_name = data.get('state_name')
    caption = data.get('caption')

    r = {'action': 'click', 'state_name': state_name, 'caption': caption}

    web_lock.acquire()
    delayed_request = r
    web_lock.release()

    return jsonify({"status": "success"}), 200


@server.route('/click_point', methods=['POST'])
def click_point_handler():
    global delayed_request

    data = request.get_json()
    state_name = data.get('state_name')
    x = data.get('x')
    y = data.get('y')

    r = {'action': 'click_point', 'state_name': state_name, 'x': x, 'y': y}

    web_lock.acquire()
    delayed_request = r
    web_lock.release()

    return jsonify({"status": "success"}), 200


@server.route('/sel_rect', methods=['POST'])
def sel_rect_handler():
    global delayed_request

    data = request.get_json()
    state_name = data.get('state_name')
    x1 = data.get('x1')
    y1 = data.get('y1')
    x2 = data.get('x2')
    y2 = data.get('y2')

    r = {'action': 'sel_rect', 'state_name': state_name, 'x1': x1, 'y1': y1, 'x2': x2, 'y2': y2}

    web_lock.acquire()
    delayed_request = r
    web_lock.release()

    return jsonify({"status": "success"}), 200
