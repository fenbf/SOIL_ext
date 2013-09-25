#include <string>
#include <iostream>
#include <io.h>
#include <vector>
#include <chrono>

#include <windows.h>
#include <shellapi.h>
#include <ctime>
#include <gl/gl.h>

#include "glext.h"
#include "SOIL.h"

///////////////////////////////
// prototypes:
LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
void EnableOpenGL(HWND hwnd, HDC*, HGLRC*);
void DisableOpenGL(HWND, HDC, HGLRC);

void DoTest();
void Render();

///////////////////////////////
int main(int argc, char *argv[])
{
    WNDCLASSEX wcex;
    HWND hwnd;
    HDC hDC;
    HGLRC hRC;
    BOOL bQuit = FALSE;

    // register window class
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_OWNDC;
    wcex.lpfnWndProc = WindowProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = GetModuleHandle (0);
    wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = "GLSample";
    wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClassEx(&wcex))
        return 0;

    // create main window
    hwnd = CreateWindowEx(0,
        "GLSample",
        "SOIL Perf Sample",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        512,
        512,
        NULL,
        NULL,
        GetModuleHandle(0),
        NULL);

    // enable OpenGL for the window
    EnableOpenGL(hwnd, &hDC, &hRC);

    DoTest();

    // shutdown OpenGL
    DisableOpenGL(hwnd, hDC, hRC);

    // destroy the window explicitly
    DestroyWindow(hwnd);

    //system("PAUSE");

    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CLOSE:
        PostQuitMessage(0);
        break;

    case WM_DESTROY:
        return 0;

    case WM_KEYDOWN:
        {
            switch (wParam)
            {
            case VK_ESCAPE:
                PostQuitMessage(0);
                break;
            }
        }
        break;

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}

void EnableOpenGL(HWND hwnd, HDC* hDC, HGLRC* hRC)
{
    PIXELFORMATDESCRIPTOR pfd;

    int iFormat;

    /* get the device context (DC) */
    *hDC = GetDC(hwnd);

    /* set the pixel format for the DC */
    ZeroMemory(&pfd, sizeof(pfd));

    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 16;
    pfd.iLayerType = PFD_MAIN_PLANE;

    iFormat = ChoosePixelFormat(*hDC, &pfd);

    SetPixelFormat(*hDC, iFormat, &pfd);

    /* create and enable the render context (RC) */
    *hRC = wglCreateContext(*hDC);

    wglMakeCurrent(*hDC, *hRC);
    glEnable( GL_TEXTURE_2D );
}

void DisableOpenGL (HWND hwnd, HDC hDC, HGLRC hRC)
{
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hRC);
    ReleaseDC(hwnd, hDC);
}

void LoadTest(const char *files[], const int numFiles, const int numLoads, GLuint *texID, unsigned int soilFlags, long *duration)
{
    auto t_start = std::chrono::high_resolution_clock::now();
  
    for (int i = 0; i < numLoads; ++i)
    {
        texID[i] = SOIL_load_OGL_texture(files[i%numFiles], SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, soilFlags);
        
        if (texID[i] == 0)
            std::cout << "error!, could not load " << files[i%numFiles] << std::endl;
    }

    auto t_end = std::chrono::high_resolution_clock::now();

    *duration = static_cast<long>(std::chrono::duration_cast<std::chrono::milliseconds>(t_end - t_start).count());
}

void DoTest()
{
    const int NUM_FILES = 3;
    const char *files[NUM_FILES] = { "test1.jpg", "test2.jpg", "test3.jpg" };

    const int NUM_LOADS = 100;    
    
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    long duration = 0;
    unsigned int texID[NUM_LOADS] = { 0 };
    LoadTest(files, NUM_FILES, NUM_LOADS, texID, SOIL_FLAG_GL_MIPMAPS, &duration);
    std::cout << "SOIL_FLAG_GL_MIPMAPS : " << duration << "ms" << std::endl;

    for (int i = 0; i < NUM_LOADS; ++i)
        glDeleteTextures(1, &texID[i]);

    LoadTest(files, NUM_FILES, NUM_LOADS, texID, SOIL_FLAG_MIPMAPS, &duration);
    std::cout << "SOIL_FLAG_MIPMAPS    : " << duration << "ms" << std::endl;

    for (int i = 0; i < NUM_LOADS; ++i)
        glDeleteTextures(1, &texID[i]);
}