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
	static void UseCamera(unsigned int pid);

	// ��ȡ����ͷʹ�ý���
	static std::vector<unsigned int> GetCameraUsers();

private:
	static void ThreadProc();

	static bool IsProcessRunning(unsigned int pid);

	static std::wstring GetDataPath();
};

