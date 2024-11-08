#include <windows.h>
#include <string>
#include <cstring>
#include <cstdio>


void Message(const std::string& message)
{
    MessageBoxA(NULL, message.c_str(), "Ã· æ", MB_OK);
}


std::string ToHex(long x)
{
    char buff[128];
    std::snprintf(buff, sizeof(buff), "%08lx\n", x);
    return buff;
}


HMODULE LoadDLL(const std::string& path)
{
    HMODULE hmod = LoadLibraryA(path.c_str());
    if (!hmod)
    {
        Message("Error: can't load DLL");
        std::exit(1);
    }
    return hmod;
}


template <typename Func>
Func* GetProc(HMODULE hmod, const std::string& name)
{
    Func* func = (Func*)GetProcAddress(hmod, name.c_str());
    if (!func)
    {
        Message("Error: can't find function " + name + " in DLL");
        std::exit(1);
    }
    return func;
}


int WINAPI WinMain(
    HINSTANCE /*hInstance*/,
    HINSTANCE /*hPrevInstance*/,
    LPSTR /*lpszCmdLine*/,
    int /*nCmdShow*/)
{
    if (__argc < 3)
    {
        Message(
            "Usage:\n"
            "   jericcam_installer.exe register <jericcam.dll path> camera_number\n"
            "   jericcam_installer.exe unregister <jericcam.dll path>");
        return 0;
    }

    std::string cmd = __argv[1], path = __argv[2];
    int cameraNumber = 0;
    if (cmd == "register")
    {
        if (__argc < 4)
        {
            Message(
                "Usage:\n"
                "   jericcam_installer.exe register <jericcam.dll path> camera_number\n"
                "   jericcam_installer.exe unregister <jericcam.dll path>");
            return 0;
        }

        cameraNumber = atoi(__argv[3]);
        if (cameraNumber <= 0)
        {
            Message("the param of camera number is wrong");
            return 0;
        }
    }

    if (cmd == "register")
    {

        auto hmod = LoadDLL(path);
        auto RegisterCameraFilter = GetProc<HRESULT STDAPICALLTYPE(int cameraNum)>(hmod, "RegisterCameraFilter");

        auto hr = RegisterCameraFilter(cameraNumber);

        if (FAILED(hr))
        {
            Message("–Èƒ‚…„œÒÕ∑∞≤◊∞ ß∞‹£¨¥ÌŒÛ¬Î(" + ToHex(hr) + ")");
            return 1;
        }

        //Message("softcam.dll has been successfully registered to the system");
        return 0;
    }
    else if (cmd == "unregister")
    {
        auto hmod = LoadDLL(path);
        auto UnregisterServer = GetProc<HRESULT STDAPICALLTYPE()>(hmod, "DllUnregisterServer");

        auto hr = UnregisterServer();

        if (FAILED(hr))
        {
            Message("–Èƒ‚…„œÒÕ∑…æ≥˝ ß∞‹£¨¥ÌŒÛ¬Î(" + ToHex(hr) + ")");
            return 1;
        }

        //Message("softcam.dll has been successfully unregistered from the system");
        return 0;
    }
    else
    {
        Message("Error: invalid option");
        return 1;
    }
}
