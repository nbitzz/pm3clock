#include "curl/curl.h"
#include "string"
#include "vector"
inline size_t curl_string_wf(char* data, size_t size, size_t nmemb, void* userdata) {
    static_cast<std::string*>(userdata)->append(data, nmemb);
    return nmemb;
}
inline size_t curl_vec_wf(unsigned char* data, size_t size, size_t nmemb, void* userdata) {
    auto vec = static_cast<std::vector<unsigned char>*>(userdata);
    vec->insert(vec->end(), &data[0], &data[nmemb]);
    return nmemb;
}
/*
struct bufc {
    unsigned int size;
    int p; // i know i could just store the pointer
           // storing p would let me check p+nmemb >= size though
           // so sorry but we're doing this
    char* buf;
}
inline size_t curl_buf_wf(char* data, size_t size, size_t nmemb, void* userdata) {
    auto bc = static_cast<bufc*>(userdata);
    memcpy(bc, data, nmemb);
    if (p+nmemb >= size) {
        throw "bufc is too small";
    }
    return nmemb;
}*/
