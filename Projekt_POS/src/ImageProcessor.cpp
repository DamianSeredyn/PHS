/**
 * @file ImageProcessor.cpp
 * @brief Implementacja wczytywania, przetwarzania i zapisu obrazow.
 */

#include "ImageProcessor.h"

#include <filesystem>
#include <stdexcept>
#include <algorithm>
#include <iostream>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

namespace fs = std::filesystem;


static const std::vector<std::string> IMAGE_EXTENSIONS =
    { ".png", ".jpg", ".jpeg", ".bmp", ".tiff", ".tif" }; // ograniczamy pliki dopuszczalne do edycji

/** @brief Sprawdza czy rozszerzenie pliku nalezy do dozwolonych formatow. */
static bool isImageFile(const fs::path& p)
{
    std::string ext = p.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    for (const auto& e : IMAGE_EXTENSIONS)
        if (ext == e) return true;
    return false;
}


ImageProcessor::ImageProcessor(int cannyLow, int cannyHigh)
    : m_cannyLow(cannyLow), m_cannyHigh(cannyHigh)
{}


std::vector<std::string> ImageProcessor::listImages(const std::string& dirPath) const
{
    fs::path dir(dirPath);
    if (!fs::exists(dir) || !fs::is_directory(dir))
        throw std::runtime_error("Folder zrodlowy nie istnieje: " + dirPath);

    std::vector<std::string> paths;

    // Rekurencyjne przejscie przez caly drzewo folderow
    for (const auto& entry : fs::recursive_directory_iterator(dir,
             fs::directory_options::skip_permission_denied))
    {
        if (entry.is_regular_file() && isImageFile(entry.path()))
            paths.push_back(entry.path().string());
    }

    std::sort(paths.begin(), paths.end());
    return paths;
}


cv::Mat ImageProcessor::loadImage(const std::string& path) const
{
    // IMREAD_COLOR 3 kanały BGR
    cv::Mat img = cv::imread(path, cv::IMREAD_COLOR);
    if (img.empty())
        std::cerr << "[WARN] Nie udalo się wczytac: " << path << "\n";
    return img;
}


cv::Mat ImageProcessor::detectEdges(const cv::Mat& src) const
{
    if (src.empty()) return {};

    cv::Mat gray, blurred, edges;

    if (src.channels() == 1)
        gray = src; // Konwersja do skali szarości 
    else
        cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);


    cv::GaussianBlur(gray, blurred, cv::Size(5, 5), 0); // Rozmycie Gaussowskie redukuje szum przed detekcją krawedzi

    // Algorytm Canny
    // m_cannyLow  piksele ponizej progu są odrzucane
    // m_cannyHigh piksele powyzej progu są zawsze krawedzia
    // Piksele pomiedzy progami są krawedzia tylko jesli sasiaduja z pewna krawedzia
    cv::Canny(blurred, edges, m_cannyLow, m_cannyHigh);

    return edges;
}

// ---------------------------------------------------------------------------

bool ImageProcessor::processImage(const std::string& srcPath,
                                  const std::string& dstPath) const
{
    cv::Mat src = loadImage(srcPath);
    if (src.empty()) return false;

    cv::Mat edges = detectEdges(src);
    if (edges.empty()) return false;

    fs::path dstDir = fs::path(dstPath).parent_path(); 
    if (!dstDir.empty()) // Czy istnieje?
        fs::create_directories(dstDir);

    if (!cv::imwrite(dstPath, edges))
    {
        std::cerr << "[WARN] Nie udalo się zapisac: " << dstPath << "\n";
        return false;
    }
    return true;
}
