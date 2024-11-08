#pragma once


namespace softcam {
namespace sender {

using CameraHandle = void*;

CameraHandle    CreateCamera(int index, int width, int height, float framerate = 60.0f);
void            DeleteCamera(CameraHandle camera);
void            SendFrame(CameraHandle camera, const void* image_bits, int length);
bool            WaitForConnection(CameraHandle camera, float timeout = 0.0f);

} //namespace sender
} //namespace softcam
