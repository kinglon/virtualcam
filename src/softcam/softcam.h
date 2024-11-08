#pragma once


//
// Softcam Sender API
//

#define SOFTCAM_API __cdecl

extern "C"
{
    using scCamera = void*;

    using scBitmap = void*;

    /*
    ��������ͷ
    @param index, ����ͷ���� 0-3
    @param width������ͷ���
    @param height�� ����ͷ�߶�
    @param framerate, ֡��
    @return ����ͷ��������������ӿ�ʱ����
    */
    scCamera SOFTCAM_API scCreateCamera(int index, int width, int height, float framerate = 60.0f);

    /*
    ɾ������ͷ
    @param camera������ͷ�����scCreateCamera���ص�ֵ
    */
    void SOFTCAM_API scDeleteCamera(scCamera camera);

    /*
    �ȴ�Ӧ��ʹ������ͷ
    @param camera������ͷ�����scCreateCamera���ص�ֵ
    @param timeout���ȴ���ʱ����λ�룬0.0f��ʾ���õȴ�
    @return true�ڳ�ʱǰ��Ӧ�����ӣ�false�ڳ�ʱǰû��Ӧ������
    */
    bool SOFTCAM_API scWaitForConnection(scCamera camera, float timeout = 0.0f);

    /*
    ����ͼƬ֡
    @param camera������ͷ�����scCreateCamera���ص�ֵ
    @param image_data��ͼƬ���ݣ�BGRA��ʽ
    @param length, image_data�ĳ��ȣ�����ͷ��*����ͷ��*4
    */
    void SOFTCAM_API scSendFrame(scCamera camera, const void* image_data, int length);

    /*
    ����ͼƬ֡����scSendFrame��ͬ���ṩһ��ͼƬ���HBITMAP��Ϊ����
    @param camera������ͷ�����scCreateCamera���ص�ֵ
    @param bitmap, ͼƬ���HBITMAP
    */
    void SOFTCAM_API scSendBitmapFrame(scCamera camera, scBitmap bitmap);    

    /*
    ��ȡ����ͷ��װ״̬
    @param index, ����ͷ���� 0-3
    @param bInstalled, ���óɹ�����Ч��true��ʾ����ͷ�Ѿ���װ, false��ʾ����ͷδ��װ
    @return true���óɹ�, false����ʧ��
    */
    bool SOFTCAM_API scGetInstallationStatus(int index, bool& bInstalled);

    /*
    ��ȡʹ������ͷ��Ӧ���б�
    @param index, ����ͷ���� 0-3
    @param pids�����ʹ������ͷ��Ӧ�ý���
    @param size, pids�����С
    @return ʹ������ͷ��Ӧ������Ӧ�ý���id�����pids���棬����ֵ���ᳬ���������size
    */
    int SOFTCAM_API scGetCameraUsers(int index, unsigned int* pids, unsigned int size);
}
