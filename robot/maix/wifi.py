from maix import network, time
import platform
import os.path


def is_ssid_visible(ssid):
    w = network.wifi.Wifi()
    w.start_scan()
    time.sleep(5)
    w.stop_scan()

    aps = w.get_scan_result()
    for ap in aps:
        if ap.ssid_str() == ssid:
            return True

    return False


def init():
    w = network.wifi.Wifi()

    if w.is_connected() and not w.is_ap_mode():
        print("Already connected IP:", w.get_ip())
        return

    if os.path.exists("/boot/wifi.ssid.sta") and os.path.exists("/boot/wifi.pass.sta"):
        f = open('/boot/wifi.ssid.sta', 'r')
        ssid = f.read()

        f = open('/boot/wifi.pass.sta', 'r')
        password = f.read()

        if is_ssid_visible(ssid):

            if w.is_ap_mode():
                w.stop_ap()

            w.connect(ssid, password, wait=True, timeout=60)

            if w.is_connected():
                print(f"Connected to {ssid} IP={w.get_ip()}")
                return

    w.disconnect()

    ssid = platform.node()

    f = open('/boot/wifi.pass.ap', 'r')
    password = f.read()

    w.start_ap(ssid, password)

    if w.is_connected():
        print(f"AP started {ssid} IP={w.get_ip()}")
