#include "softcam.h"
#include <Windows.h>
#include <olectl.h>
#include <initguid.h>
#include <string>

#include <softcamcore/DShowSoftcam.h>
#include <softcamcore/SenderAPI.h>
#include <softcamcore/CameraUseMonitor.h>
#include <softcamcore/Setting.h>

HANDLE g_dllModule = NULL;
CFactoryTemplate g_Templates[CAMERA_NUMBER];
int g_cTemplates = CAMERA_NUMBER;

namespace {

const AMOVIESETUP_MEDIATYPE s_pin_types[] =
{
    {
        &MEDIATYPE_Video,       // Major type
        &MEDIASUBTYPE_ARGB32      // Minor type
    },
    {
        &MEDIATYPE_Video,       // Major type
        &MEDIASUBTYPE_RGB32      // Minor type
    }
};

const AMOVIESETUP_PIN s_pins[] =
{
    {
        const_cast<LPWSTR>(L"Output"),  // Pin string name
        FALSE,                  // Is it rendered
        TRUE,                   // Is it an output
        FALSE,                  // Can we have none
        FALSE,                  // Can we have many
        &CLSID_NULL,            // Connects to filter
        NULL,                   // Connects to pin
        2,                      // Number of types
        s_pin_types             // Pin details
    }
};

const REGFILTER2 s_reg_filter2 =
{
    1,
    MERIT_DO_NOT_USE,
    1,
    s_pins
};

CUnknown * WINAPI CreateSoftcamInstance(LPUNKNOWN lpunk, HRESULT *phr)
{
    const std::vector<GUID>& cameraClsid = CSetting::GetInstance()->GetCameraClsid();
    return softcam::Softcam::CreateInstance(lpunk, cameraClsid[0], phr);
}

CUnknown* WINAPI CreateSoftcamInstance2(LPUNKNOWN lpunk, HRESULT* phr)
{
    const std::vector<GUID>& cameraClsid = CSetting::GetInstance()->GetCameraClsid();
    return softcam::Softcam::CreateInstance(lpunk, cameraClsid[1], phr);
}

CUnknown* WINAPI CreateSoftcamInstance3(LPUNKNOWN lpunk, HRESULT* phr)
{
    const std::vector<GUID>& cameraClsid = CSetting::GetInstance()->GetCameraClsid();
    return softcam::Softcam::CreateInstance(lpunk, cameraClsid[2], phr);
}

CUnknown* WINAPI CreateSoftcamInstance4(LPUNKNOWN lpunk, HRESULT* phr)
{
    const std::vector<GUID>& cameraClsid = CSetting::GetInstance()->GetCameraClsid();
    return softcam::Softcam::CreateInstance(lpunk, cameraClsid[3], phr);
}

} // namespace

STDAPI RegisterCameraFilter(int cameraNum)
{
    HRESULT hr = AMovieDllRegisterServer2(TRUE);
    if (FAILED(hr))
    {
        return hr;
    }

    hr = CoInitialize(nullptr);
    if (FAILED(hr))
    {
        return hr;
    }

    if (cameraNum > CAMERA_NUMBER)
    {
        cameraNum = CAMERA_NUMBER;
    }
    const std::vector<std::wstring>& cameraNames = CSetting::GetInstance()->GetCameraNames();
    const std::vector<GUID>& cameraClsid = CSetting::GetInstance()->GetCameraClsid();
    for (int i=0; i< cameraNum; i++)
    {
        IFilterMapper2* pFM2 = nullptr;
        hr = CoCreateInstance(
            CLSID_FilterMapper2, nullptr, CLSCTX_INPROC_SERVER,
            IID_IFilterMapper2, (void**)&pFM2);
        if (FAILED(hr))
        {
            break;
        }

        pFM2->UnregisterFilter(
            &CLSID_VideoInputDeviceCategory,
            0,
            cameraClsid[i]);

        hr = pFM2->RegisterFilter(
            cameraClsid[i],
            cameraNames[i].c_str(),
            0,
            &CLSID_VideoInputDeviceCategory,
            cameraNames[i].c_str(),
            &s_reg_filter2);

        pFM2->Release();
    }

    CoFreeUnusedLibraries();
    CoUninitialize();
    return hr;
}

STDAPI DllRegisterServer()
{
    return RegisterCameraFilter(CAMERA_NUMBER);
}

STDAPI DllUnregisterServer()
{
    HRESULT hr = AMovieDllRegisterServer2(FALSE);
    if (FAILED(hr))
    {
        return hr;
    }

    hr = CoInitialize(nullptr);
    if (FAILED(hr))
    {
        return hr;
    }

    const std::vector<std::wstring>& cameraNames = CSetting::GetInstance()->GetCameraNames();
    const std::vector<GUID>& cameraClsid = CSetting::GetInstance()->GetCameraClsid();
    for (int i = 0; i < CAMERA_NUMBER; i++)
    {
        IFilterMapper2 *pFM2 = nullptr;
        hr = CoCreateInstance(
                CLSID_FilterMapper2, nullptr, CLSCTX_INPROC_SERVER,
                IID_IFilterMapper2, (void**)&pFM2);
        if (FAILED(hr))
        {
            break;
        }

        hr = pFM2->UnregisterFilter(
                &CLSID_VideoInputDeviceCategory,
                cameraNames[i].c_str(),
                cameraClsid[i]);

        pFM2->Release();
    }

    CoFreeUnusedLibraries();
    CoUninitialize();
    return hr;
}

extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);

