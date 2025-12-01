#include "server/server.h"
#include <cxxopts.hpp>

void UnitTest();

int main(int argc, char* argv[]) {
    cxxopts::Options options("calc_server", "国标麻将 C++ 算番服务器");
    options.add_options()
        ("t,unit-test", "执行单元测试", cxxopts::value<bool>()->default_value("false"))
        ("p,port", "指定服务器端口", cxxopts::value<int>()->default_value("17711"))
        ("h,help", "显示帮助信息");

    auto result = options.parse(argc, argv);

    if (result.count("help")) {
        std::cout << options.help() << std::endl;
        return 0;
    }

    if (result["unit-test"].as<bool>()) {
        UnitTest();
        return 0;
    }

    int port = result["port"].as<int>();
    start_server(port);
    return 0;
}
