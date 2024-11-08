#pragma once

#include <vector>
#include <string>

// �����Щ����ʹ������ͷ
class CCameraUseMonitor
{
public:
	// ��ʼ��
	static void Init();

	// ָ������ʹ������ͷ
	static void UseCamera(std::wstring cameraName, unsigned int pid);

	// ��ȡ����ͷʹ�ý���
	static std::vector<unsigned int> GetCameraUsers(std::wstring cameraName);

private:
	static void ThreadProc();

	static bool IsProcessRunning(unsigned int pid);

	static std::wstring GetDataPath(std::wstring cameraName);
};

