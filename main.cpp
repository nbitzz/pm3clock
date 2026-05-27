#include "raylib.h"
#include "ctime"
#include "weather.hpp"
#include "config.h"
#include "cmath"
#include "curl/curl.h"
#include "util.hpp"
#include "regex"
#include "thread"
#include "chrono"
#include "mutex"

const unsigned char dm_mono_buf[] = {
    #embed "fonts/DMMono-Regular.ttf"
};

const unsigned char dm_sans_semibold_buf[] = {
    #embed "fonts/DMSans-SemiBold.ttf"
};

// use with Forecast
Image load_image_from_url(std::string &url) {
    std::vector<unsigned char> data;
    CURL* hnd = curl_easy_init();
    curl_easy_setopt(hnd, CURLOPT_BUFFERSIZE, 102400L);
    curl_easy_setopt(hnd, CURLOPT_URL, url.c_str());
    curl_easy_setopt(hnd, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(hnd, CURLOPT_USERAGENT, "curl/8.15.0");
    curl_easy_setopt(hnd, CURLOPT_MAXREDIRS, 50L);
    curl_easy_setopt(hnd, CURLOPT_CUSTOMREQUEST, "GET");
    curl_easy_setopt(hnd, CURLOPT_TCP_KEEPALIVE, 1L);
    curl_easy_setopt(hnd, CURLOPT_WRITEDATA, &data);
    curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, curl_vec_wf);
    
    curl_easy_perform(hnd);
    return LoadImageFromMemory(".png", data.data(), data.size()); 
}

Image make_background_image(Image &img) {
    ImageResize(&img, 800, 800);
    ImageCrop(&img, Rectangle{0, 160, 800, 480});
    ImageColorBrightness(&img, -127);
    ImageBlurGaussian(&img, 10);
    return img;
}

// convenience function
Texture2D image_to_texture(Image &img) {
    auto t = LoadTextureFromImage(img);
    UnloadImage(img);
    return t;
}
Texture2D image_to_texture(Image &&img) {
    return image_to_texture(img);
}

int main() {
    Weather::Gridpoint gp(latlong);
    
    InitWindow(800, 480, "pm3clock");
    SetTargetFPS(fps);
    auto dm_mono = LoadFontFromMemory(".ttf", dm_mono_buf, sizeof(dm_mono_buf), clock_sz, NULL, 0);
    auto dm_sans = LoadFontFromMemory(".ttf", dm_sans_semibold_buf, sizeof(dm_sans_semibold_buf), meta_sz, NULL, 0);

    #if fullscreen
    if (!IsWindowFullscreen())
        ToggleFullscreen();
    #endif

    auto bottom_gradient = image_to_texture(GenImageGradientLinear(800, meta_sz+10, 0, Color{0,0,0,0}, BLACK));

    auto tx = image_to_texture(GenImageColor(800, 480, BLACK));
    auto fc = Weather::Forecast { .image_url = "", .forecast_long = "Trying to get forecast..." };
    Image* new_img = nullptr;
    std::mutex new_img_mtx;

    auto t = std::thread([&fc, &new_img, &gp, &new_img_mtx]() {
        while (!WindowShouldClose()) {
            auto new_fc = gp.forecast();
            // update background when needed
            if (fc.image_url != new_fc.image_url) {
                auto i = load_image_from_url(new_fc.image_url);
                make_background_image(i);

                // this feels dumb LOL
                // but i think this is the best way to copy it out
                // maybe i could see if i could move?
                const std::lock_guard guard(new_img_mtx);
                new_img = new Image(i);
            }
            fc = new_fc; // maybe i should use a mutex for this too but i'm lazy. i think it's less of an issue
            using namespace std::chrono;
            std::this_thread::sleep_for(weather_refresh_time); // lol ts is not closing
        }
    });

    while (!WindowShouldClose()) {
        BeginDrawing();
            ClearBackground(BLACK);
            if (new_img_mtx.try_lock()) {
                // need to load textures in main thread
                if (new_img != nullptr) {
                    UnloadTexture(tx);
                    tx = image_to_texture(*new_img);
                    delete new_img;
                    new_img = nullptr;
                }
                new_img_mtx.unlock();
            }

            DrawTexture(tx, 0, 0, WHITE);

            // drawing clock
            auto t = std::time(NULL);
            char time[10];
            std::strftime(time, 10, "%H %M %S", std::localtime(&t));
            char date[20];
            std::strftime(date, 20, "%A, %B %d", std::localtime(&t));
            char week[10];
            std::strftime(week, 10, "Week %V", std::localtime(&t));

            const auto ctr_top = (480-(clock_sz+(meta_sz*2)))/2;

            // yuck
            DrawTextEx(dm_mono, time, Vector2 { .x = (800 - MeasureTextEx(dm_mono, time, clock_sz, 0).x) / 2, .y = ctr_top }, clock_sz, 0, WHITE);
            DrawTextEx(dm_mono, "  :  :  ", Vector2 { .x = (800 - MeasureTextEx(dm_mono, time, clock_sz, 0).x) / 2, .y = ctr_top }, clock_sz, 0, t % 2 == 0 ? LIGHTGRAY : GRAY);
            DrawTextEx(dm_sans, date, Vector2 { .x = (800 - MeasureTextEx(dm_sans, date, meta_sz, 0).x) / 2, .y = ctr_top+clock_sz }, meta_sz, 0, LIGHTGRAY);
            DrawTextEx(dm_sans, week, Vector2 { .x = (800 - MeasureTextEx(dm_sans, week, meta_sz, 0).x) / 2, .y = ctr_top+clock_sz+meta_sz }, meta_sz, 0, LIGHTGRAY);
            // draw scrolling forecast text
            DrawTexture(bottom_gradient,0,480-meta_sz-10,WHITE);
            auto fct = fc.forecast_long.c_str();
            auto fctx = MeasureTextEx(dm_sans, fc.forecast_long.c_str(), meta_sz, 0).x;

            // we only need to make it scroll if it's too wide
            DrawTextEx(dm_sans, fct, Vector2 { 
                .x = fctx > 800 
                    ? static_cast<float>(800 - ((800 + fctx)*(std::fmod(GetTime() / (((fctx+1600)/800)*10), 1))))
                    : (800-fctx)/2, 
                .y = 480 - meta_sz - 10
            }, meta_sz, 0, LIGHTGRAY);
            
        EndDrawing();
    }

    t.join();
    CloseWindow();

    return 0;
}