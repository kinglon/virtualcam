import sys
import os
from moviepy.editor import VideoFileClip
import time

# Get the directory of the running script
script_dir = os.path.dirname(os.path.abspath(__file__))

# Append the directory to sys.path
sys.path.append(script_dir)
import softcam

def main():
    video = VideoFileClip(r'C:\Users\zengxiangbin\Downloads\VID_20231116_210820.mp4')
    cam = softcam.camera(video.w, video.h, 30)
    interval = 1 / 30

    # Here, you can wait for an application to connect to this camera.
    while not cam.wait_for_connection(timeout=1):
        pass

    # Iterate over the frames
    for frame in video.iter_frames():
        # Convert RGB to BGR
        bgr_frame = frame[:, :, ::-1]
        cam.send_frame(bgr_frame)
        time.sleep(interval)

    # Close the video file
    video.close()

if __name__ == '__main__':
    main()
