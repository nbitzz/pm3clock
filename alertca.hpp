#include "string"
#include "vector"
namespace AlertCA {
    struct Camera {
        std::string id;
        std::string name;
        double latitude;
        double longitude;
    };
    std::vector<Camera> getCameras();
}