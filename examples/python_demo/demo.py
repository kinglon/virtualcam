import sys
import os
from moviepy.editor import VideoFileClip
import time

# Get the directory of the running script
script_dir = os.path.dirname(os.path.abspath(__file__))

# Append the directory to sys.path
sys.path.append(script_dir)
import softcam

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

def install_camera(install):
    cmdLine = []
    executable_path = r'"{}\softcam_installer.exe"'.format(script_dir)
    cmdLine.append(executable_path)
    if install:
        cmdLine.append('register')
    else:
        cmdLine.append('unregister')
    cmdLine.append(r'"{}\softcam.dll"'.format(script_dir))
    runAsAdmin(cmdLine, True)

def main():
    # check if softcam is installed
    if not softcam.is_installed():
        install_camera(True)

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

    # uninstall the camera if not need
    install_camera(False)

if __name__ == '__main__':
    main()
