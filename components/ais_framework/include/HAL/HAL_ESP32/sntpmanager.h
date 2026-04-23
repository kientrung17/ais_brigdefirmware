#pragma once
#include <ctime>
#include <string>

class SntpManager
{
public:
    // ctor: truyền server (vd: "pool.ntp.org") và chuỗi timezone (vd: "ICT-7")
    SntpManager();

    // B2. Bắt đầu SNTP và chờ đồng bộ lần đầu
    void beginUpdateSync(const std::string &server = "pool.ntp.org",
                         const std::string &timezone = "ICT-7");
    // B1. kiểm tra đã đồng bộ chưa
    bool checkSntpSynced();
    // Lấy giờ hiện tại (giờ địa phương)
    bool getLocalTime(struct tm &out_tm) const;

    // Trả về epoch time (s)
    time_t now() const { return time(nullptr); }
};
