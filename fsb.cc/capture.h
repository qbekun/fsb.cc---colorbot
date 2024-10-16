#ifndef CAPTURE_H
#define CAPTURE_H

#include <opencv2/opencv.hpp>
#include <Windows.h>

class Capture {
private:
    RECT monitor;

    class HDCWrapper {
    public:
        HDC hdc;
        HDCWrapper(HDC hdc) : hdc(hdc) {}
        ~HDCWrapper() {
            if (hdc) DeleteDC(hdc);
        }
    };

    class HBITMAPWrapper {
    public:
        HBITMAP hbitmap;
        HBITMAPWrapper(HBITMAP hbitmap) : hbitmap(hbitmap) {}
        ~HBITMAPWrapper() {
            if (hbitmap) DeleteObject(hbitmap);
        }
    };

public:
    Capture(int x, int y, int x_fov, int y_fov) {
        monitor.left = x;
        monitor.top = y;
        monitor.right = x + x_fov;
        monitor.bottom = y + y_fov;
    }

    cv::Mat get_screen() {
        HDCWrapper hScreenDC(GetDC(NULL));
        HDCWrapper hMemoryDC(CreateCompatibleDC(hScreenDC.hdc));
        int width = monitor.right - monitor.left;
        int height = monitor.bottom - monitor.top;

        HBITMAPWrapper hBitmap(CreateCompatibleBitmap(hScreenDC.hdc, width, height));
        if (!hBitmap.hbitmap) {
            return cv::Mat();
        }

        HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemoryDC.hdc, hBitmap.hbitmap);
        if (!hOldBitmap) {
            return cv::Mat();
        }

        if (!BitBlt(hMemoryDC.hdc, 0, 0, width, height, hScreenDC.hdc, monitor.left, monitor.top, SRCCOPY)) {
            return cv::Mat();
        }

        BITMAPINFOHEADER bi;
        bi.biSize = sizeof(BITMAPINFOHEADER);
        bi.biWidth = width;
        bi.biHeight = -height;
        bi.biPlanes = 1;
        bi.biBitCount = 32;
        bi.biCompression = BI_RGB;
        bi.biSizeImage = 0;
        bi.biXPelsPerMeter = 0;
        bi.biYPelsPerMeter = 0;
        bi.biClrUsed = 0;
        bi.biClrImportant = 0;

        cv::Mat mat(height, width, CV_8UC4);
        if (!GetDIBits(hMemoryDC.hdc, hBitmap.hbitmap, 0, height, mat.data, (BITMAPINFO*)&bi, DIB_RGB_COLORS)) {
            return cv::Mat();
        }

        return mat;
    }
};

#endif // CAPTURE_H
