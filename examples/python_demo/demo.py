import sys
import os
from moviepy.editor import VideoFileClip
import time
import cv2
import threading

# Get the directory of the running script
script_dir = os.path.dirname(os.path.abspath(__file__))

# Append the directory to sys.path
sys.path.append(script_dir)
import jericcam


def run_as_admin(cmdline=None, wait=True):
    import win32con
    import win32event
    import win32process
    from win32com.shell.shell import ShellExecuteEx
    from win32com.shell import shellcon
    cmd = '"%s"' % (cmdline[0],)
    params = " ".join(['"%s"' % (x,) for x in cmdline[1:]])
    showcmd = win32con.SW_SHOWNORMAL
    lpverb = 'runas'  # causes UAC elevation prompt.
    proc_info = ShellExecuteEx(nShow=showcmd,
                               fMask=shellcon.SEE_MASK_NOCLOSEPROCESS,
                               lpVerb=lpverb,
                               lpFile=cmd,
                               lpParameters=params)
    if wait:
        proc_handle = proc_info['hProcess']
        win32event.WaitForSingleObject(proc_handle, win32event.INFINITE)
        rc = win32process.GetExitCodeProcess(proc_handle)
    else:
        rc = None
    return rc


# camera_number 取值1-4
def install_camera(install, camera_number):
    cmdline = []
    executable_path = r'"{}\jericcam_installer.exe"'.format(script_dir)
    cmdline.append(executable_path)
    if install:
        cmdline.append('register')
    else:
        cmdline.append('unregister')
    cmdline.append(r'"{}\jericcam.dll"'.format(script_dir))
    if install:
        cmdline.append(str(camera_number))
    run_as_admin(cmdline, True)


def convert_frame_to_bgra(frame):
    """Converts an RGB or RGBA frame to BGRA format.

    Args:
        frame: A NumPy array representing the frame (RGB or RGBA).

    Returns:
        A NumPy array representing the frame in BGRA format.  Returns None if conversion fails.
    """
    try:
        # Check if the frame is RGB or RGBA
        if frame.shape[-1] == 3:  # RGB
            bgr_frame = cv2.cvtColor(frame, cv2.COLOR_RGB2BGR)
            bgra_frame = cv2.cvtColor(bgr_frame, cv2.COLOR_BGR2BGRA)

        elif frame.shape[-1] == 4:  # RGBA
            rgba_frame = frame.copy()  # avoid modifying the original frame
            bgra_frame = rgba_frame[:, :, [2, 1, 0, 3]]  # reorder channels
        else:
            print("Error: Unsupported frame format. Must be RGB or RGBA.")
            return None

        return bgra_frame
    except Exception as e:
        print(f"Error during BGRA conversion: {e}")
        return None


def send_thread(camera_index):
    video = VideoFileClip(r'C:\Users\zengxiangbin\Downloads\file_example_MP4_1280_10MG.mp4')
    cam = jericcam.camera(camera_index, video.w, video.h, 30)
    interval = 1 / 30

    # Here, you can wait for an application to connect to this camera.
    while not cam.wait_for_connection(timeout=1):
        pass

    # Iterate over the frames
    for frame in video.iter_frames():
        # Convert RGB to BGRA
        bgr_frame = convert_frame_to_bgra(frame)
        cam.send_frame(bgr_frame)
        time.sleep(interval)

    # Close the video file
    video.close()


def main():
    # check if jericcam is installed
    camera_count = 2
    for camera_index in range(camera_count):
        if not jericcam.is_installed(camera_index):
            install_camera(True, camera_count)
            break

    threads = []
    for camera_index in range(camera_count):
        thread = threading.Thread(target=send_thread, args=(camera_index,))
        thread.start()
        threads.append(thread)
    for thread in threads:
        thread.join()

    # uninstall the camera if not need, the second param is ignored
    install_camera(False, 0)


if __name__ == '__main__':
    main()
