#include "util.hpp"
#include "curl/curl.h"
#include "string"
#include "regex"
#include "vector"
#include "alertca.hpp"
namespace AlertCA {
    namespace regex {
        using std::regex;
        // i should just get a json parser module LOL
        regex camera("\\{\\s*\"type\":\\s*\"Feature\",\\s*\"geometry\":\\s*\\{\\s*\"type\":\\s*\"Point\",\\s*\"coordinates\":\\s*\\[\\s*([\\-\\.\\d]+),\\s*([\\-\\.\\d]+)[\\S\\s]*?\"properties\":\\s*\\{\\s*\"id\":\\s*\"(.*?)\",\\s*\"name\":\\s*\"(.*?)\",");
    }
    std::vector<Camera> getCameras() {
        std::string data;
        CURL* hnd = curl_easy_init();
        curl_easy_setopt(hnd, CURLOPT_BUFFERSIZE, 102400L);
        curl_easy_setopt(hnd, CURLOPT_URL, "https://cameras.alertcalifornia.org/public-camera-data/all_cameras-v3.json");
        curl_easy_setopt(hnd, CURLOPT_NOPROGRESS, 1L);
        curl_easy_setopt(hnd, CURLOPT_USERAGENT, "curl/8.15.0");
        curl_easy_setopt(hnd, CURLOPT_MAXREDIRS, 50L);
        curl_easy_setopt(hnd, CURLOPT_CUSTOMREQUEST, "GET");
        curl_easy_setopt(hnd, CURLOPT_TCP_KEEPALIVE, 1L);
        curl_easy_setopt(hnd, CURLOPT_WRITEDATA, &data);
        curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, curl_string_wf);
        curl_easy_perform(hnd);

        std::smatch m;
        std::vector<Camera> cams;
        auto i = data.cbegin();
        while (i < data.cend()) {
            auto matched = std::regex_search(i, data.cend(), m, regex::camera);
            if (matched) {
                i = m[0].second;
                cams.insert(cams.end(), Camera {m[3].str(), m[4].str(), std::stof(m[2].str()), std::stof(m[1].str())});
            } else break;
        }

        return cams;
    }
}