#pragma once

#include <string>
#include <vector>
#include <opencv2/core.hpp>

/**
 * @file ImageProcessor.h
 * @brief Ładowanie, przetwarzanie i zapis obrazów.
 */

/**
 * @class ImageProcessor
 * @brief Obsługuje wczytywanie obrazów ze źródła, detekcję konturów
 *        algorytmem Canny oraz zapis wyników do folderu docelowego.
 */
class ImageProcessor
{
public:
    /**
     * @brief Konstruktor.
     * @param cannyLow   Dolny próg histerezy algorytmu Canny.
     * @param cannyHigh  Górny próg histerezy algorytmu Canny.
     */
    ImageProcessor(int cannyLow, int cannyHigh);

    /**
     * @brief Rekurencyjnie zbiera ścieżki do wszystkich plików graficznych
     *        w podanym folderze (png, jpg, jpeg, bmp, tiff, tif).
     * @param dirPath Ścieżka do przeszukiwanego folderu.
     * @return Wektor bezwzględnych ścieżek do plików.
     * @throws std::runtime_error gdy folder nie istnieje.
     */
    std::vector<std::string> listImages(const std::string& dirPath) const;

    /**
     * @brief Przetwarza pojedynczy obraz: wczytuje go, stosuje detekcję
     *        konturów (Canny) i zapisuje wynik pod wskazaną ścieżką.
     * @param srcPath  Ścieżka do pliku wejściowego.
     * @param dstPath  Ścieżka zapisu przetworzonego obrazu.
     * @return @c true przy sukcesie, @c false przy błędzie I/O lub dekodowania.
     */
    bool processImage(const std::string& srcPath, const std::string& dstPath) const;

    /**
     * @brief Wczytuje obraz z dysku.
     * @param path Ścieżka do pliku.
     * @return Obiekt cv::Mat; pusty jeśli wczytanie się nie powiodło.
     */
    cv::Mat loadImage(const std::string& path) const;

    /**
     * @brief Stosuje algorytm Canny na obrazie.
     * @param src Obraz wejściowy (dowolna liczba kanałów).
     * @return Jednokanałowy obraz krawędzi (CV_8U).
     */
    cv::Mat detectEdges(const cv::Mat& src) const;

private:
    int m_cannyLow;   ///< Dolny próg Canny.
    int m_cannyHigh;  ///< Górny próg Canny.
};
