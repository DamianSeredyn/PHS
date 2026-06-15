#include "ConfigReader.h"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <thread>
#include <map>

static std::string trim(const std::string& text)
{
    const std::string whitespace = " \t\r\n";

    size_t first = text.find_first_not_of(whitespace);
    if (first == std::string::npos) {
        return "";
    }

    size_t last = text.find_last_not_of(whitespace);
    return text.substr(first, last - first + 1);
}

static std::string removeComment(const std::string& text)
{
    size_t pos = text.find(';');

    if (pos == std::string::npos) {
        pos = text.size();
    }

    return trim(text.substr(0, pos));
}

static int toInt(const std::string& value, int defaultValue)
{
    try {
        return std::stoi(value);
    }
    catch (...) {
        return defaultValue;
    }
}

AppConfig ConfigReader::read(const std::string& iniPath)
{
    std::ifstream file(iniPath);
    if (!file.is_open()) {
        throw std::runtime_error("Nie mozna otworzyc pliku: " + iniPath);
    }

    std::map<std::string, std::string> values;
    std::string section;
    std::string line;

    while (std::getline(file, line)) {
        line = trim(line);

        if (line.empty() || line[0] == ';' || line[0] == '#') {
            continue;
        }

        if (line.front() == '[' && line.back() == ']') {
            section = trim(line.substr(1, line.size() - 2));
            continue;
        }

        size_t eq = line.find('=');
        if (eq == std::string::npos) {
            continue;
        }

        std::string key = trim(line.substr(0, eq));
        std::string value = removeComment(line.substr(eq + 1));

        values[section + "." + key] = value;
    }

    auto getString = [&](const std::string& key) -> std::string {
        if (!values.count(key)) {
            throw std::runtime_error("Brak pola w configu: " + key);
        }

        return values[key];
        };

    auto getInt = [&](const std::string& key, int defaultValue) -> int {
        if (!values.count(key)) {
            return defaultValue;
        }

        return toInt(values[key], defaultValue);
        };

    AppConfig cfg;

    cfg.sourceDir = getString("paths.source_dir");
    cfg.destDir = getString("paths.dest_dir");

    int defaultThreads = static_cast<int>(std::thread::hardware_concurrency());
    if (defaultThreads < 1) {
        defaultThreads = 1;
    }

    cfg.numThreads = getInt("processing.num_threads", defaultThreads);
    cfg.thumbnailSize = getInt("processing.thumbnail_size", 128);
    cfg.cannyLow = getInt("processing.canny_low", 50);
    cfg.cannyHigh = getInt("processing.canny_high", 150);

    if (cfg.numThreads < 1) {
        cfg.numThreads = 1;
    }

    if (cfg.thumbnailSize < 16) {
        cfg.thumbnailSize = 16;
    }

    if (cfg.cannyLow < 10) {
        cfg.cannyLow = 10;
    }

    if (cfg.cannyHigh > 150 || cfg.cannyHigh < cfg.cannyLow) {
        cfg.cannyHigh = 150;
    }

    return cfg;
}