#!/bin/bash
ffmpeg -i polet_navigatora.mp4 -f rtsp -c copy rtsp://0.0.0.0:8554/live

ffmpeg -i polet_navigatora.mp4 -map 0:1 -f rtp -c copy rtp://192.168.33.7:8554/live
