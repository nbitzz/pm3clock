# pm3clock
little practice project to learn C++—a clock made with raylib and curl. background images fetched from [ALERTCalifornia](https://alertcalifornia.org/), weather data from [weather.gov API](https://api.weather.gov)

![demo](./showcase.png)

## building
> [!IMPORTANT]
> curl is dynamically linked, install separately on system

first, copy [`example.config.h`](/example.config.h) into a new `config.h` file and adjust the preprocessor directives as you wish. **latlong must be specified!**

after, you can build the application with
```sh
mkdir -p build && cmake -B build -DPLATFORM=DESKTOP - && make -C build
```
for testing and 
```sh
mkdir -p build && cmake -B build -DPLATFORM=DRM && make -C build
```
to run on the pm3