/**
 * @file ThumbnailBuilder.cpp
 * @brief Implementacja budowania  miniatur
 */

#include "ThumbnailBuilder.h"

#include <cmath>
#include <iostream>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

// ---------------------------------------------------------------------------

ThumbnailBuilder::ThumbnailBuilder(int tileSize, int cols)
    : m_tileSize(tileSize), m_cols(cols)
{}

// ---------------------------------------------------------------------------

cv::Mat ThumbnailBuilder::makeTile(const cv::Mat& img) const
{
    cv::Mat tile(m_tileSize, m_tileSize, CV_8UC3, cv::Scalar(0, 0, 0));     // Kafelek docelowy czarne tło, 3 kanały BGR


    if (img.empty()) return tile;

    cv::Mat src;    // Konwersja jednokanałowego (krawędzie Canny) na BGR aby mozaika była spójna

    if (img.channels() == 1)
        cv::cvtColor(img, src, cv::COLOR_GRAY2BGR);
    else
        src = img;

    // Skalowanie z zachowaniem proporcji (fit inside square)
    double scaleW = static_cast<double>(m_tileSize) / src.cols;
    double scaleH = static_cast<double>(m_tileSize) / src.rows;
    double scale  = std::min(scaleW, scaleH);

    int newW = static_cast<int>(src.cols * scale);
    int newH = static_cast<int>(src.rows * scale);

    cv::Mat resized;
    cv::resize(src, resized, cv::Size(newW, newH), 0, 0, cv::INTER_AREA);

    // Wyśrodkowanie na czarnym tle 
    int offX = (m_tileSize - newW) / 2;
    int offY = (m_tileSize - newH) / 2;
    resized.copyTo(tile(cv::Rect(offX, offY, newW, newH)));

    return tile;
}

// ---------------------------------------------------------------------------

cv::Mat ThumbnailBuilder::build(const std::vector<std::string>& imagePaths) const
{
    if (imagePaths.empty()) return {};

    int n    = static_cast<int>(imagePaths.size());
    int cols = (m_cols > 0) ? m_cols
                             : static_cast<int>(std::ceil(std::sqrt(n)));
    int rows = static_cast<int>(std::ceil(static_cast<double>(n) / cols));

    // Alokuj cala mozaike z gory
    cv::Mat mosaic(rows * m_tileSize, cols * m_tileSize, CV_8UC3,
                   cv::Scalar(0, 0, 0));

    for (int i = 0; i < n; ++i)
    {
        cv::Mat img = cv::imread(imagePaths[i], cv::IMREAD_ANYCOLOR);
        if (img.empty())
        {
            std::cerr << "[WARN] Mozaika: pominieto " << imagePaths[i] << "\n";
            continue;
        }

        cv::Mat tile = makeTile(img);

        int row = i / cols;
        int col = i % cols;
        tile.copyTo(mosaic(cv::Rect(col * m_tileSize, row * m_tileSize,
                                    m_tileSize, m_tileSize)));
    }

    return mosaic;
}
