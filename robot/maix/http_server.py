from maix import image
from http.server import BaseHTTPRequestHandler, HTTPServer
import threading
import states
import json
import track_utils
import tracker


class Server(HTTPServer):
    def __init__(self, address, request_handler):
        super().__init__(address, request_handler)


server: Server = None
last_img: image.Image = None
main_th_lock = None
main_th_condition: threading.Condition = None
delayed_request = None
delay_call = None
delay_result = None


class RequestHandler(BaseHTTPRequestHandler):
    def __init__(self, request, client_address, server_class):
        self.server_class = server_class
        super().__init__(request, client_address, server_class)
        self.req_vars = {}

    def process_URL(self):
        if self.path == "/":
            self.root()
        elif self.path == "/state":
            self.state()
        elif self.path == "/img" or self.path.startswith("/img?"):
            self.img()
        elif self.path == "/click":
            self.click()
        elif self.path == "/click_point":
            self.click_point()
        elif self.path == "/sel_rect":
            self.sel_rect()
        else:
            self.send_response(404)
            self.send_header("Content-Length", "0")
            self.end_headers()

    def parse_POST(self):

        ctype = self.headers['content-type']
        length = int(self.headers['content-length'])
        postvars = self.rfile.read(length)
        if ctype == 'application/json':
            postvars = json.loads(postvars)
        return postvars

    def do_GET(self):
        self.process_URL()

    def do_POST(self):
        self.req_vars = self.parse_POST()
        self.process_URL()

    def log_message(self, fmt: str, *args) -> None:
        pass

    def send_html(self, success_response=None):
        if success_response is None:
            self.send_error(500)
            self.send_header("Content-Length", "0")
            self.end_headers()
            return

        self.send_response(200)
        self.send_header("Content-type", "text/html")
        self.send_header("Content-Length", str(len(success_response)))
        self.end_headers()
        self.wfile.write(str(success_response).encode('utf8'))

    def send_json(self, success_response=None):
        if success_response is None:
            self.send_error(500)
            self.send_header("Content-Length", "0")
            self.end_headers()
            return

        self.send_response(200)
        self.send_header("Content-type", "application/json")
        self.send_header("Content-Length", str(len(success_response)))
        self.end_headers()
        self.wfile.write(str(success_response).encode('utf8'))

    def send_img(self, success_response=None):
        if success_response is None:
            self.send_error(500)
            self.send_header("Content-Length", "0")
            self.end_headers()
            return

        self.send_response(200)
        self.send_header("Content-type", "image/jpeg")
        self.send_header("Content-Length", str(len(success_response)))
        self.end_headers()
        self.wfile.write(success_response)

    def root(self):
        file = open(track_utils.BASE_PATH + "/assets/index.html", 'r')
        file_content = file.read()
        self.send_html(file_content)

    def state(self):
        st = call_in_main_thread(get_current_state)

        d = '{}'
        if st:
            d = json.dumps(st)
        self.send_json(d)

    def img(self):
        bts = call_in_main_thread(get_img_bytes)
        self.send_img(bts)

    def click(self):
        state_name = self.req_vars['state_name']
        caption = self.req_vars['caption']

        st = call_in_main_thread(switch_state, state_name, caption)

        d = '{}'
        if st:
            d = json.dumps(st)
        self.send_json(d)

    def click_point(self):
        state_name = self.req_vars['state_name']
        x = int(self.req_vars['x'])
        y = int(self.req_vars['y'])

        st = call_in_main_thread(click_point, state_name, x, y)

        d = '{}'
        if st:
            d = json.dumps(st)
        self.send_json(d)

    def sel_rect(self):
        state_name = self.req_vars['state_name']
        x1 = int(self.req_vars['x1'])
        y1 = int(self.req_vars['y1'])
        x2 = int(self.req_vars['x2'])
        y2 = int(self.req_vars['y2'])

        st = call_in_main_thread(sel_rect, state_name, x1, y1, x2, y2)

        d = '{}'
        if st:
            d = json.dumps(st)
        self.send_json(d)


def thread_task():
    global server

    server_address = ("0.0.0.0", 80)
    server = Server(server_address, RequestHandler)
    server.serve_forever()


def init():
    global main_th_lock
    global main_th_condition

    main_th_lock = threading.Lock()
    main_th_condition = threading.Condition(lock=main_th_lock)
    t1 = threading.Thread(target=thread_task)
    t1.start()


def shutdown():
    global server

    server.shutdown()



def call_in_main_thread(func, *args, **kwargs):
    global delay_call
    global delay_result
    global main_th_condition

    with main_th_condition:
        delay_call = (func, args, kwargs)
        while delay_call is not None:
            main_th_condition.wait()

        res = delay_result
        delay_result = None

    return res


def process():
    global delay_call
    global delay_result
    global main_th_condition

    with main_th_condition:
        if delay_call:
            func, args, kwargs = delay_call
            delay_result = func(*args, **kwargs)
            delay_call = None
            main_th_condition.notify()


def get_img_bytes():
    global last_img

    bts = None

    if last_img is None:
        return bts

    http_img = last_img.copy()
    tracker.draw_trackers(http_img)

    if http_img:
        jpg = http_img.to_jpeg()
        if jpg:
            bts = jpg.to_bytes()

    return bts


def get_current_state():
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

    return cur_state


def switch_state(state_name, caption):
    st = states.current_state
    if st is None or state_name != st.state_name:
        return get_current_state()

    btn = st.button_by_name(caption)
    if btn is not None and btn.enabled:
        st.on_click_button(btn)

    return get_current_state()


def click_point(state_name, x, y):
    st = states.current_state
    if st is None or state_name != st.state_name:
        return get_current_state()

    if st.accept_click:
        st.on_click([x, y])

    return get_current_state()


def sel_rect(state_name, x1, y1, x2, y2):
    st = states.current_state
    if st is None or state_name != st.state_name:
        return get_current_state()

    if st.accept_rectangle and last_img:
        rc = track_utils.make_rect([x1, y1], [x2, y2])
        st.on_rectangle(last_img, rc)

    return get_current_state()
