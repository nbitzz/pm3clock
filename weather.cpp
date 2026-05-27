#include "curl/curl.h"
#include "string"
#include "regex"
#include "string.h"
#include "weather.hpp"
#include "util.hpp"
namespace Weather {
    namespace regex {
        using std::regex;
        regex grid_posn("\"gridId\":\\s*\"([A-Z]{3})\",\\s*\"gridX\":\\s*(\\d+),\\s*\"gridY\":\\s*(\\d+)");
        regex forecast_and_url("\"icon\":\\s*\"(.*)\",[\\s\\S]*?\"detailedForecast\":\\s*\"(.*)\"");
    }

    Gridpoint::Gridpoint(GridPosition position) : position(position) {}
    Gridpoint::Gridpoint(double latitude, double longitude) {
        char u[40];
        std::string data;
        // since curl only takes c style strings we might as well use them
        sprintf(u, "https://api.weather.gov/points/%.4f,%.4f", latitude, longitude);
        CURL* hnd = curl_easy_init();
        curl_easy_setopt(hnd, CURLOPT_BUFFERSIZE, 102400L);
        curl_easy_setopt(hnd, CURLOPT_URL, u);
        curl_easy_setopt(hnd, CURLOPT_NOPROGRESS, 1L);
        curl_easy_setopt(hnd, CURLOPT_USERAGENT, "curl/8.15.0");
        curl_easy_setopt(hnd, CURLOPT_MAXREDIRS, 50L);
        curl_easy_setopt(hnd, CURLOPT_CUSTOMREQUEST, "GET");
        curl_easy_setopt(hnd, CURLOPT_TCP_KEEPALIVE, 1L);
        curl_easy_setopt(hnd, CURLOPT_WRITEDATA, &data);
        curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, curl_string_wf);
        
        curl_easy_perform(hnd);
        auto buf = data.c_str();
        std::cmatch m;
        std::regex_search(buf, m, regex::grid_posn);

        this->position = GridPosition {
            .office_id = m[1].str(),
            .x = std::stoi(m[2].str()),
            .y = std::stoi(m[3].str())
        };
    }

    Forecast Gridpoint::forecast() {
        char u[50];
        std::string data;
        // since curl only takes c style strings we might as well use them
        sprintf(u, "https://api.weather.gov/gridpoints/%s/%d,%d/forecast", this->position.office_id.c_str(), this->position.x, this->position.y);
        CURL* hnd = curl_easy_init();
        curl_easy_setopt(hnd, CURLOPT_BUFFERSIZE, 102400L);
        curl_easy_setopt(hnd, CURLOPT_URL, u);
        curl_easy_setopt(hnd, CURLOPT_NOPROGRESS, 1L);
        curl_easy_setopt(hnd, CURLOPT_USERAGENT, "curl/8.15.0");
        curl_easy_setopt(hnd, CURLOPT_MAXREDIRS, 50L);
        curl_easy_setopt(hnd, CURLOPT_CUSTOMREQUEST, "GET");
        curl_easy_setopt(hnd, CURLOPT_TCP_KEEPALIVE, 1L);
        curl_easy_setopt(hnd, CURLOPT_WRITEDATA, &data);
        curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, curl_string_wf);
        
        curl_easy_perform(hnd);
        auto buf = data.c_str();
        std::cmatch m;
        std::regex_search(buf, m, regex::forecast_and_url);
        return Forecast {
            // replace medium with large
            .image_url = m[1].str().replace(m[1].str().size()-6, 6, "large"),
            .forecast_long = m[2].str()
        };
    }
}