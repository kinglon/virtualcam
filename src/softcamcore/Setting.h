#pragma once

#include <Windows.h>
#include <olectl.h>
#include <initguid.h>
#include <string>
#include <vector>

// ��������ͷ�ĸ���
#define CAMERA_NUMBER 4

class CSetting
{
private:
	CSetting();

public:
	static CSetting* GetInstance();

	// ��ȡ����ͷ�����б�
	const std::vector<std::wstring>& GetCameraNames() { return m_cameraNames; }

	// ��ȡָ��CLSID������ͷ����
	std::wstring GetCameraName(const GUID& clsid);

	// ��ȡ����ͷCLSID�б�GUID��
	const std::vector<GUID>& GetCameraClsid() { return m_cameraClsids;  }

	// ��ȡ����ͷCLSID�б�wstring��
	const std::vector<std::wstring>& GetCameraClsidStrings() { return m_cameraClsidStrings; }

	// ��ȡָ������ͷ��clsid�ַ���
	std::wstring GetCameraClsidString(const GUID& clsid);

	// ��ȡָ������ͷ��mutex��shared memory����
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
