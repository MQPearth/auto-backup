#include "iostream"
#include <chrono>
#include <ctime>
#include <iomanip>
#include <cstring>
#include "cos/cos_api.h"
#include "cos/cos_defines.h"
#include "cos/cos_config.h"
#include "cos/request/object_req.h"
#include "cos/response/object_resp.h"


#define HELP_INFO "参数错误, 程序调用示例: \nauto-backup -p /usr/local/xxx/data/";

/**
 * 校验参数
 *
 * @param argc
 * @param argv
 * @throw 错误信息
 */
void verify_param(int argc, char **argv) noexcept(false);

/**
 * 将文件上传到COS
 *
 * @param filename
 */
void upload(char *cmd_path, std::string filename) noexcept(false);

/**
 * 压缩文件
 *
 * @param path
 * @return
 */
std::string zip(char *path);

/**
 * 删除压缩文件
 *
 * @param file
 */
void deleteReduceFile(std::string file);

/**
 * main
 *
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char **argv) {
    try {
        verify_param(argc, argv);

        std::ifstream ifs(argv[2], std::ios::in);
        if (!ifs || !ifs.is_open()) {
            std::cout << "[" << argv[2] << "]路径不存在或权限不足" << std::endl;
            return 0;
        }
        std::string filename = zip(argv[2]);

        upload(argv[0], filename);

        deleteReduceFile(filename);

        std::cout << "备份成功" << std::endl;

        return 0;

    } catch (char const *&msg) {
        std::cout << msg << std::endl;
        return -1;
    }
}


void verify_param(int argc, char **argv) {

    if (argc <= 2) {
        throw HELP_INFO;
    }

    if (argc == 3) {
        if (strcmp(argv[1], "-p") != 0) {
            throw HELP_INFO;
        }
    }

    if (argc > 3) {
        throw "参数数量错误";
    }
}


std::string zip(char *path) {
    auto now = std::chrono::system_clock::now();
    auto ticks = std::chrono::system_clock::to_time_t(now);
    auto local_time = localtime(&ticks);

    std::ostringstream filename_oss;

    filename_oss << std::put_time(local_time, "%F");
    filename_oss << std::put_time(local_time, "T%T");
    filename_oss << "-backup.zip";
    std::string filename = filename_oss.str();

    std::ostringstream cmd_oss;

    cmd_oss << "zip -q -r ";
    cmd_oss << filename;
    cmd_oss << " ";
    cmd_oss << path;

    system(cmd_oss.str().c_str());

    std::ifstream ifs(filename.c_str(), std::ios::in);
    if (!ifs || !ifs.is_open()) {
        throw "文件压缩失败, 请检查权限是否正常以及zip是否正确安装";
    }

    std::cout << "文件压缩成功, 等待上传" << std::endl;

    return filename;
}


void upload(char *_cmd_path, std::string filename) {

    std::string cmd_path(_cmd_path);

    std::stringstream config_path_ss;
    config_path_ss << cmd_path.substr(0, cmd_path.find_last_of("/") + 1);
    config_path_ss << "cos_config.json";

    std::string config_path = config_path_ss.str();

    std::cout << config_path << std::endl;

    qcloud_cos::CosConfig config(config_path);
    qcloud_cos::CosAPI cos(config);

    std::ifstream ifs(config_path.c_str(), std::ios::in);
    Poco::JSON::Parser parser;
    Poco::JSON::Object::Ptr object = parser.parse(ifs).extract<Poco::JSON::Object::Ptr>();

    std::string bucket_name;
    std::string object_name;
    qcloud_cos::CosConfig::JsonObjectGetStringValue(object, "BucketName", &bucket_name);
    qcloud_cos::CosConfig::JsonObjectGetStringValue(object, "ObjectName", &object_name);

    if (bucket_name.empty() || object_name.empty()) {
        throw "配置文件错误";
    }

    // 替换为用户指定的文件路径
    qcloud_cos::PutObjectByFileReq req(bucket_name, object_name.append(filename), filename);

    qcloud_cos::PutObjectByFileResp resp;

    qcloud_cos::CosResult result = cos.PutObject(req, &resp);

    if (!result.IsSucc()) {
        std::stringstream ss;
        // 上传文件失败，可以调用 CosResult 的成员函数输出错误信息，比如 requestID 等
        ss << "HttpStatus=" << result.GetHttpStatus() << std::endl;
        ss << "ErrorCode=" << result.GetErrorCode() << std::endl;
        ss << "ErrorMsg=" << result.GetErrorMsg() << std::endl;
        ss << "ResourceAddr=" << result.GetResourceAddr() << std::endl;
        ss << "XCosRequestId=" << result.GetXCosRequestId() << std::endl;
        ss << "XCosTraceId=" << result.GetXCosTraceId() << std::endl;
        throw ss.str();
    }
    return;
}


void deleteReduceFile(std::string file) {
    std::ostringstream cmd_oss;

    cmd_oss << "rm ";
    cmd_oss << file;

    system(cmd_oss.str().c_str());
}

