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
#include "braids_quantizer.h"
#include "OC_scales.h"

// Semitones (5) default
const int DEFAULT_SCALE = 8;
const int MAX_NOTES = 16;
const int MAX_VALUE = 99;

enum notes
{
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

class NoteBucket
{
public:
    NoteBucket() : note_weights{}, notes_weighted{}, accumulated_weight(0), scale(OC::Scales::GetScale(DEFAULT_SCALE))
    {
        for (uint8_t i = 0; i < MAX_NOTES; i++)
        {
            note_weights[i] = 0;
            notes_weighted[i] = 0;
        }

        RerollWeights();
    }

    void UpdateNoteWeight(uint8_t note, uint8_t weight)
    {
        note_weights[note] = weight;

        updateWeights();
    }

    int GetRandom()
    {
        if (accumulated_weight == 0)
        {
            // no notes are selected
            // FIXME: Returning the root note, not sure what else should be done? Don't trigger?
            return scale.notes[0];
        }

        uint16_t w = random(accumulated_weight);
        for (uint8_t i = 0; i < scale.num_notes; i++)
        {
            if (notes_weighted[i] >= w)
            {
                return scale.notes[i];
            }
        }

        return scale.notes[0];
    }

    void SetScale(braids::Scale newScale)
    {
        scale = newScale;
        updateWeights();
    }

    void RerollWeights()
    {
        for (uint8_t i = 0; i < scale.num_notes; i++)
        {
            UpdateNoteWeight(i, random(MAX_VALUE));
        }
    }

    uint8_t note_weights[MAX_NOTES];
    uint16_t notes_weighted[MAX_NOTES];
    uint16_t accumulated_weight;

    braids::Scale scale;

    void updateWeights()
    {
        uint16_t accumulated_weight = 0;
        for (uint8_t i = 0; i < scale.num_notes; i++)
        {
            accumulated_weight += note_weights[i];
            notes_weighted[i] = accumulated_weight;
        }
        this->accumulated_weight = accumulated_weight;
    }
};

class Dicer : public HSApplication
{
public:
    Dicer() : noteBucket() {}

    void Start()
    {
        uint32_t _seed = OC::ADC::value<ADC_CHANNEL_1>() + OC::ADC::value<ADC_CHANNEL_2>() + OC::ADC::value<ADC_CHANNEL_3>() + OC::ADC::value<ADC_CHANNEL_4>();
        randomSeed(_seed);
    }

    void Resume() {}
    void Controller()
    {
        if (gate_timeout > 0)
        {
            gate_timeout--;
        }

        // Get a note to play
        if (Clock(0))
        {
            input = Proportion(DetentedIn(0), 7000, 100);
            gate = random(100) + 1 > input;

            if (gate)
            {
                activePitch = noteBucket.GetRandom();
                activeOctave = random(minOctave, maxOctave + 1);

                // Leave gate open for 50% PWM
                gate_timeout = ClockCycleTicks(0) / 2;
            }
        }
        else if (gate_timeout <= 0)
        {
            gate = false;
        }

        // Gate Out
        // TODO: Custom note timings. Echo the clock (and clock pulse width) for now
        GateOut(0, gate);

        // CV Out
        Out(DAC_CHANNEL_B, activePitch, activeOctave);
    }

    void Screensaver()
    {
    }

    void View()
    {
        gfxHeader("Slice & Dice");
        int x = 10;
        int x_incr = 15;
        int y = 20;
        for (uint8_t i = 0; i < noteBucket.scale.num_notes; i++)
        {
            gfxPrint(x, y, noteBucket.note_weights[i]);
            x += x_incr;
        }

        x = 10 + (x_incr * selected_note);
        y = 31;
        gfxDottedLine(x, y, x + 10, y);

        y += 17;
        gfxPrint(10, y, activePitch);
        gfxPrint(50, y, (int)gate);
        y += 15;
        gfxPrint(10, y, input);
    }

    void RerollNoteWeights()
    {
        noteBucket.RerollWeights();
    }

    void UpdateSelectedNote(const UI::Event &event)
    {
        int value = selected_note + event.value;
        if (value < 0)
        {
            value = noteBucket.scale.num_notes - 1;
        }
        selected_note = value % noteBucket.scale.num_notes;
    }

    void UpdateSelectedNoteValue(const UI::Event &event)
    {
        int value = noteBucket.note_weights[selected_note] + event.value;
        if (value < 0)
        {
            value = 0;
        }
        else if (value > MAX_VALUE)
        {
            value = MAX_VALUE;
        }

        noteBucket.UpdateNoteWeight(selected_note, value);
    }

private:
    uint8_t minOctave = 1;
    uint8_t maxOctave = 2;
    NoteBucket noteBucket;

    uint8_t restChance = 20;
    bool gate = false;
    uint32_t activePitch = 0;
    uint32_t activeOctave = 0;

    uint32_t input = 0;

    int gate_timeout;

    int scale_index = DEFAULT_SCALE;

    int selected_note = 0;
};

Dicer Dicer_instance;

void Dicer_init()
{
    Dicer_instance.BaseStart();
}

size_t Dicer_storageSize()
{
    return 0;
}
size_t Dicer_save(void *storage)
{
    return 0;
}
size_t Dicer_restore(const void *storage)
{
    return 0;
}

void Dicer_handleAppEvent(OC::AppEvent event)
{
    if (event == OC::APP_EVENT_RESUME)
    {
        Dicer_instance.Resume();
    }
}

void Dicer_loop() {}

void Dicer_menu()
{
    Dicer_instance.BaseView();
}

void Dicer_screensaver()
{
    Dicer_instance.Screensaver();
}

void Dicer_handleButtonEvent(const UI::Event &event)
{
    if (UI::EVENT_BUTTON_PRESS == event.type)
    {
        if (event.control == OC::CONTROL_BUTTON_DOWN)
        {
            Dicer_instance.RerollNoteWeights();
        }
    }
}

void Dicer_handleEncoderEvent(const UI::Event &event)
{
    if (OC::CONTROL_ENCODER_L == event.control)
    {
        Dicer_instance.UpdateSelectedNote(event);
    }
    if (OC::CONTROL_ENCODER_R == event.control)
    {
        Dicer_instance.UpdateSelectedNoteValue(event);
    }
}

void Dicer_isr()
{
    return Dicer_instance.BaseController();
}
