# GB-Mahjong-Client

国标麻将算番 HTTP 服务器，基于 [GB-Mahjong](https://github.com/Choimoe/GB-Mahjong) C++ 库实现。

## 简介

GB-Mahjong-Client 是一个轻量级的 HTTP 服务器，提供国标麻将的算番、听牌计算等功能。配合浏览器用户脚本，可以在网页麻将游戏中实时展示番数计算结果。

## 依赖

### 第三方库

- [cpp-httplib](https://github.com/yhirose/cpp-httplib) - HTTP 服务器库
- [nlohmann/json](https://github.com/nlohmann/json) - JSON 解析库
- [cxxopts](https://github.com/jarro2783/cxxopts) - 命令行参数解析
- [GB-Mahjong](https://github.com/Choimoe/GB-Mahjong) - 国标麻将核心库

## 快速开始

使用 CMake 构建：

```bash
git clone --recursive https://github.com/Choimoe/GB-Mahjong-Client.git
cd GB-Mahjong-Client

mkdir build && cd build

cmake ..
make -j
```

运行：

```bash
# 使用默认端口 17711 启动
./calc_server

# 指定端口
./calc_server -p 8080

# 指定日志文件路径
./calc_server -l /path/to/logfile.log

# 查看帮助信息
./calc_server --help
```

### 命令行选项

```
选项:
  -t, --unit-test      执行单元测试
  -p, --port PORT      指定服务器端口 (默认: 17711)
  -l, --log PATH       指定日志文件路径 (默认: logs/目录)
  -v, --version        显示版本信息
  -h, --help           显示帮助信息
```

## API 文档

### 算番接口

**请求**

```http
POST /calculate
Content-Type: application/json

{
  "q": "手牌字符串",
  "userAgent": "客户端信息（可选）"
}
```

手牌字符串格式参见 [GB-Mahjong 文档](https://github.com/Choimoe/GB-Mahjong#README)。

示例：
```json
{
  "q": "123789s123789p33m"
}
```

**响应**

```json
{
  "status": "success",
  "total_fan": 18,
  "fan_details": [
    {
      "name": "小于五",
      "score": 12,
      "packs": ""
    },
    {
      "name": "平和",
      "score": 2,
      "packs": ""
    }
  ],
  "ting": ["🀇", "🀐", "🀙"],
  "parsed_hand": "123789s123789p33m|EE0000|"
}
```