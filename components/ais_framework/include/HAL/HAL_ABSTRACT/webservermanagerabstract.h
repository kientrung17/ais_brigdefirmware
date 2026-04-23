#pragma once

#include <string>

class WebServerAbstract {
public:
    virtual ~WebServerAbstract() = default;
    virtual bool start() = 0;
    virtual void stop() = 0;
};
