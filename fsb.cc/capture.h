#ifndef CAPTURE_H
#define CAPTURE_H

#include <opencv2/opencv.hpp>
#include <Windows.h>
#include <iostream>

class Capture {
private:
    RECT monitor;
    cv::Scalar lower_color;  // Zakres dolny koloru
    cv::Scalar upper_color;  // Zakres górny koloru

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

    void setColorRanges(const cv::Scalar& lower, const cv::Scalar& upper) {
        lower_color = lower;
        upper_color = upper;
    }

    cv::Mat get_screen() {
        HDCWrapper hScreenDC(GetDC(NULL));
        HDCWrapper hMemoryDC(CreateCompatibleDC(hScreenDC.hdc));

        int width = monitor.right - monitor.left;
        int height = monitor.bottom - monitor.top;

        HBITMAPWrapper hBitmap(CreateCompatibleBitmap(hScreenDC.hdc, width, height));
        if (!hBitmap.hbitmap) {
            std::cerr << "CreateCompatibleBitmap failed" << std::endl;
            return cv::Mat();
        }

        HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemoryDC.hdc, hBitmap.hbitmap);
        if (!hOldBitmap) {
            std::cerr << "SelectObject failed" << std::endl;
            return cv::Mat();
        }

        if (!BitBlt(hMemoryDC.hdc, 0, 0, width, height, hScreenDC.hdc, monitor.left, monitor.top, SRCCOPY)) {
            std::cerr << "BitBlt failed" << std::endl;
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
            std::cerr << "GetDIBits failed" << std::endl;
            return cv::Mat();
        }

        cv::cvtColor(mat, mat, cv::COLOR_BGRA2BGR); // Convert BGRA to BGR

        // Image conversion to HSV space
        cv::Mat hsv_image;
        cv::cvtColor(mat, hsv_image, cv::COLOR_BGR2HSV);

        // Create a mask for a color using preset ranges
        cv::Mat mask;
        cv::inRange(hsv_image, lower_color, upper_color, mask);

        // Morphological operations to eliminate noise
        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
        cv::dilate(mask, mask, kernel);
        cv::erode(mask, mask, kernel);

        // Applying the mask to the original image
        cv::Mat result;
        cv::bitwise_and(mat, mat, result, mask);

        // Add Gaussian blur for better noise filtering
        cv::GaussianBlur(result, result, cv::Size(5, 5), 0);

        return result;
    }
};

#endif // CAPTURE_H
