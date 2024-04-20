## Pulmonary Function Testing (PFT)
This project is about personal PFT device.
For now, device measures rafly expiratory volume and FEV1/FVC.
Displays chart and numbers on a built-in screen which gives me ideas how bad my asthma currently is.

## Hardware
1. [LILYGO® T-Display-S3 Touch Edition ESP32-S3](https://www.aliexpress.com/item/1005004928000937.html)
2. [Waveshare 9523 distance sensor](https://botland.store/distance-sensors/4459-distance-sensor-reflective-33v5v-30mm-waveshare-9523-5904422373986.html)
3. Metal shaft. I used 30mm shaft with 1 mm diameter simply because I got one.
4. [Li-Po battery](https://allegro.pl/oferta/bateria-akumulator-li-polymer-3-7v-250mah-pcm-12200080940). Could be any with 3.7v and 2 wires which fits sizes (Optional)
5. Micro switch to disconnect battery (Optional)
6. 3d printed parts

## 3d model
[3d model](./model/) can be found here
You can use already preapared stl files.
Or you can open in [Blender](https://www.blender.org/) pikfluometer.blend if you need to change something.

## Software
Use Arduino to compile pft.ino and upload firmware

## References
* [Pulmonary Function Testing (PFT) made simple](https://medicine.uiowa.edu/iowaprotocols/pulmonary-function-testing-pft-made-simple)
* [Understanding Spirometry - Normal, Obstructive vs Restrictive](https://www.youtube.com/watch?v=YwcNbVnHNAo)
* [MedCram - Medical Lectures Explained CLEARLY](https://www.youtube.com/watch?v=EmRgAGLEY8Y)
* [Spirometry Interpretation | Lung Function Tests | OSCE Guide | UKMLA | CPSA](https://www.youtube.com/watch?v=qW8ujyrLZxk)
