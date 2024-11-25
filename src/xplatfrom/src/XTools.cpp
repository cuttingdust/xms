#include "XTools.h"

#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

std::string XTools::getDirData(std::string path)
{
    std::string data = "";
    try
    {
        for (const auto &entry : fs::directory_iterator(path))
        {
            std::string tmp = "";
            if (entry.is_directory())
            {
                continue;
            }

            const auto &file     = entry.path();
            const auto &fileName = file.filename().string();
            const auto &fileSize = fs::file_size(file);

            tmp = std::format("{},{} Byte;", fileName, fileSize);

            data += tmp;
        }
        if (!data.empty())
        {
            data = data.substr(0, data.size() - 1);
        }
    }
    catch (const fs::filesystem_error &e)
    {
        std::cerr << "Error while listing directory: " << e.what() << std::endl;
        return "";
    }
    catch (...)
    {
        std::cerr << "Unknown error occurred." << std::endl;
        return "";
    }
    return data;
}
