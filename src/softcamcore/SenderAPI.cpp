#include "SenderAPI.h"

#include <atomic>
#include <vector>
#include "FrameBuffer.h"
#include "Misc.h"
#include "Setting.h"


namespace {

struct Camera
{
    softcam::FrameBuffer    m_frame_buffer;
    softcam::Timer          m_timer;
};

} //namespace


namespace softcam {
namespace sender {

// cameraIndex  ��0��ʼ
CameraHandle    CreateCamera(int cameraIndex, int width, int height, float framerate)
{
    if (cameraIndex >= CAMERA_NUMBER)
    {
        return nullptr;
    }
    
    std::wstring mutextName = CSetting::GetInstance()->GetMutexNameByIndex(cameraIndex);
    std::wstring sharedMemoryName = CSetting::GetInstance()->GetSharedMemoryNameByIndex(cameraIndex);
    if (auto fb = FrameBuffer::create(mutextName.c_str(), sharedMemoryName.c_str(), width, height, framerate))
    {
        Camera* camera = new Camera{ fb, Timer() };
        return camera;        
    }
    return nullptr;
}

void            DeleteCamera(CameraHandle camera)
{
    Camera* target = static_cast<Camera*>(camera);
    if (target)
    {
        target->m_frame_buffer.deactivate();
        delete target;
    }
}

void            SendFrame(CameraHandle camera, const void* image_bits, int length)
{
    Camera* target = static_cast<Camera*>(camera);
    if (target && image_bits && length >= target->m_frame_buffer.framesize())
    {
        auto framerate = target->m_frame_buffer.framerate();
        auto frame_counter = target->m_frame_buffer.frameCounter();

        // To deliver frames in the regular period, we sleep here a bit
        // before we deliver the new frame if it's not the time yet.
        // If it's already the time, we deliver it immediately and
        // let the timer keep running so that if the next frame comes
        // in time the constant delivery recovers.
        // However if the delay grew too much (greater than 50 percent
        // of the period), we reset the timer to avoid continuing
        // irregular delivery.
        if (0.0f < framerate)
        {
            if (0 == frame_counter) // the first frame
            {
                target->m_timer.reset();
            }
            else
            {
                auto ref_delta = 1.0f / framerate;
                auto time = target->m_timer.get();
                if (time < ref_delta)
                {
                    Timer::sleep(ref_delta - time);
                }
                if (time < ref_delta * 1.5f)
                {
                    target->m_timer.rewind(ref_delta);
                }
                else
                {
                    target->m_timer.reset();
                }
            }
        }

        target->m_frame_buffer.write(image_bits);
    }
}

bool            WaitForConnection(CameraHandle camera, float timeout)
{
    Camera* target = static_cast<Camera*>(camera);
    if (target)
    {
        Timer timer;
        while (!target->m_frame_buffer.connected())
        {
            if (0.0f < timeout && timeout <= timer.get())
            {
                return false;
            }
            Timer::sleep(0.001f);
        }
        return true;
    }
    return false;
}

} //namespace sender
} //namespace softcam
