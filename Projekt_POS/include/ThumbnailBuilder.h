#pragma once

#include <string>
#include <vector>
#include <opencv2/core.hpp>

/**
 * @file ThumbnailBuilder.h
 * @brief Budowanie mozaiki miniatur z listy obrazów.
 */

/**
 * @class ThumbnailBuilder
 * @brief Scala listę obrazów w siatkę miniatur (mozaikę).
 *
 * Miniatury mają zachowane proporcje wysokość/szerokość.
 * Brakujące komórki siatki wypełniane są czernią.
 *
 * Przykład użycia:
 * @code
 * ThumbnailBuilder tb(128, 4);
 * cv::Mat mosaic = tb.build(imagePaths);
 * cv::imwrite("mosaic.png", mosaic);
 * @endcode
 */
class ThumbnailBuilder
{
public:
    /**
     * @brief Konstruktor.
     * @param tileSize  Rozmiar boku pojedynczej komórki siatki [px].
     * @param cols      Liczba kolumn siatki (0 = automatycznie: sqrt(n)).
     */
    explicit ThumbnailBuilder(int tileSize, int cols = 0);

    /**
     * @brief Buduje mozaikę miniatur ze ścieżek do plików.
     * @param imagePaths Lista ścieżek do obrazów źródłowych lub wynikowych.
     * @return Obraz mozaiki (CV_8UC3). Pusty przy pustej liście.
     */
    cv::Mat build(const std::vector<std::string>& imagePaths) const;

private:
    int m_tileSize;  ///< Rozmiar komórki siatki [px].
    int m_cols;      ///< Liczba kolumn (0 = auto).

    /**
     * @brief Skaluje obraz do kwadratu @p tileSize × @p tileSize
     *        z zachowaniem proporcji (letterbox czernią).
     * @param img Obraz wejściowy.
     * @return Kafelek o rozmiarze tileSize × tileSize.
     */
    cv::Mat makeTile(const cv::Mat& img) const;
};
