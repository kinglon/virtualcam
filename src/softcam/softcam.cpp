#include "softcam.h"
#include <Windows.h>
#include <olectl.h>
#include <initguid.h>
#include <string>

#include <softcamcore/DShowSoftcam.h>
#include <softcamcore/SenderAPI.h>
#include <softcamcore/CameraUseMonitor.h>


// {AEF3B972-5FA5-4647-9571-358EB472BC9E}
DEFINE_GUID(CLSID_DShowSoftcam,
0xaef3b972, 0x5fa5, 0x4647, 0x95, 0x71, 0x35, 0x8e, 0xb4, 0x72, 0xbc, 0x9e);


namespace {

// Setup data

const wchar_t FILTER_NAME[] = L"DirectShow Softcam";
const GUID &FILTER_CLASSID = CLSID_DShowSoftcam;

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
    return softcam::Softcam::CreateInstance(lpunk, FILTER_CLASSID, phr);
}

} // namespace

// COM global table of objects in this dll

CFactoryTemplate g_Templates[] =
{
    {
        FILTER_NAME,
        &FILTER_CLASSID,
        &CreateSoftcamInstance,
        NULL,
        nullptr
    }
};
int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);


STDAPI DllRegisterServer()
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
    do
    {
        IFilterMapper2 *pFM2 = nullptr;
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
                FILTER_CLASSID);
        hr = pFM2->RegisterFilter(
                FILTER_CLASSID,
                FILTER_NAME,
                0,
                &CLSID_VideoInputDeviceCategory,
                FILTER_NAME,
                &s_reg_filter2);
        pFM2->Release();
    } while (0);
    CoFreeUnusedLibraries();
    CoUninitialize();
    return hr;
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
    do
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
                FILTER_NAME,
                FILTER_CLASSID);
        pFM2->Release();
    } while (0);
    CoFreeUnusedLibraries();
    CoUninitialize();
    return hr;
}

extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);

BOOL APIENTRY DllMain(HANDLE hModule, DWORD  dwReason, LPVOID lpReserved)
{
    return DllEntryPoint((HINSTANCE)(hModule), dwReason, lpReserved);
}


//
// Softcam Sender API
//

extern "C" scCamera scCreateCamera(int width, int height, float framerate)
{
    CCameraUseMonitor::Init();
    return softcam::sender::CreateCamera(width, height, framerate);
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

extern "C" bool    scGetInstallationStatus(bool& bInstalled)
{
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
        const std::wstring friendName = FILTER_NAME;

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


extern "C" int SOFTCAM_API scGetCameraUsers(unsigned int* pids, unsigned int size)
{
    std::vector<unsigned int> users = CCameraUseMonitor::GetCameraUsers();
    unsigned int length = min((unsigned int)users.size(), size);
    for (unsigned int i = 0; i < length; i++)
    {
        pids[i] = users[i];
    }
    return length;
}