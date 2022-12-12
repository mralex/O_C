// Copyright (c) 2022, Alex Roberts
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "HSApplication.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define STARS_COUNT 32
#define STAR_ORIGIN_X 64
#define STAR_ORIGIN_Y 32
#define STARS_MAX_Z 64
#define SCREENSAVER_TICKS 48

typedef struct {
    int x;
    int y;
    int z;
    int velocity;
} Star;

class Hello : public HSApplication
{
public:
    void Start() {
        uint32_t _seed = OC::ADC::value<ADC_CHANNEL_1>() + OC::ADC::value<ADC_CHANNEL_2>() + OC::ADC::value<ADC_CHANNEL_3>() + OC::ADC::value<ADC_CHANNEL_4>();
        randomSeed(_seed);

        for (int i = 0; i < STARS_COUNT; i++) {
            stars[i].x = random(SCREEN_WIDTH * 10) - (STAR_ORIGIN_X * 10);
            stars[i].y = random(SCREEN_HEIGHT * 10) - (STAR_ORIGIN_Y * 10);
            stars[i].z = random(STARS_MAX_Z);
            stars[i].velocity = 1 + random(5);
        }

        screensaverTicks = SCREENSAVER_TICKS;
    }

    void Resume() {}
    void Controller() {}

    void Screensaver() {
        screensaverTicks--;

        for (int i = 0; i < STARS_COUNT; i++)
        {
            if (screensaverTicks <= 0) {
                stars[i].z -= stars[i].velocity;
            }

            int x = STAR_ORIGIN_X + stars[i].x / (stars[i].z > 0 ? stars[i].z : 1);
            int y = STAR_ORIGIN_Y + stars[i].y / (stars[i].z > 0 ? stars[i].z : 1);

            if (x < 0 || x > SCREEN_WIDTH || y < 0 || y > SCREEN_HEIGHT) {
                stars[i].z = STARS_MAX_Z;
            } else {
                gfxPixel(x, y);
            }

            if (stars[i].z <= 0) {
                stars[i].z = STARS_MAX_Z;
            }
        }

        if (screensaverTicks <= 0) {
            screensaverTicks = SCREENSAVER_TICKS;
        }
    }

    void View() {
        gfxHeader("Hello!");
        gfxPrint(5, 15, "Weird sequencers");
        gfxPrint(5, 25, "Coming soon?");
    }

private:
    Star stars[STARS_COUNT] = {};
    int screensaverTicks;
};

Hello Hello_instance;

void Hello_init() {
    Hello_instance.BaseStart();
}


size_t Hello_storageSize() {
    return 0;
}
size_t Hello_save(void *storage) {
    return 0;
}
size_t Hello_restore(const void *storage) {
    return 0;
}
    
void Hello_handleAppEvent(OC::AppEvent event) {
    if (event == OC::APP_EVENT_RESUME) {
        Hello_instance.Resume();
    }
}
    
void Hello_loop() {}

void Hello_menu () {
    Hello_instance.BaseView();
}

void Hello_screensaver() {
    Hello_instance.Screensaver();
}
    
void Hello_handleButtonEvent(const UI::Event &event) {}
    
void Hello_handleEncoderEvent(const UI::Event &event) {}
    
void Hello_isr() {
    return Hello_instance.BaseController();
}
