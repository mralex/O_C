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
const int DEFAULT_SCALE = 11; // Aeolian
const int MAX_NOTES = 16;
const int MAX_VALUE = 99;
const int LINE_SPACE = 3;

const int MAX_OCTAVE_VALUE = 7;
const int MIN_SCALE = 5;
const int MAX_SCALE = 16;

enum SETTINGS
{
    SLICES,
    SCALE,
    MIN_OCTAVE,
    MAX_OCTAVE,
    ROOT,
    SETTINGS_COUNT
};

class NoteBucket
{
public:
    NoteBucket() : note_weights{}, notes_weighted{}, accumulated_weight(0), scale_idx(DEFAULT_SCALE), scale(OC::Scales::GetScale(DEFAULT_SCALE))
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
            last_note = 0;
            return scale.notes[0] + root_pitch;
        }

        uint16_t w = random(accumulated_weight);
        for (uint8_t i = 0; i < scale.num_notes; i++)
        {
            if (notes_weighted[i] >= w)
            {
                last_note = i;
                return scale.notes[i] + root_pitch;
            }
        }

        return scale.notes[0] + root_pitch;
    }

    void SetScale(int index)
    {
        scale_idx = index;
        scale = OC::Scales::GetScale(scale_idx);
        updateWeights();
    }

    void SetRoot(int r)
    {
        root = r;
        root_pitch = root << 7;
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
    uint8_t last_note = 0;
    int8_t root = 0;
    int32_t root_pitch = 0;

    uint32_t scale_idx;
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
        if (saver_gate_timeout > 0)
        {
            saver_gate_timeout--;
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
                saver_gate_timeout = ClockCycleTicks(0);
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
        int x = 10;
        int width = 118 / noteBucket.scale.num_notes;
        int y = 22;

        if (saver_gate_timeout > 0)
            gfxRect(x + (width * noteBucket.last_note), y, width, 20);
    }

    void View()
    {
        gfxHeader("Slice & Dice");

        if (gate)
            gfxRect(115, 2, 5, 5);

        int x = 10;
        int x_incr = 15;
        int y = 17;

        gfxPixel(x + 7 + (x_incr * noteBucket.last_note), y);

        y += 3;
        for (uint8_t i = 0; i < noteBucket.scale.num_notes; i++)
        {
            gfxPrint(x, y, noteBucket.note_weights[i]);
            x += x_incr;
        }

        x = 10;
        y += 11;
        if (selected_setting == SETTINGS::SLICES)
        {
            if (editing)
            {
                x = 10 + (x_incr * selected_note);
                gfxDottedLine(x, y, x + 10, y);
            }
            else
            {
                gfxDottedLine(x, y, x + (x_incr * noteBucket.scale.num_notes), y);
            }
        }

        y += LINE_SPACE;
        gfxPrint(10, y, OC::scale_names[noteBucket.scale_idx]);

        y += 11;
        if (selected_setting == SETTINGS::SCALE)
        {
            gfxDottedLine(10, y, 10 + 28, y);
        }

        y += LINE_SPACE;
        gfxPrint(10, y, minOctave);
        gfxPrint(20, y, maxOctave);

        gfxPrint(60, y, OC::Strings::note_names_unpadded[noteBucket.root]);

        y += 11;
        if (selected_setting == SETTINGS::MIN_OCTAVE)
        {
            gfxDottedLine(10, y, 10 + 6, y);
        }
        if (selected_setting == SETTINGS::MAX_OCTAVE)
        {
            gfxDottedLine(20, y, 20 + 6, y);
        }
        if (selected_setting == SETTINGS::ROOT)
        {
            gfxDottedLine(60, y, 60 + 12, y);
        }
    }

    void RerollNoteWeights()
    {
        noteBucket.RerollWeights();
    }

    void ScrollSetting(const UI::Event &event)
    {
        if (editing)
        {
            UpdateSelectedNote(event);
        }
        else
        {
            int value = selected_setting + event.value;
            if (value < 0)
            {
                value = SETTINGS::SETTINGS_COUNT - 1;
            }
            value %= SETTINGS::SETTINGS_COUNT;
            selected_setting = value;
        }
    }

    void EditSetting(const UI::Event &event)
    {
        if (selected_setting == SETTINGS::SLICES && editing)
        {
            UpdateSelectedNoteValue(event);
            return;
        }

        if (selected_setting == SETTINGS::MIN_OCTAVE)
        {
            int value = minOctave + event.value;
            if (value < 0)
            {
                value = 0;
            }
            else if (value > maxOctave)
            {
                value = maxOctave;
            }
            else if (value > MAX_OCTAVE_VALUE)
            {
                value = MAX_OCTAVE_VALUE;
            }
            minOctave = value;
        }

        if (selected_setting == SETTINGS::MAX_OCTAVE)
        {
            int value = maxOctave + event.value;
            if (value < 0)
            {
                value = 0;
            }
            else if (value < minOctave)
            {
                value = minOctave;
            }
            else if (value > MAX_OCTAVE_VALUE)
            {
                value = MAX_OCTAVE_VALUE;
            }
            maxOctave = value;
        }

        if (selected_setting == SETTINGS::SCALE)
        {
            int value = noteBucket.scale_idx + event.value;
            if (value < MIN_SCALE)
            {
                value = MAX_SCALE;
            }
            else if (value > MAX_SCALE)
            {
                value = MIN_SCALE;
            }
            noteBucket.SetScale(value);
        }

        if (selected_setting == SETTINGS::ROOT)
        {
            int value = noteBucket.root + event.value;
            CONSTRAIN(value, 0, 11);
            noteBucket.SetRoot(value);
        }
    }

    void ToggleEditingSelection(const UI::Event &event)
    {
        if (selected_setting == SETTINGS::SLICES)
        {
            editing = !editing;
        }
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
    int saver_gate_timeout;

    int scale_index = DEFAULT_SCALE;

    int selected_setting = 0;
    bool editing = false;
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
        else if (event.control == OC::CONTROL_BUTTON_R)
        {
            // Pressed right encoder
            Dicer_instance.ToggleEditingSelection(event);
        }
    }
}

void Dicer_handleEncoderEvent(const UI::Event &event)
{
    if (OC::CONTROL_ENCODER_L == event.control)
    {
        Dicer_instance.ScrollSetting(event);
    }
    if (OC::CONTROL_ENCODER_R == event.control)
    {
        Dicer_instance.EditSetting(event);
    }
}

void Dicer_isr()
{
    return Dicer_instance.BaseController();
}
