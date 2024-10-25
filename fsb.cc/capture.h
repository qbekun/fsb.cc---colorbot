#ifndef CAPTURE_H
#define CAPTURE_H

#include <opencv2/opencv.hpp>
#include <Windows.h>
#include <iostream>
#include <thread>
#include <mutex>

class Capture {
private:
    RECT monitor;
    cv::Scalar lower_color;  // Dolny zakres kolorów
    cv::Scalar upper_color;  // Górny zakres kolorów

public:
    Capture(int x, int y, int x_fov, int y_fov) {
        monitor = { x, y, x + x_fov, y + y_fov }; // Inicjalizacja prostokąta
    }

    void setColorRanges(const cv::Scalar& lower, const cv::Scalar& upper) {
        lower_color = lower;
        upper_color = upper;
    }

    cv::Mat get_screen() {
        const int width = monitor.right - monitor.left;
        const int height = monitor.bottom - monitor.top;

        // Przechwytywanie ekranu
        HDC hScreenDC = GetDC(NULL);
        HDC hMemoryDC = CreateCompatibleDC(hScreenDC);
        HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, width, height);
        HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemoryDC, hBitmap);

        BitBlt(hMemoryDC, 0, 0, width, height, hScreenDC, monitor.left, monitor.top, SRCCOPY);

        // Przygotowanie nagłówka informacji bitmapy
        BITMAPINFOHEADER bi = {};
        bi.biSize = sizeof(BITMAPINFOHEADER);
        bi.biWidth = width;
        bi.biHeight = -height; // Ujemna wysokość, aby obraz był właściwie ustawiony
        bi.biPlanes = 1;
        bi.biBitCount = 32;
        bi.biCompression = BI_RGB;

        cv::Mat mat(height, width, CV_8UC4);
        GetDIBits(hMemoryDC, hBitmap, 0, height, mat.data, (BITMAPINFO*)&bi, DIB_RGB_COLORS);

        // Sprzątanie obiektów GDI
        SelectObject(hMemoryDC, hOldBitmap);
        DeleteObject(hBitmap);
        DeleteDC(hMemoryDC);
        ReleaseDC(NULL, hScreenDC);

        // Konwersja BGRA na BGR
        cv::cvtColor(mat, mat, cv::COLOR_BGRA2BGR);

        // Przetwarzanie obrazu w HSV
        cv::Mat hsv_image;
        cv::cvtColor(mat, hsv_image, cv::COLOR_BGR2HSV);

        // Tworzenie maski dla koloru
        cv::Mat mask;
        cv::inRange(hsv_image, lower_color, upper_color, mask);

        // Operacje morfologiczne, aby zlikwidować szum
        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
        cv::morphologyEx(mask, mask, cv::MORPH_CLOSE, kernel); // Użycie operacji zamknięcia

        // Zastosowanie maski do oryginalnego obrazu
        cv::Mat result;
        cv::bitwise_and(mat, mat, result, mask);

        // Add Gaussian blur for better noise filtering
        cv::GaussianBlur(result, result, cv::Size(5, 5), 0);

        return result; // Zwrócenie przetworzonego obrazu
    }
};

#endif // CAPTURE_H
