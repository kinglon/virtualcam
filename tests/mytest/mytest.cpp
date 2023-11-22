#include <iostream>
#include <softcam/softcam.h>

int main()
{
    bool isInstalled = false;
    if (!scGetInstallationStatus(isInstalled))
    {
        std::cout << "failed to get the installation" << std::endl;
    }
    else
    {
        std::cout << "the status is " << isInstalled << std::endl;
    }
}