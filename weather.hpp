#include "string"
#include "vector"

namespace Weather {
    struct GridPosition {
        std::string office_id;
        int x;
        int y;
    };
    struct Forecast {
        std::string image_url;
        std::string forecast_long;
    };
    class Gridpoint {
        GridPosition position;
        public:
            Gridpoint() = delete;
            Gridpoint(GridPosition position);
            Gridpoint(double latitude, double longitude);
            Forecast forecast();
    };
}