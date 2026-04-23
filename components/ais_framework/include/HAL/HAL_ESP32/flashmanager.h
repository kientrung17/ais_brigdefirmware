#pragma once

#include <string>
#include <vector>
#include "nvs_flash.h"
#include "nvs.h"
#include "flashmanagerabstract.h"

class FlashManager : public FlashManagerAbstract
{
public:
    FlashManager();
    ~FlashManager();

    /**
     * @brief  Khởi tạo NVS và mở namespace.
     * @param  ns: tên namespace (mặc định "storage").
     * @return true nếu init/open thành công.
     */
    virtual bool init(const char *ns = "storage") override;

    /** Đóng handle NVS */
    virtual void deinit() override;

    /** Ghi 1 chuỗi vào flash */
    virtual bool writeString(const char *key, const std::string &value) override;

    /** Đọc 1 chuỗi từ flash.
     *  @param default_val: trả về nếu flash chưa có giá trị key này.
     */
    virtual bool readString(const char *key, std::string &out, const std::string &default_val = "") override;

    /** Ghi 1 số int32_t vào flash */
    virtual bool writeInt(const char *key, int32_t value) override;

    /** Đọc 1 số int32_t từ flash với default nếu không tồn tại */
    virtual bool readInt(const char *key, int32_t &out, int32_t default_val = 0) override;

    /** Ghi blob (mảng byte) vào flash */
    virtual bool writeBlob(const char *key, const void *data, size_t size) override;

    /**
     * Đọc blob từ flash
     * @param size: kích thước buffer truyền vào
     * @param out_size: kích thước thực đọc được
     */
    virtual bool readBlob(const char *key, void *data, size_t size, size_t &out_size) override;

private:
    nvs_handle_t mHandle{0};
};
