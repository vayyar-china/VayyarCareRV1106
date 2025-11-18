#include <json.h>
#include <fstream>
using namespace std;

int main() {
    Json::Value root;
    
    // 组装JSON内容
    root["FIFO_NAME"] = "/root/autodl-tmp/myfifo";  //log管道文件
    root["LOG_PATH"] = "/root/autodl-tmp/RV_log/Log_server/logfiles/";  //Log保存目录    
    root["TIME_RV_LOG_LOOP"] = 6;    //600s间隔    
    root["MAX_FILES"] = 144;  //log文件保存的数量    
    root["IS_UPLOAD_TOCLOUD"] = 1;  //是否上传至阿里云 
    
    // 使用StyledWriter格式化输出
    Json::StyledWriter writer;
    ofstream os;
    os.open("/root/autodl-tmp/RV_log/Log_server/Log.json");
    os << writer.write(root);
    os.close();
    
    return 0;
}
