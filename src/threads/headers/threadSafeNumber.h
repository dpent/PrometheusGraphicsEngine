#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <mutex>

namespace Prometheus
{
    class SafeUint16_t{
    protected:
        std::mutex mutex;
    public:
        uint16_t number;

        SafeUint16_t(uint16_t startValue);
        SafeUint16_t();

        void add(uint16_t value);

        uint16_t getValue();
    };
}
