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

class Hello : public HSApplication {
public:
    void Start() {}
    void Resume() {}
    void Controller() {}

    void View() {
        gfxHeader("Hello!");
        gfxPrint(5, 15, "Weird sequencers");
        gfxPrint(5, 25, "Coming soon?");
    }
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

void Hello_screensaver() {}
    
void Hello_handleButtonEvent(const UI::Event &event) {}
    
void Hello_handleEncoderEvent(const UI::Event &event) {}
    
void Hello_isr() {
    return Hello_instance.BaseController();
}
