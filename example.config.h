#define current_latitude 0
#define current_longitude 0
#define fullscreen true
#define fps 30
#define weather_refresh_time minutes(5)
#define photo_refresh_time seconds(15)

// UI settings
#define clock_sz 100
#define meta_sz 40
#define forecast_pad 10
#define clock_pad 40
#define zoom_factor 1.35f
#define cam_pool 10 // n of closest cams to pull from
#define cam_selection_mode 1 // 0=random | 1=tied_to_day
#define transition_time seconds(2) // time for background transition
// #define force_cam_id "Axis-DeerCanyon2" // overrides cam_selection_mode