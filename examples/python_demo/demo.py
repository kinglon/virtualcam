import sys
import os
from moviepy.editor import VideoFileClip
import time
import numpy as np
import cv2

# Get the directory of the running script
script_dir = os.path.dirname(os.path.abspath(__file__))

# Append the directory to sys.path
sys.path.append(script_dir)
import jericcam

def runAsAdmin(cmdLine=None, wait=True):
    import win32api, win32con, win32event, win32process
    from win32com.shell.shell import ShellExecuteEx
    from win32com.shell import shellcon
    python_exe = sys.executable
    cmd = '"%s"' % (cmdLine[0],)
    # XXX TODO: isn't there a function or something we can call to massage command line params?
    params = " ".join(['"%s"' % (x,) for x in cmdLine[1:]])
    cmdDir = ''
    showCmd = win32con.SW_SHOWNORMAL
    #showCmd = win32con.SW_HIDE
    lpVerb = 'runas'  # causes UAC elevation prompt.
    # print "Running", cmd, params
    # ShellExecute() doesn't seem to allow us to fetch the PID or handle
    # of the process, so we can't get anything useful from it. Therefore
    # the more complex ShellExecuteEx() must be used.
    # procHandle = win32api.ShellExecute(0, lpVerb, cmd, params, cmdDir, showCmd)
    procInfo = ShellExecuteEx(nShow=showCmd,
                              fMask=shellcon.SEE_MASK_NOCLOSEPROCESS,
                              lpVerb=lpVerb,
                              lpFile=cmd,
                              lpParameters=params)
    if wait:
        procHandle = procInfo['hProcess']
        obj = win32event.WaitForSingleObject(procHandle, win32event.INFINITE)
        rc = win32process.GetExitCodeProcess(procHandle)
        #print "Process handle %s returned code %s" % (procHandle, rc)
    else:
        rc = None
    return rc

# camera_number 取值1-4
def install_camera(install, camera_number):
    cmdLine = []
    executable_path = r'"{}\jericcam_installer.exe"'.format(script_dir)
    cmdLine.append(executable_path)
    if install:
        cmdLine.append('register')        
    else:
        cmdLine.append('unregister')
    cmdLine.append(r'"{}\jericcam.dll"'.format(script_dir))
    if install:
        cmdLine.append(str(camera_number))
    runAsAdmin(cmdLine, True)

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
            rgba_frame = frame.copy() #avoid modifying the original frame
            bgra_frame = rgba_frame[:,:,[2,1,0,3]] #reorder channels


        else:
            print("Error: Unsupported frame format. Must be RGB or RGBA.")
            return None

        return bgra_frame
    except Exception as e:
        print(f"Error during BGRA conversion: {e}")
        return None
        
def main():
    # check if jericcam is installed
    camera_index = 0  # 0-3
    if not jericcam.is_installed(camera_index):
        install_camera(True, 2)

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

    # uninstall the camera if not need, the second param is ignored
    install_camera(False, 0)

if __name__ == '__main__':
    main()
