#include <string>
#include <iostream>
#include <io.h>
#include <vector>
#include <chrono>
#include <iomanip>

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

void DoTest(std::vector<std::string> args);
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

    std::vector<std::string> args;
    for (int i = 1; i < argc; ++i)
        args.push_back(argv[i]);

    DoTest(args);

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

void CalculateTextureObjectPixels(GLuint texID, unsigned long *pixelCount, unsigned long *memoryUsed)
{
    int baseLevel = 0, maxLevel = 0;
    glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, &baseLevel);
    glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, &maxLevel);

    long pixels = 0, bytes = 0, bpp = 0;
    int texW = 0, texH = 0, texFmt = 0, compressed = 0, compressedBytes = 0;
    for (int level = baseLevel; level <= maxLevel; ++level)
    {
        glGetTexLevelParameteriv(GL_TEXTURE_2D, level, GL_TEXTURE_WIDTH, &texW);
        glGetTexLevelParameteriv(GL_TEXTURE_2D, level, GL_TEXTURE_HEIGHT, &texH);
        glGetTexLevelParameteriv(GL_TEXTURE_2D, level, GL_TEXTURE_INTERNAL_FORMAT, &texFmt);
        glGetTexLevelParameteriv(GL_TEXTURE_2D, level, GL_TEXTURE_COMPRESSED, &compressed);
        pixels += texW*texH;

        if (compressed == GL_TRUE)
        {
            glGetTexLevelParameteriv(GL_TEXTURE_2D, level, GL_TEXTURE_COMPRESSED_IMAGE_SIZE, &compressedBytes);
            bytes += compressedBytes;
        }
        else
        {
            if (texFmt == GL_RGB || texFmt == GL_RGB8)
                bpp = 3;
            else if (texFmt == GL_RGBA || texFmt == GL_RGBA8)
                bpp = 4;
            else
                bpp = 0;    

            bytes += texW*texH*bpp;
        }

        if (texW == 1 && texH == 1)
            break;
    }
    *pixelCount += pixels;
    *memoryUsed += bytes;
}

struct TestParams
{
    std::vector<std::string> files;
    int numberOfLoads;
    unsigned int soilFlags;
};

struct TestOutputs
{
    TestOutputs() : durationInMsec(0), totalPixels(0), totalMemoryInBytes(0) { }
    
    long durationInMsec;
    unsigned long totalPixels;
    unsigned long totalMemoryInBytes;
};

TestOutputs LoadTest(const TestParams &params)
{
    const size_t numFiles = params.files.size();
    std::vector<GLuint> texID(params.numberOfLoads);
  
    TestOutputs outputs;

    for (int i = 0; i < params.numberOfLoads; ++i)
    {
        auto t_start = std::chrono::high_resolution_clock::now();
        {
            texID[i] = SOIL_load_OGL_texture(params.files[i%numFiles].c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, params.soilFlags);
        }
        auto t_end = std::chrono::high_resolution_clock::now();
        outputs.durationInMsec += static_cast<long>(std::chrono::duration_cast<std::chrono::milliseconds>(t_end - t_start).count());
        
        if (texID[i] == 0)
            std::cout << "error!, could not load " << params.files[i%numFiles] << std::endl;

        CalculateTextureObjectPixels(texID[i], &outputs.totalPixels, &outputs.totalMemoryInBytes);
    }

    for (int i = 0; i < params.numberOfLoads; ++i)
        glDeleteTextures(1, &texID[i]);

    return outputs;
}

void BuildFileLIst(std::vector<std::string> *files, std::string pattern)
{
    WIN32_FIND_DATA ffd;
    HANDLE hFind;

    hFind = FindFirstFile(pattern.c_str(), &ffd);

    if (INVALID_HANDLE_VALUE == hFind) 
    {
        std::cout << "Error..." << std::endl;
        return;
    } 

    do
    {
        if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
            std::cout << ffd.cFileName << std::endl;
            files->push_back(ffd.cFileName);
        }
    }
    while (FindNextFile(hFind, &ffd) != 0);

    FindClose(hFind);
}

void DoTest(std::vector<std::string> args)
{
    const int NUM_FILES = 3;
    const char *defaultFiles[NUM_FILES] = { "lenna1.jpg", "lenna2.jpg", "lenna3.jpg" };
    std::vector<std::string> files;

    const int NUM_LOADS = args.size() > 0 ? atoi(args[0].c_str()) : 50;
    std::cout << "num of loads: " << NUM_LOADS << std::endl;
    
    if (args.size() > 1)
        BuildFileLIst(&files, args[1]);
    
    if (files.empty())
    {
        for (int i = 0; i < NUM_FILES; ++i)
            files.push_back(defaultFiles[i]);
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // GL_MIPMAP
    {
        TestParams paramsMipGL;
        paramsMipGL.files = files;
        paramsMipGL.numberOfLoads = NUM_LOADS;
        paramsMipGL.soilFlags = SOIL_FLAG_GL_MIPMAPS;

        TestOutputs resultMipGL = LoadTest(paramsMipGL);

        double durationInSec = (double)(resultMipGL.durationInMsec*0.001);
        double pixSpeed = resultMipGL.totalPixels/durationInSec;
        double memInMB  = resultMipGL.totalMemoryInBytes/(double)(1024.0*1024);
        double memSpeed = memInMB/durationInSec;

        printf("FLAG_GL_MIPMAPS: %2.2fsec, memory: %3.2f MB, speed %3.3f MB/s\n", 
              (float)(durationInSec), (float)memInMB, (float)memSpeed);
    }

    // MIPMAP soil version
    {
        TestParams paramsMip;
        paramsMip.files = files;
        paramsMip.numberOfLoads = NUM_LOADS;
        paramsMip.soilFlags = SOIL_FLAG_MIPMAPS;

        TestOutputs resultMip = LoadTest(paramsMip);

        double durationInSec = (double)(resultMip.durationInMsec*0.001);
        double pixSpeed = resultMip.totalPixels/durationInSec;
        double memInMB  = resultMip.totalMemoryInBytes/(double)(1024.0*1024);
        double memSpeed = memInMB/durationInSec;

        printf("FLAG_MIPMAPS: %2.2fsec, memory: %3.2f MB, speed %3.3f MB/s\n", 
              (float)(durationInSec), (float)memInMB, (float)memSpeed);
    }
}