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
#include "alertca.hpp"
#include "algorithm"

const unsigned char dm_sans_extrabold_buf[] = {
    #embed "fonts/DMSans-ExtraBold.ttf"
};

const unsigned char dm_sans_semibold_buf[] = {
    #embed "fonts/DMSans-SemiBold.ttf"
};

// use with Forecast
Image load_image_from_url(std::string url) {
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
    InitWindow(800, 480, "pm3clock");
    SetTargetFPS(fps);
    auto clock_font = LoadFontFromMemory(".ttf", dm_sans_extrabold_buf, sizeof(dm_sans_semibold_buf), clock_sz, NULL, 0);
    auto meta_font = LoadFontFromMemory(".ttf", dm_sans_semibold_buf, sizeof(dm_sans_semibold_buf), meta_sz, NULL, 0);

    #if fullscreen
    if (!IsWindowFullscreen())
        ToggleFullscreen();
    #endif

    auto tx = image_to_texture(GenImageColor(800, 480, BLACK));
    auto fc = Weather::Forecast { .image_url = "", .forecast_long = "Waiting for weather service..." };

    // TODO: move these out into maybe another function at least or something
    // this is a little too much for main()

    auto weather_service = std::thread([&fc]() {
        Weather::Gridpoint gp(current_latitude, current_longitude);
        while (!WindowShouldClose()) {
            fc = gp.forecast(); // maybe i should use a mutex for this too but i'm lazy. i think it's less of an issue
            using namespace std::chrono;
            std::this_thread::sleep_for(weather_refresh_time); // lol ts is not closing
        }
    });

    Image* new_img = nullptr;
    std::mutex new_img_mtx;
    auto wildlife_photo_service = std::thread([&new_img, &new_img_mtx]() {
        // first get cams and sort by nearest
        auto cams = AlertCA::getCameras();
        std::sort(cams.begin(), cams.end(), [](AlertCA::Camera &a, AlertCA::Camera &b) {
            const auto distance_a = std::sqrtf(std::powf(a.latitude - current_latitude, 2) + std::powf(a.longitude - current_longitude, 2));
            const auto distance_b = std::sqrtf(std::powf(b.latitude - current_latitude, 2) + std::powf(b.longitude - current_longitude, 2));
            return distance_b > distance_a;
        });
        // generate our background to be laid on top of the image
        auto overlay = GenImageColor(800, 480, Color{0,0,0,0});
        // left side
        ImageDrawRectangle(&overlay, 0, 0, 400, 480, Color{0,0,0,200});
        ImageDraw(
            &overlay,
            GenImageGradientLinear(400, 480, 90, Color{0,0,0,200}, Color{0,0,0,0}),
            Rectangle{0,0,400,480},
            Rectangle{400,0,400,480},
            WHITE
        );
        // bottom scrolling forecast area
        ImageDraw(
            &overlay,
            GenImageGradientLinear(800, meta_sz+forecast_pad, 0, Color{0,0,0,0}, BLACK),
            Rectangle{0,0,800,meta_sz+forecast_pad},
            Rectangle{0,480-meta_sz-forecast_pad,800,meta_sz+forecast_pad},
            WHITE
        );
        while (!WindowShouldClose()) {
            // fetch image
            #if defined(force_cam_id)
                auto i = load_image_from_url("https://cameras.alertcalifornia.org/public-camera-data/" + std::string(force_cam_id) + "/latest-frame.jpg");
            #else
                std::srand(std::time(NULL));
                auto cam = cams[std::rand() % 10];
                auto i = load_image_from_url("https://cameras.alertcalifornia.org/public-camera-data/" + cam.id + "/latest-frame.jpg");
            #endif
            // zoom
            ImageResize(&i, i.width/(i.height/(480*zoom_factor)), (480*zoom_factor));
            ImageCrop(&i, Rectangle{((i.width/(i.height/(480*zoom_factor))) - 800) / 2, 240*(zoom_factor-1),800,480});
            // overlay
            ImageDraw(&i, overlay, Rectangle{0,0,800,480}, Rectangle{0,0,800,480}, WHITE);
            new_img_mtx.lock();
            new_img = new Image(i);
            new_img_mtx.unlock();
            using namespace std::chrono;
            std::this_thread::sleep_for(photo_refresh_time); // lol ts is not closing
        }
    });

    while (!WindowShouldClose()) {
        BeginDrawing();
            ClearBackground(WHITE);
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
            std::strftime(time, 10, "%H:%M:%S", std::localtime(&t));
            char date[20];
            std::strftime(date, 20, "%A, %B %d", std::localtime(&t));
            char week[10];
            std::strftime(week, 10, "Week %V", std::localtime(&t));

            const auto ctr_top = (480-(clock_sz+(meta_sz*2)))/2;

            // yuck
            // draw the clock by iterating over each character
            // because it otherwise looks like shit
            auto cell_sz = GetGlyphInfo(clock_font, '0').advanceX;
            auto align_offset = ((cell_sz - GetGlyphInfo(clock_font, *std::begin(time)).advanceX) / 2);
            for (auto p = std::begin(time); p < std::end(time); p++) {
                if (*p == 0) break; // break on nul
                auto width = GetGlyphInfo(clock_font, *p).advanceX;
                DrawTextCodepoint(
                    clock_font,
                    *p,
                    Vector2{
                        .x = static_cast<float>(
                            clock_pad // adds padding to left of clock
                            + ((cell_sz - width) / 2) // centers the number in the "cell"
                            + ((p-std::begin(time)) * cell_sz) // moves the number to where it should be
                            - align_offset // minus the left pad for the first number
                        ),
                        .y = ctr_top
                    },
                    clock_sz,
                    *p == ':'
                    ? (t % 2 == 0 ? LIGHTGRAY : GRAY)
                    : WHITE
                );
            }
            DrawTextEx(meta_font, date, Vector2 { .x = clock_pad, .y = ctr_top+clock_sz }, meta_sz, 0, LIGHTGRAY);
            DrawTextEx(meta_font, week, Vector2 { .x = clock_pad, .y = ctr_top+clock_sz+meta_sz }, meta_sz, 0, LIGHTGRAY);
            // draw scrolling forecast text
            auto fct = fc.forecast_long.c_str();
            auto fctx = MeasureTextEx(meta_font, fc.forecast_long.c_str(), meta_sz, 0).x;

            // we only need to make it scroll if it's too wide
            DrawTextEx(meta_font, fct, Vector2 { 
                .x = fctx > 800 
                    ? static_cast<float>(800 - ((800 + fctx)*(std::fmod(GetTime() / (((fctx+1600)/800)*10), 1))))
                    : (800-fctx)/2, 
                .y = 480 - meta_sz - forecast_pad
            }, meta_sz, 0, LIGHTGRAY);
            
        EndDrawing();
    }

    weather_service.join();
    wildlife_photo_service.join();
    CloseWindow();

    return 0;
}