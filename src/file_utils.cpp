#include "file_utils.h"

#include <fstream>
#include <sstream>

bool read_text_file(const std::string& path, std::string& out_text)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    out_text = buffer.str();
    return true;
}
