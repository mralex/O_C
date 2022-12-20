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

// Slice & Dice
// 
// A compact sequencer inspired by Vermona meloDicer.

// #include <stdint.h>
#include "HSApplication.h"

enum notes {
    NOTE_C = 0,
    NOTE_CC,
    NOTE_D,
    NOTE_DD,
    NOTE_E,
    NOTE_F,
    NOTE_FF,
    NOTE_G,
    NOTE_GG,
    NOTE_A,
    NOTE_AA,
    NOTE_B,
    NOTE_COUNT,
};

class NoteBucket {
public:
    NoteBucket() : note_weights{}, notes_weighted{}, accumulated_weight(0) {
        for(uint8_t i = 0; i < NOTE_COUNT; i++) {
            note_weights[i] = 0;
            notes_weighted[i] = 0;
        }
        // UpdateNoteWeight(NOTE_C, 75);
        // UpdateNoteWeight(NOTE_E, 45);
        // UpdateNoteWeight(NOTE_F, 65);
        UpdateNoteWeight(NOTE_C, 78);
        UpdateNoteWeight(NOTE_D, 45);
        UpdateNoteWeight(NOTE_DD, 65);
        UpdateNoteWeight(NOTE_F, 75);
        UpdateNoteWeight(NOTE_G, 45);
        UpdateNoteWeight(NOTE_GG, 35);
        UpdateNoteWeight(NOTE_AA, 45);
    }

    void UpdateNoteWeight(uint8_t note, uint8_t weight) {
        note_weights[note] = weight;

        uint16_t accumulated_weight = 0;
        for(int i = 0; i < notes::NOTE_COUNT; i++) {
            accumulated_weight += note_weights[i];
            notes_weighted[i] = accumulated_weight;
        }
        this->accumulated_weight = accumulated_weight;
    }

    int GetRandom() {
        uint16_t w = random(accumulated_weight);
        for (uint8_t i = 0; i < notes::NOTE_COUNT; i++)
        {
            if (notes_weighted[i] >= w) {
                return i;
            }
        }

        return 0;
    }

    uint8_t note_weights[notes::NOTE_COUNT];
    uint16_t notes_weighted[notes::NOTE_COUNT];
    uint16_t accumulated_weight;
};

class Dicer : public HSApplication
{
public:
    Dicer() : noteBucket() {}

    void Start() {
        uint32_t _seed = OC::ADC::value<ADC_CHANNEL_1>() + OC::ADC::value<ADC_CHANNEL_2>() + OC::ADC::value<ADC_CHANNEL_3>() + OC::ADC::value<ADC_CHANNEL_4>();
        randomSeed(_seed);
    }

    void Resume() {}
    void Controller() {
        // Get a note to play 
        if (Clock(0))// && !clockHigh)
        {
            input = Proportion(DetentedIn(0), 7000, 100);
            gate = random(100) + 1 > input;

            if (gate) {
                activeNote = noteBucket.GetRandom();
                activeOctave = random(minOctave, maxOctave + 1);
            }

            clockHigh = true;
            // Leave gate open for 50% PWM
            gate_timeout = ClockCycleTicks(0) / 2;
        } else if (--gate_timeout <= 0) { // if (!Clock(0) && clockHigh){
            gate = false;
            clockHigh = false;
        }

        // Gate Out
        // TODO: Custom note timings. Echo the clock (and clock pulse width) for now
        GateOut(0, gate);

        // CV Out
        SemitoneOut(DAC_CHANNEL_B, activeNote, activeOctave);
    }

    void Screensaver() {
    }

    void View() {
        gfxHeader("Slice & Dice");
        gfxPrint(10, 20, activeNote);
        gfxPrint(20, 20, (int)gate);
        gfxPrint(10, 32,ClockCycleTicks(0));
        gfxPrint(10, 40, input);
    }

private:
    uint8_t minOctave = 1;
    uint8_t maxOctave = 2;
    NoteBucket noteBucket;

    bool clockHigh = false;
    uint8_t restChance = 20;
    bool gate = false;
    uint32_t activeNote = 0;
    uint32_t activeOctave = 0;

    uint32_t input = 0;

    int gate_timeout;
};

Dicer Dicer_instance;

void Dicer_init() {
    Dicer_instance.BaseStart();
}


size_t Dicer_storageSize() {
    return 0;
}
size_t Dicer_save(void *storage) {
    return 0;
}
size_t Dicer_restore(const void *storage) {
    return 0;
}
    
void Dicer_handleAppEvent(OC::AppEvent event) {
    if (event == OC::APP_EVENT_RESUME) {
        Dicer_instance.Resume();
    }
}
    
void Dicer_loop() {}

void Dicer_menu () {
    Dicer_instance.BaseView();
}

void Dicer_screensaver() {
    Dicer_instance.Screensaver();
}
    
void Dicer_handleButtonEvent(const UI::Event &event) {}
    
void Dicer_handleEncoderEvent(const UI::Event &event) {}
    
void Dicer_isr() {
    return Dicer_instance.BaseController();
}
