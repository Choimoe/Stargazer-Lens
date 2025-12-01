#include "server/server.h"
#include <iostream>
#include <stdexcept>
#include <console/console.h>
#include <console/print.h>
#include <mahjong/fan.h>
#include <mahjong/handtiles.h>
#include "utils/log.h"
#include "utils/format.h"

using json = nlohmann::json;

void start_server(int port) {
    static Logger logger(get_log_filename());
    httplib::Server svr;

    svr.Options("/calculate", [](const httplib::Request&, httplib::Response &res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");
        res.set_header("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
        res.status = 204;
    });

    svr.Post("/calculate", [&](const httplib::Request &req, httplib::Response &res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        json response_json;
        std::string gb_string;
        std::string client_info;
        std::string remote_ip = req.remote_addr;
        try {
            json received_json = json::parse(req.body);
            if (received_json.contains("type") && received_json["type"] == "connect_test") {
                client_info = "[CONNECT_TEST] IP: " + remote_ip + " " + received_json.dump();
                logger.info(client_info);
                response_json["status"] = "success";
                response_json["message"] = "连接测试成功";
                res.set_content(response_json.dump(4), "application/json; charset=utf-8");
                return;
            }
            logger.info(std::string("[REQUEST] 收到请求: IP: ") + remote_ip + " " + req.body);
            logger.info(std::string("[CLIENT] IP: ") + remote_ip + " UA: " + (received_json.contains("userAgent") ? received_json["userAgent"].get<std::string>() : "未知UA"));
            logger.debug(std::string("解析后的JSON: ") + received_json.dump(4));
            gb_string = JsonToGbString::translate(received_json);
            logger.debug(std::string("牌型字符串: ") + gb_string);
            mahjong::Handtiles ht;
            ht.StringToHandtiles(gb_string);
            logger.debug(std::string("Handtiles对象: ") + ht.HandtilesToString());
            mahjong::Fan fan;
            std::vector<mahjong::Tile> ting_tiles = fan.CalcTing(ht);
            json ting_result;
            for (auto t : ting_tiles) {
                ting_result.push_back(mahjong::TileToEmojiString(t));
            }
            logger.debug(std::string("听牌结果: ") + ting_result.dump());
            fan.CountFan(ht);
            json fan_details = json::array();
            for (int i = 1; i < mahjong::FAN_SIZE; i++) {
                for (size_t j = 0; j < fan.fan_table_res[i].size(); j++) {
                    std::string pack_string;
                    for (auto pid : fan.fan_table_res[i][j]) {
                        pack_string += " " + mahjong::PackToEmojiString(fan.fan_packs_res[pid]);
                    }
                    fan_details.push_back({
                        {"name", mahjong::FAN_NAME[i]},
                        {"score", mahjong::FAN_SCORE[i]},
                        {"packs", pack_string}
                    });
                }
            }
            logger.debug(std::string("算番详情: ") + fan_details.dump());
            response_json["status"] = "success";
            response_json["total_fan"] = fan.tot_fan_res;
            response_json["fan_details"] = fan_details;
            response_json["ting"] = ting_result;
            response_json["parsed_hand"] = ht.HandtilesToString();
            logger.info(std::string("[RESPONSE] 返回内容: ") + response_json.dump(4));
        } catch (const std::exception& e) {
            logger.error(std::string("处理时发生错误: ") + e.what());
            response_json["status"] = "error";
            response_json["message"] = e.what();
            response_json["failed_string"] = gb_string;
        }
        res.set_content(response_json.dump(4), "application/json; charset=utf-8");
    });

    std::string local_ip = "0.0.0.0";
    std::string intranet_ip;
#if defined(_WIN32)
    char buf[256] = {0};
    FILE* fp = popen("powershell -Command \"(Get-NetIPAddress -AddressFamily IPv4 | Where-Object { $_.InterfaceAlias -ne 'Loopback Pseudo-Interface 1' -and $_.IPAddress -notlike '169.*' }).IPAddress\"", "r");
    if (fp && fgets(buf, sizeof(buf), fp)) {
        intranet_ip = std::string(buf);
        if (!intranet_ip.empty() && intranet_ip.back() == '\n') intranet_ip.pop_back();
    } else {
        intranet_ip = "无法获取内网IP";
    }
    if (fp) pclose(fp);
#elif defined(__APPLE__)
    char buf[256] = {0};
    FILE* fp = popen("ipconfig getifaddr en0", "r");
    if (fp && fgets(buf, sizeof(buf), fp)) {
        intranet_ip = std::string(buf);
        if (!intranet_ip.empty() && intranet_ip.back() == '\n') intranet_ip.pop_back();
    } else {
        intranet_ip = "无法获取内网IP";
    }
    if (fp) pclose(fp);
#elif defined(__linux__)
    char buf[256] = {0};
    FILE* fp = popen("hostname -I | awk '{print $1}'", "r");
    if (fp && fgets(buf, sizeof(buf), fp)) {
        intranet_ip = std::string(buf);
        if (!intranet_ip.empty() && intranet_ip.back() == '\n') intranet_ip.pop_back();
    } else {
        intranet_ip = "无法获取内网IP";
    }
    if (fp) pclose(fp);
#else
    intranet_ip = "请手动查询内网IP";
#endif
    std::cout << "C++ 算番服务器已启动，正在监听端口: " << port << std::endl;
    std::cout << "本机访问地址: http://" << local_ip << ":" << port << std::endl;
    std::cout << "内网访问地址: http://" << intranet_ip << ":" << port << std::endl;
    std::cout << "请保持此窗口开启..." << std::endl;
    svr.listen("0.0.0.0", port);
}