BOOL APIENTRY DllMain(HANDLE hModule, DWORD  dwReason, LPVOID lpReserved)
{
    g_dllModule = hModule;
    const std::vector<std::wstring>& cameraNames = CSetting::GetInstance()->GetCameraNames();
    const std::vector<GUID>& cameraClsids = CSetting::GetInstance()->GetCameraClsid();
    for (int i = 0; i < CAMERA_NUMBER; i++)
    {
        g_Templates[i].m_Name = cameraNames[i].c_str();
        g_Templates[i].m_ClsID = &cameraClsids[i];
        g_Templates[i].m_lpfnInit = nullptr;
        g_Templates[i].m_pAMovieSetup_Filter = nullptr;
    }
    g_Templates[0].m_lpfnNew = &CreateSoftcamInstance;
    g_Templates[1].m_lpfnNew = &CreateSoftcamInstance2;
    g_Templates[2].m_lpfnNew = &CreateSoftcamInstance3;
    g_Templates[3].m_lpfnNew = &CreateSoftcamInstance4;

    return DllEntryPoint((HINSTANCE)(hModule), dwReason, lpReserved);
}


//
// Softcam Sender API
//

extern "C" scCamera scCreateCamera(int index, int width, int height, float framerate)
{
    CCameraUseMonitor::Init();
    return softcam::sender::CreateCamera(index, width, height, framerate);
}

extern "C" void     scDeleteCamera(scCamera camera)
{
    return softcam::sender::DeleteCamera(camera);
}

extern "C" void     scSendFrame(scCamera camera, const void* image_bits, int length)
{
    return softcam::sender::SendFrame(camera, image_bits, length);
}

extern "C" void     scSendBitmapFrame(scCamera camera, scBitmap bitmap)
{
    HBITMAP hBitmap = (HBITMAP)bitmap;

    // Get the bitmap information
    BITMAP bmp;
    GetObject(hBitmap, sizeof(BITMAP), &bmp);

    // Allocate memory for pixel data
    int dataSize = bmp.bmWidth * bmp.bmHeight * 4; // 4 bytes per pixel for BGRA format
    BYTE* pixelData = new BYTE[dataSize];
    if (pixelData == nullptr)
    {
        return;
    }

    // Get the bitmap data
    BITMAPINFOHEADER bi;
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = bmp.bmWidth;
    bi.biHeight = bmp.bmHeight*-1;
    bi.biPlanes = 1;
    bi.biBitCount = 32; // BGRA format
    bi.biCompression = BI_RGB;
    bi.biSizeImage = dataSize;

    HDC hdc = GetDC(NULL);
    GetDIBits(hdc, hBitmap, 0, bmp.bmHeight, pixelData, (BITMAPINFO*)&bi, DIB_RGB_COLORS);
    softcam::sender::SendFrame(camera, pixelData, dataSize);

    // Clean up
    delete[] pixelData;
    ReleaseDC(NULL, hdc);
}

extern "C" bool     scWaitForConnection(scCamera camera, float timeout)
{
    return softcam::sender::WaitForConnection(camera, timeout);
}

extern "C" bool    scGetInstallationStatus(int index, bool& bInstalled)
{
    if (index >= CAMERA_NUMBER)
    {
        return false;
    }

    bInstalled = false;

    // Initialize the COM library
    HRESULT hr = CoInitialize(nullptr);
    if (FAILED(hr))
    {
        return false;
    }

    // Create the system device enumerator
    ICreateDevEnum* pDevEnum = nullptr;
    hr = CoCreateInstance(CLSID_SystemDeviceEnum, nullptr, CLSCTX_INPROC_SERVER, 
        IID_ICreateDevEnum, (void**)(&pDevEnum));
    if (FAILED(hr))
    {
        //std::cerr << "Failed to create system device enumerator." << std::endl;
        //CoUninitialize();
        return false;
    }

    // Enumerate the video capture devices (cameras)
    IEnumMoniker* pEnumMoniker = nullptr;
    hr = pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnumMoniker, 0);
    if (hr == S_OK)
    {
        const std::wstring friendName = CSetting::GetInstance()->GetCameraNames()[index];

        // Iterate through the devices
        IMoniker* pMoniker = nullptr;
        while (pEnumMoniker->Next(1, &pMoniker, nullptr) == S_OK)
        {
            // Get the friendly name of the device            
            IPropertyBag* pPropBag = nullptr;
            hr = pMoniker->BindToStorage(nullptr, nullptr, IID_PPV_ARGS(&pPropBag));
            if (SUCCEEDED(hr))
            {
                VARIANT varName;
                VariantInit(&varName);

                hr = pPropBag->Read(L"FriendlyName", &varName, nullptr);
                if (SUCCEEDED(hr))
                {
                    // Compare the device name with the desired name
                    if (friendName.compare(varName.bstrVal) == 0)
                    {
                        bInstalled = true;
                        VariantClear(&varName);
                        pPropBag->Release();
                        pMoniker->Release();
                        break;
                    }
                }

                VariantClear(&varName);
                pPropBag->Release();
            }

            pMoniker->Release();
        }

        pDevEnum->Release();
        pEnumMoniker->Release();
        return true;
    }
    else
    {
        pDevEnum->Release();
        return false;
    }
}


extern "C" int SOFTCAM_API scGetCameraUsers(int index, unsigned int* pids, unsigned int size)
{
    if (index >= CAMERA_NUMBER)
    {
        return 0;
    }

    std::wstring cameraName = CSetting::GetInstance()->GetCameraNames()[index];
    std::vector<unsigned int> users = CCameraUseMonitor::GetCameraUsers(cameraName);
    unsigned int length = min((unsigned int)users.size(), size);
    for (unsigned int i = 0; i < length; i++)
    {
        pids[i] = users[i];
    }
    return length;
}