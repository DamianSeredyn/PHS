/**
 * @file main.cpp
 * @brief glowny plik programu
 *
 * Uruchamianie:
 * @code
 * imgproc.exe [sciezka_do_config.ini]
 * @endcode
 * Domyslna sciezka do pliku INI: @c config.ini (obok pliku wykonywalnego).
 *
 * Program:
 * -# Wczytuje konfigurację z pliku INI
 * -# Skanuje folder zrodlowy w poszukiwaniu obrazow
 * -# Przetwarza obrazy wielowatkowo
 * -# Zapisuje wyniki
 * -# Generuje dwa pliki zrodlowa i wynikowa
 */

#include <iostream>
#include <filesystem>
#include <atomic>
#include <mutex>
#include <chrono>
#include <iomanip>

#include "ConfigReader.h"
#include "ImageProcessor.h"
#include "ThumbnailBuilder.h"
#include "ThreadPool.h"
#include <opencv2/imgcodecs.hpp>

namespace fs = std::filesystem;

// Pomocnicze: pasek postepu w konsoli


/**
 * @brief Generuje prosty pasek postępu w konsoli
 * @param done    Liczba zakończonych zadań
 * @param total   Całkowita liczba zadań
 * @param mutex   Mutex chroniący dostęp do std::cout.
 */
static void printProgress(int done, int total, std::mutex& mutex)
{
    std::lock_guard<std::mutex> lock(mutex);
    int pct = (total > 0) ? (done * 100 / total) : 100;
    std::cout << "\r  Postęp: [";
    int filled = pct / 5;
    for (int i = 0; i < 20; ++i)
        std::cout << (i < filled ? '#' : '.');
    std::cout << "] " << std::setw(3) << pct << "% ("
              << done << "/" << total << ")" << std::flush;
}


int main(int argc, char* argv[])
{
   
    std::string iniPath = (argc > 1) ? argv[1] : "config.ini"; // sciezka confgu

    std::cout << " Wielowatkowy procesor obrazow\n";
    std::cout << "Konfiguracja: " << iniPath << "\n\n";

    AppConfig cfg;
    try
    {
        cfg = ConfigReader::read(iniPath);
    }
    catch (const std::exception& e)
    {
        std::cerr << "[BLAD] " << e.what() << "\n";
        return 1;
    }

    std::cout << "  Zrodlo    : " << cfg.sourceDir     << "\n";
    std::cout << "  Cel       : " << cfg.destDir       << "\n";
    std::cout << "  Watki     : " << cfg.numThreads    << "\n";
    std::cout << "  Canny     : " << cfg.cannyLow
              << " / "            << cfg.cannyHigh     << "\n";
    std::cout << "  Miniatura : " << cfg.thumbnailSize << "px\n\n";

    ImageProcessor processor(cfg.cannyLow, cfg.cannyHigh);  //Lista plików źródłowych (obrazy)
    std::vector<std::string> srcPaths;
    try
    {
        srcPaths = processor.listImages(cfg.sourceDir);
    }
    catch (const std::exception& e)
    {
        std::cerr << "[BLAD] " << e.what() << "\n";
        return 1;
    }

    if (srcPaths.empty())
    {
        std::cout << "[INFO] Brak plikow graficznych w: " << cfg.sourceDir << "\n";
        return 0;
    }
    std::cout << "Znaleziono " << srcPaths.size() << " plikow.\n\n";

    // Utwórz folder docelowy jeśli nie istnieje
    fs::create_directories(cfg.destDir);

    // 4. Wielowątkowe przetwarzanie 
    std::vector<std::string> dstPaths(srcPaths.size());
    std::atomic<int>         doneCount{0};
    std::mutex               coutMutex;
    auto                     t0 = std::chrono::steady_clock::now();

    {
        ThreadPool pool(cfg.numThreads);

        for (size_t i = 0; i < srcPaths.size(); ++i)
        {

            fs::path srcFile(srcPaths[i]);             // Wyznaczamy ścieżkę docelową
            fs::path dstFile = fs::path(cfg.destDir) / srcFile.filename();
            dstPaths[i] = dstFile.string();

            pool.submit([i, srcPath = srcPaths[i], dstPath = dstPaths[i],
                         &processor, &doneCount, &coutMutex,
                         total = (int)srcPaths.size()]()
            {
                processor.processImage(srcPath, dstPath);
                int done = ++doneCount;
                printProgress(done, total, coutMutex);
            });
        }

        pool.waitAll();
    }

    auto t1 = std::chrono::steady_clock::now();
    double elapsed = std::chrono::duration<double>(t1 - t0).count();
    std::cout << "\n\nPrzetworzono " << srcPaths.size()
              << " plikow w " << std::fixed << std::setprecision(2)
              << elapsed << " s.\n\n";

    ThumbnailBuilder tb(cfg.thumbnailSize); // generujemy miniaturki

    std::cout << "Generowanie mozaiki zrodłowej...";
    cv::Mat mosaicSrc = tb.build(srcPaths);
    fs::path mosaicSrcPath = fs::path(cfg.destDir) / "_mozaika_zrodlowa.png";
    if (!mosaicSrc.empty())
    {
        cv::imwrite(mosaicSrcPath.string(), mosaicSrc);
        std::cout << " OK -> " << mosaicSrcPath << "\n";
    }

    std::cout << "Generowanie mozaiki wynikowej... ";
    cv::Mat mosaicDst = tb.build(dstPaths);
    fs::path mosaicDstPath = fs::path(cfg.destDir) / "_mozaika_wynikowa.png";
    if (!mosaicDst.empty())
    {
        cv::imwrite(mosaicDstPath.string(), mosaicDst);
        std::cout << "OK -> " << mosaicDstPath << "\n";
    }

    std::cout << "\nGotowe.\n";
    return 0;
}
