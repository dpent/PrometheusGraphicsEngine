#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


namespace Prometheus{
    enum operationId : uint16_t{
        CREATE_OBJECT = 0,
        DELETE_OBJECT = 1
    };

    struct Job{
    public:
        operationId opId;
        uint64_t threadId;

        Job(operationId opId);
    };
}