#pragma once
#include <string>
#include <httplib.h>
#include <nlohmann/json.hpp>

void start_server(int port = 17711, const std::string& log_path = "");