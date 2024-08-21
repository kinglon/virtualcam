#include <cstdio>
#include <Windows.h>
#include <vector>
#include <algorithm>
#include <string>
#include <cmath>
#include <gdiplus.h>
#include <softcam/softcam.h>

#pragma comment (lib, "gdiplus.lib")

const int WIDTH = 1920;
const int HEIGHT = 1080;


/// A Rendering Example of Bouncing Balls
class BouncingBalls
{
 public:
    static constexpr int NUM_BALLS = 2;
    static constexpr float RADIUS = 30.0f;
    static constexpr float GRAVITY = 400.0f;
    static constexpr float STIFFNESS = 600.0f;

    struct Ball
    {
        float x = RADIUS, y = RADIUS * 2.0f;
        float vx = 100.0f, vy = 0.0f;
        float ax = 0.0f, ay = 0.0f;
        float rx = RADIUS, ry = RADIUS;

        void    move(float delta)
        {
            vx += delta * ax;
            vy += delta * ay;
            x += delta * vx;
            y += delta * vy;
            float cx = min(max(x, RADIUS), (float)WIDTH - RADIUS);
            float cy = min(y, (float)HEIGHT - RADIUS);
            ax = (cx - x) * STIFFNESS;
            ay = (cy - y) * STIFFNESS + GRAVITY;
            rx = max(RADIUS - std::abs(cx - x), RADIUS * 0.1f);
            ry = max(RADIUS - std::abs(cy - y), RADIUS * 0.1f);
        }
        void    collide(Ball& other)
        {
            float dx = x - other.x;
            float dy = y - other.y;
            float d = std::sqrt(dx * dx + dy * dy);
            if (d <= RADIUS * 2.0f)
            {
                float p = RADIUS * 2.0f - d;
                float nx = dx / max(RADIUS, d);
                float ny = dy / max(RADIUS, d);
                ax += p * STIFFNESS * nx;
                ay += p * STIFFNESS * ny;
                other.ax -= p * STIFFNESS * nx;
                other.ay -= p * STIFFNESS * ny;
            }
        }
        bool    hit(int sx, int sy) const
        {
            float dx = (float(sx) + 0.5f - x) / rx;
            float dy = (float(sy) + 0.5f - y) / ry;
            return 1.0f >= dx * dx + dy * dy;
        }
    };

    Ball balls[NUM_BALLS];

    BouncingBalls()
    {
        for (int i = 0; i < NUM_BALLS; i++)
        {
            balls[i].x = (float)(WIDTH * (i + 1) / (2 * NUM_BALLS));
            balls[i].y = (float)(HEIGHT * i / (2 * NUM_BALLS)) + RADIUS * 2.0f;
            balls[i].vx = RADIUS * 2.0f * (float)(i + 1);
        }
    }
    void move(float delta)
    {
        for (int i = 0; i < NUM_BALLS; i++)
        {
            balls[i].move(delta);
        }
        for (int i = 0; i < NUM_BALLS; i++)
        {
            for (int j = i + 1; j < NUM_BALLS; j++)
            {
                balls[i].collide(balls[j]);
            }
        }
    }
    void draw(unsigned char *image)
    {
        for (int y = 0; y < HEIGHT; y++)
        {
            for (int x = 0; x < WIDTH; x++)
            {
                if (std::any_of(
                        balls, balls + NUM_BALLS,
                        [x,y](Ball& ball) { return ball.hit(x, y); }))
                {
                    int red = x * 256 / WIDTH;
                    int green = 255;
                    int blue = y * 256 / HEIGHT;
                    int alpha = 255;

                    image[0] = (unsigned char)blue;
                    image[1] = (unsigned char)green;
                    image[2] = (unsigned char)red;
                    image[3] = (unsigned char)alpha;
                }
                else
                {
                    image[0] = 0;
                    image[1] = 0;
                    image[2] = 0;
                    image[3] = 0;
                }
                image += 4;
            }
        }
    }
};


int main()
{
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    // First, create a virtual camera instance with scCreateCamera().
    // A virtual camera is a source of a video image stream.
    // The dimension width and height can be any positive number
    // that is a multiple of 4.
    // The third argument framerate is used to make sending frames at regular intervals.
    // This framerate argument can be omitted, and the default framerate is 60.
    // If you want to send every frame immediately without the frame rate regulator,
    // specify 0 to the framerate argument, then it will be a variable frame rate.
    scCamera cam = scCreateCamera(WIDTH, HEIGHT, 30);
    if (!cam)
    {
        std::printf("failed to create camera\n");
        return 1;
    }
    std::printf("Ok, jericcam is now active.\n");

    // Here, we wait for an application to connect to this camera.
    // You can comment out this line to start sending frames immediately
    // no matter there is a receiver or not.
    // scWaitForConnection(cam);

    // Our canvas is a simple array of ARGB pixels.
    // Note that the color component order is BGRA, not ARGB.
    // This is due to the convention of DirectShow.
    std::vector<unsigned char> image(WIDTH * HEIGHT * 4);

    // This is an example class for drawing something to the canvas.
    BouncingBalls balls;

    // ��ȡ����ͼƬ·��
    wchar_t szModulePath[MAX_PATH] = { 0 };
    GetModuleFileName(NULL, szModulePath, MAX_PATH);
    for (size_t i = wcslen(szModulePath); i >= 0; --i)
    {
        if (szModulePath[i] == '\\')
        {
            szModulePath[i] = 0;
            break;
        }
    }
    std::wstring testImagePath = szModulePath;
    testImagePath += L"\\test.png";

    // ����ͼƬ
    HBITMAP hBitmap = NULL;
    Gdiplus::Bitmap* bitmap = Gdiplus::Bitmap::FromFile(testImagePath.c_str(), FALSE);
    if (bitmap)
    {
        bitmap->GetHBITMAP(NULL, &hBitmap);
        delete bitmap;
    }

    DWORD lastCheckTime = GetTickCount();
    while(true)
    {
        if (hBitmap != NULL)
        {
            scSendBitmapFrame(cam, (scBitmap)hBitmap);
        }
        else
        {
            // Draw bouncing balls.
            balls.move(1.0f / 60.0f);
            balls.draw(image.data());

            // Send the image as a newly captured frame of the camera.
            scSendFrame(cam, image.data(), (int)image.size());
        }        

        if (GetTickCount() - lastCheckTime >= 3000)
        {
            unsigned int pids[10];
            int count = scGetCameraUsers(pids, ARRAYSIZE(pids));
            if (count == 0)
            {
                std::printf("not any in use\n");
            }
            else
            {
                for (int i = 0; i < count; i++)
                {
                    std::printf("%d in use\n", pids[i]);
                }
            }
            lastCheckTime = GetTickCount();
        }
    }

    // Delete the camera instance.
    // The receiver application will no longer receive new frames from this camera.
    scDeleteCamera(cam);
    std::printf("Softcam has been shut down.\n");
}
