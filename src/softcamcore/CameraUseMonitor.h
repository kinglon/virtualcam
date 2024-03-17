#pragma once

#include <vector>
#include <string>

// 监控哪些程序使用摄像头
class CCameraUseMonitor
{
public:
	// 初始化
	static void Init();

	// 指定进程使用摄像头
	static void UseCamera(unsigned int pid);

	// 获取摄像头使用进程
	static std::vector<unsigned int> GetCameraUsers();

private:
	static void ThreadProc();

	static bool IsProcessRunning(unsigned int pid);

	static std::wstring GetDataPath();
};

