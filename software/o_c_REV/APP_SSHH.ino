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
#include "OC_DAC.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define STARS_COUNT 32
#define STAR_ORIGIN_X 64
#define STAR_ORIGIN_Y 32
#define STARS_MAX_Z 64
#define SCREENSAVER_TICKS 48

// Super Sample and Hold Hold

class SSHH : public HSApplication
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

        // Initialize S&Hs
        sh_1 = random(0, HSAPPLICATION_5V);
        sh_2 = random(0, HSAPPLICATION_5V);
        sh_3 = random(0, HSAPPLICATION_5V);
        sh_4 = random(0, HSAPPLICATION_5V);
    }

    void Resume() {}
    void Controller() {
        // Update S&H when clocked
        if (Clock(0)) {
            sh_1 = random(0, HSAPPLICATION_5V);
            sh_2 = random(0, HSAPPLICATION_5V);
            sh_3 = random(0, HSAPPLICATION_5V);
            sh_4 = random(0, HSAPPLICATION_5V);
        }

        Out(DAC_CHANNEL_A, sh_1, 0);
        Out(DAC_CHANNEL_B, sh_2, 0);
        Out(DAC_CHANNEL_C, sh_3, 0);
        Out(DAC_CHANNEL_D, sh_4, 0);
    }

    void Screensaver() {
        screensaverTicks--;

        for (uint8_t i = 0; i < STARS_COUNT; i++)
        {
            if (screensaverTicks <= 0) {
                stars[i].z -= stars[i].velocity;
            }

            uint8_t x = STAR_ORIGIN_X + stars[i].x / (stars[i].z > 0 ? stars[i].z : 1);
            uint8_t y = STAR_ORIGIN_Y + stars[i].y / (stars[i].z > 0 ? stars[i].z : 1);

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
        gfxHeader("Super Sample & Hold");
    }

private:
    Star stars[STARS_COUNT] = {};
    uint8_t screensaverTicks;

    uint32_t sh_1;
    uint32_t sh_2;
    uint32_t sh_3;
    uint32_t sh_4;
};

SSHH SSHH_instance;

void SSHH_init() {
    SSHH_instance.BaseStart();
}


size_t SSHH_storageSize() {
    return 0;
}
size_t SSHH_save(void *storage) {
    return 0;
}
size_t SSHH_restore(const void *storage) {
    return 0;
}
    
void SSHH_handleAppEvent(OC::AppEvent event) {
    if (event == OC::APP_EVENT_RESUME) {
        SSHH_instance.Resume();
    }
}
    
void SSHH_loop() {} 

void SSHH_menu () {
    SSHH_instance.BaseView();
}

void SSHH_screensaver() {
    SSHH_instance.Screensaver();
}
    
void SSHH_handleButtonEvent(const UI::Event &event) {}
    
void SSHH_handleEncoderEvent(const UI::Event &event) {}
    
void SSHH_isr() {
    return SSHH_instance.BaseController();
}
