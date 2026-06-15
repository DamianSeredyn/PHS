#pragma once

#include <string>

/**
 * @file ConfigReader.h
 * @brief Odczyt konfiguracji z pliku ini
 */

 /**
  * @struct AppConfig
  * @brief Struktura przechowująca konfigurację aplikacji
  */
struct AppConfig
{
    std::string sourceDir;      ///< Ścieżka folderu z obrazami
    std::string destDir;        ///< Ścieżka folderu wyjściowego
    int         numThreads;     ///< Liczba wątków
    int         thumbnailSize;  ///< Rozmiar miniatury
    int         cannyLow;       ///< Dolny próg Canny
    int         cannyHigh;      ///< Górny próg Canny
};

/**
 * @class ConfigReader
 * @brief Parsuje plik INI i zwraca strukturę AppConfig
 */
class ConfigReader
{
public:
    /**
     * @brief wczytuje plik ini
     * @param iniPath Ścieżka do pliku ini
     * @return Wypełniona struktura AppConfig
     * @throws std::runtime_error gdy plik nie istnieje lub nie ma pozycji w ini
     */
    static AppConfig read(const std::string& iniPath);
};
