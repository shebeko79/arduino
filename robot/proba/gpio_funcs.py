from maix.peripheral import pinmap

pins = pinmap.get_pins()

print("All pins of MaixCAM:")
print(pins)

for p in pins:
    print(f"GPIO {p} pin functions:")
    f = pinmap.get_pin_functions(p)
    print(f)

#print(f"Set GPIO A28 to {f[0]} function")
#pinmap.set_pin_function("A28", f[0])