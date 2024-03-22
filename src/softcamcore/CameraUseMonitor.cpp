#include "CameraUseMonitor.h"
#include <thread>
#include <shlobj.h>
#include "Setting.h"

void CCameraUseMonitor::Init()
{
	static bool init = false;
	if (init)
	{
		return;
	}

	// �����̶߳�ʱ���ʹ�ý����Ƿ��˳�
	new std::thread(&CCameraUseMonitor::ThreadProc);

	init = true;
}

std::wstring CCameraUseMonitor::GetDataPath()
{
	static std::wstring dataPath;
	if (!dataPath.empty())
	{
		return dataPath;
	}

	// ����pidsĿ¼
	TCHAR szPath[MAX_PATH];
	if (SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, szPath) == S_OK)
	{
		dataPath = std::wstring(szPath) + L"\\softcam";
		CreateDirectory(dataPath.c_str(), nullptr);
	}
	else
	{
		dataPath = L"C:\\softcam";
		CreateDirectory(dataPath.c_str(), nullptr);
	}
	dataPath += std::wstring(L"\\") + CSetting::GetCameraName();
	CreateDirectory(dataPath.c_str(), nullptr);
	dataPath += L"\\pids\\";
	CreateDirectory(dataPath.c_str(), nullptr);

	return dataPath;
}

void CCameraUseMonitor::UseCamera(unsigned int pid)
{
	// �����ļ�����pid����
	std::wstring filePath = GetDataPath() + std::to_wstring(pid);
	HANDLE hFile = CreateFile(filePath.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hFile);
	}
}

std::vector<unsigned int> CCameraUseMonitor::GetCameraUsers()
{
	std::vector<unsigned int> pids;
	WIN32_FIND_DATA findFileData;
	HANDLE hFind = FindFirstFile((GetDataPath() + L"*").c_str(), &findFileData);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				int pid = _wtoi(findFileData.cFileName);
				if (pid > 0 &&IsProcessRunning(pid))
				{
					pids.push_back(pid);
				}
			}
		} while (FindNextFile(hFind, &findFileData));
		FindClose(hFind);
	}

	return pids;
}

void CCameraUseMonitor::ThreadProc()
{	
	while (true)
	{
		std::this_thread::sleep_for(std::chrono::seconds(3));

		// ɨ���Ѿ��˳��Ľ���
		std::vector<std::wstring> deadPids;
		WIN32_FIND_DATA findFileData;
		HANDLE hFind = FindFirstFile((GetDataPath() + L"*").c_str(), &findFileData);
		if (hFind != INVALID_HANDLE_VALUE)
		{
			do
			{
				if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
				{
					int pid = _wtoi(findFileData.cFileName);
					if (pid> 0 && !IsProcessRunning(pid))
					{
						deadPids.push_back(findFileData.cFileName);
					}
				}
			} while (FindNextFile(hFind, &findFileData));
			FindClose(hFind);
		}

		// ɾ���˳����̵��ļ�
		for (auto deadPid : deadPids)
		{
			::DeleteFile((GetDataPath() + deadPid).c_str());
		}
	}
}

bool CCameraUseMonitor::IsProcessRunning(unsigned int pid)
{
	HANDLE processHandle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
	if (processHandle != NULL)
	{
		DWORD exitCode = 0;
		if (GetExitCodeProcess(processHandle, &exitCode) && exitCode == STILL_ACTIVE)
		{
			CloseHandle(processHandle);
			return true;
		}

		CloseHandle(processHandle);
		return false;
	}
	else
	{
		return false;
	}
}