#include "Setting.h"

extern HANDLE g_dllModule;

// {AEF3B972-5FA5-4647-9571-358EB472BC9E}
DEFINE_GUID(CLSID_DShowSoftcam,
	0xaef3b972, 0x5fa5, 0x4647, 0x95, 0x71, 0x35, 0x8e, 0xb4, 0x72, 0xbc, 0x9e);

// {B58FE118-04CD-4A06-9166-C8126B2F3F6B}
DEFINE_GUID(CLSID_DShowSoftcam2,
	0xb58fe118, 0x4cd, 0x4a06, 0x91, 0x66, 0xc8, 0x12, 0x6b, 0x2f, 0x3f, 0x6b);

// {72C51EA8-B739-41AC-968F-1BE15666B0E6}
DEFINE_GUID(CLSID_DShowSoftcam3,
	0x72c51ea8, 0xb739, 0x41ac, 0x96, 0x8f, 0x1b, 0xe1, 0x56, 0x66, 0xb0, 0xe6);

// {452E97C4-CE5C-4EA2-9146-99FEA2D2B22F}
DEFINE_GUID(CLSID_DShowSoftcam4,
	0x452e97c4, 0xce5c, 0x4ea2, 0x91, 0x46, 0x99, 0xfe, 0xa2, 0xd2, 0xb2, 0x2f);

CSetting::CSetting()
{
	init();
}

CSetting* CSetting::GetInstance()
{
	static CSetting* instance = new CSetting();
	return instance;
}

void CSetting::init()
{
	// 初始化GUID
	m_cameraClsids.push_back(CLSID_DShowSoftcam);	
	m_cameraClsids.push_back(CLSID_DShowSoftcam2);	
	m_cameraClsids.push_back(CLSID_DShowSoftcam3);	
	m_cameraClsids.push_back(CLSID_DShowSoftcam4);

	m_cameraClsidStrings.push_back(L"AEF3B972-5FA5-4647-9571-358EB472BC9E");
	m_cameraClsidStrings.push_back(L"B58FE118-04CD-4A06-9166-C8126B2F3F6B");
	m_cameraClsidStrings.push_back(L"72C51EA8-B739-41AC-968F-1BE15666B0E6");
	m_cameraClsidStrings.push_back(L"452E97C4-CE5C-4EA2-9146-99FEA2D2B22F");

	// 获取dll所在的路径
	wchar_t szModulePath[MAX_PATH] = { 0 };
	GetModuleFileName((HMODULE)g_dllModule, szModulePath, MAX_PATH);
	for (auto i = wcslen(szModulePath); i >= 0; --i)
	{
		if (szModulePath[i] == '\\')
		{
			szModulePath[i] = 0;
			break;
		}
	}
	std::wstring strSoftInstallPath = std::wstring(szModulePath) + L"\\";

	// 从配置文件加载摄像头名称
	std::wstring configFilePath = strSoftInstallPath + L"conifg.ini";
	for (int i = 1; i <= CAMERA_NUMBER; i++)
	{
		std::wstring keyName = L"name" + std::to_wstring(i);
		wchar_t keyValue[MAX_PATH];
		memset(keyValue, 0, sizeof(keyValue));
		GetPrivateProfileString(L"camera_name", keyName.c_str(), L"", keyValue, MAX_PATH, configFilePath.c_str());
		if (wcslen(keyValue) == 0)
		{
			std::wstring defaultName = L"jeric camera";
			if (i > 1)
			{
				defaultName += std::to_wstring(i);				
			}
			m_cameraNames.push_back(defaultName);
		}
		else
		{
			m_cameraNames.push_back(keyValue);
		}
	}
}

std::wstring CSetting::GetCameraName(const GUID& clsid)
{
	for (unsigned int i = 0; i < m_cameraClsids.size(); i++)
	{
		if (IsEqualGUID(m_cameraClsids[i], clsid))		
		{
			return m_cameraNames[i];
		}
	}

	return L"";
}

std::wstring CSetting::GetCameraClsidString(const GUID& clsid)
{
	for (unsigned int i = 0; i < m_cameraClsids.size(); i++)
	{
		if (IsEqualGUID(m_cameraClsids[i], clsid))
		{
			return m_cameraClsidStrings[i];
		}
	}

	return L"";
}

std::wstring CSetting::GetMutexNameByIndex(int cameraIndex)
{
	if (cameraIndex >= 0 && cameraIndex < CAMERA_NUMBER)
	{
		return m_cameraClsidStrings[cameraIndex] + L"_mutex";
	}

	return L"";
}

std::wstring CSetting::GetSharedMemoryNameByIndex(int cameraIndex)
{
	if (cameraIndex >= 0 && cameraIndex < CAMERA_NUMBER)
	{
		return m_cameraClsidStrings[cameraIndex] + L"_sharedmemory";
	}

	return L"";
}

std::wstring CSetting::GetMutexNameByClsid(const GUID& clsid)
{
	for (unsigned int i = 0; i < m_cameraClsids.size(); i++)
	{
		if (IsEqualGUID(m_cameraClsids[i], clsid))
		{
			return GetMutexNameByIndex(i);
		}
	}

	return L"";
}

std::wstring CSetting::GetSharedMemoryNameByClsid(const GUID& clsid)
{
	for (unsigned int i = 0; i < m_cameraClsids.size(); i++)
	{
		if (IsEqualGUID(m_cameraClsids[i], clsid))
		{
			return GetSharedMemoryNameByIndex(i);
		}
	}

	return L"";
}