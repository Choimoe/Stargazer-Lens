#pragma once
#include <string>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace JsonToGbString {
    std::string convertIntToTileStr(int t);
    std::string formatHandStr(std::vector<std::string>& tiles);
    std::string translate(const json& jsonData);
}

std::string get_log_filename();