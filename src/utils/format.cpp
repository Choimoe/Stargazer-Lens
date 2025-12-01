#include <chrono>

#include <vector>
#include <map>
#include <algorithm>
#include <stdexcept>
#include "third_party/json/include/nlohmann/json.hpp"
using json = nlohmann::json;

namespace JsonToGbString {
    std::string convertIntToTileStr(int t) {
        auto cvt = [](int val) -> int {
            if (val == 0) return -1;
            int suit = ((val >> 4) & 0xF);
            int rank = (val & 0xF);
            return ((suit - 1) * 9) + (rank - 1);
        };

        static const std::vector<std::string> tlcls = {
            "1m", "2m", "3m", "4m", "5m", "6m", "7m", "8m", "9m",
            "1s", "2s", "3s", "4s", "5s", "6s", "7s", "8s", "9s",
            "1p", "2p", "3p", "4p", "5p", "6p", "7p", "8p", "9p",
            "1z", "2z", "3z", "4z", "5z", "6z", "7z"
        };
        
        static const std::map<std::string, std::string> gb_map = {
            {"1z", "E"}, {"2z", "S"}, {"3z", "W"}, {"4z", "N"},
            {"5z", "C"}, {"6z", "F"}, {"7z", "P"}
        };

        int index = cvt(t);

        if (index < 0 || index >= tlcls.size()) {
            throw std::runtime_error("Unknown or invalid tile integer value: " + std::to_string(t));
        }

        const std::string& tile_str = tlcls[index];

        if (gb_map.count(tile_str)) {
            return gb_map.at(tile_str);
        }

        return tile_str;
    }

    std::string formatHandStr(std::vector<std::string>& tiles) {
        static const std::map<char, int> suit_order = {{'m', 1}, {'s', 2}, {'p', 3}};
        static const std::map<char, int> honor_order = {{'E', 4}, {'S', 5}, {'W', 6}, {'N', 7}, {'C', 8}, {'F', 9}, {'P', 10}};

        std::sort(tiles.begin(), tiles.end(), [&](const std::string& a, const std::string& b) {
            bool a_is_honor = honor_order.count(a[0]);
            bool b_is_honor = honor_order.count(b[0]);

            if (a_is_honor != b_is_honor) return a_is_honor < b_is_honor;

            if (a_is_honor) {
                return honor_order.at(a[0]) < honor_order.at(b[0]);
            } else {
                if (a.back() != b.back()) return suit_order.at(a.back()) < suit_order.at(b.back());
                return a[0] < b[0];
            }
        });

        std::string result = "";
        if (tiles.empty()) return result;

        for (size_t i = 0; i < tiles.size(); ) {
            char current_suit = tiles[i].back();
            if (honor_order.count(tiles[i][0])) {
                result += tiles[i];
                i++;
                continue;
            }
            
            std::string rank_part = "";
            size_t j = i;
            while(j < tiles.size() && tiles[j].back() == current_suit) {
                rank_part += tiles[j][0];
                j++;
            }
            result += rank_part + current_suit;
            i = j;
        }
        return result;
    }

    std::string translate(const json& jsonData) {
        if (!jsonData.contains("q") || !jsonData["q"].contains("h")) {
            throw std::runtime_error("JSON格式无效，缺少牌数据 'q.h'");
        }
        const auto& q_data = jsonData["q"];
        const auto& hand_data = q_data["h"];
        std::string result_str = "";

        if (hand_data.contains("p")) {
            for (int p_val : hand_data["p"]) {
                int o = (p_val >> 12) & 0xF;
                int c = (p_val >> 8) & 0xF;
                int t = p_val & 0xFF;
                std::string base_tile_str = convertIntToTileStr(t);
                
                std::string rank_str = base_tile_str.substr(0, 1);
                std::string suit_str = base_tile_str.length() > 1 ? base_tile_str.substr(1) : "";
                
                result_str += "[";
                switch (c) {
                    case 1: {
                        int rank = base_tile_str[0] - '0';
                        result_str += std::to_string(rank - 1) + std::to_string(rank) + std::to_string(rank + 1) + suit_str;
                        result_str += ",1";
                        break;
                    }
                    case 2:
                        result_str += rank_str + rank_str + rank_str + suit_str;
                        result_str += "," + std::to_string(o);
                        break;
                    case 3:
                        result_str += rank_str + rank_str + rank_str + rank_str + suit_str;
                        if (o != 0) {
                             result_str += "," + std::to_string(o);
                        }
                        break;
                }
                result_str += "]";
            }
        }
        
        std::vector<std::string> standing_tiles;
        if (hand_data.contains("s")) {
            for (int t_val : hand_data["s"]) {
                standing_tiles.push_back(convertIntToTileStr(t_val));
            }
        }
        result_str += formatHandStr(standing_tiles);

        if (hand_data.contains("a") && hand_data["a"] != 0) {
            result_str += convertIntToTileStr(hand_data["a"]);
        }
        
        std::string situation_str = "|";
        if (q_data.contains("r") && q_data.contains("s")) {
            int round_val = q_data["r"];
            int seat_val = q_data["s"];
            int choice_val = q_data.value("c", 0);

            auto map_wind = [](int val) -> char {
                switch(val) {
                    case 0: return 'E';
                    case 1: return 'S';
                    case 2: return 'W';
                    case 3: return 'N';
                    default: return 'E';
                }
            };
            
            situation_str += map_wind( round_val / 4 );
            situation_str += map_wind( seat_val );
            
            situation_str += (choice_val & 1) ? '1' : '0';
            situation_str += (choice_val & 2) ? '1' : '0';
            situation_str += '0';
            situation_str += '0';
        } else {
            situation_str += "EE0000";
        }
        result_str += situation_str;
        
        return result_str;
    }
}

std::string get_log_filename() {
    auto now = std::chrono::system_clock::now();
    auto tt = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
#if defined(_WIN32) || defined(_WIN64)
    localtime_s(&tm, &tt);
#else
    localtime_r(&tt, &tm);
#endif
    char log_filename[64];
    std::strftime(log_filename, sizeof(log_filename), "logs/mahjong_server_%Y%m%d_%H%M%S.log", &tm);
    return std::string(log_filename);
}
