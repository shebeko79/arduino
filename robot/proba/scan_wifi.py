from maix import network, time

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

w = network.wifi.Wifi()
f = open('/boot/wifi.ssid.sta', 'r')
ssid = f.read()
f = open('/boot/wifi.pass.sta', 'r')
password = f.read()

print(f'{ssid} {password} {is_ssid_visible(ssid)}')
