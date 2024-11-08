#pragma once

#include <Windows.h>
#include <olectl.h>
#include <initguid.h>
#include <string>
#include <vector>

// 虚拟摄像头的个数
#define CAMERA_NUMBER 4

class CSetting
{
private:
	CSetting();

public:
	static CSetting* GetInstance();

	// 获取摄像头名字列表
	const std::vector<std::wstring>& GetCameraNames() { return m_cameraNames; }

	// 获取指定CLSID的摄像头名字
	std::wstring GetCameraName(const GUID& clsid);

	// 获取摄像头CLSID列表，GUID型
	const std::vector<GUID>& GetCameraClsid() { return m_cameraClsids;  }

	// 获取摄像头CLSID列表，wstring型
	const std::vector<std::wstring>& GetCameraClsidStrings() { return m_cameraClsidStrings; }

	// 获取指定摄像头的clsid字符串
	std::wstring GetCameraClsidString(const GUID& clsid);

	// 获取指定摄像头的mutex和shared memory名字
	std::wstring GetMutexNameByIndex(int cameraIndex);
	std::wstring GetSharedMemoryNameByIndex(int cameraIndex);
	std::wstring GetMutexNameByClsid(const GUID& clsid);
	std::wstring GetSharedMemoryNameByClsid(const GUID& clsid);

private:
	void init();

private:
	std::vector<std::wstring> m_cameraNames;

	std::vector<GUID> m_cameraClsids;

	std::vector<std::wstring> m_cameraClsidStrings;
};
