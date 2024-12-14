/*
 * Copyright (c) 2024 Robson Martins
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
// -----------------------------------------------------------------------------------------------
/**
 * @file cpp/MidiSpec.h
 * @brief Header of MIDI Spec Constants.
 *
 * @author Robson Martins (https://www.robsonmartins.com)
 */
// -----------------------------------------------------------------------------------------------

#ifndef ANDROID_MIDI_SYNTH_MIDISPEC_H
#define ANDROID_MIDI_SYNTH_MIDISPEC_H

#include <cstdint>

//
// MIDI Messages
//

// Channel Commands
/** @brief MIDI Note OFF (08). */
static const uint8_t kMIDIChanCmd_NoteOff       = 0x08;
/** @brief MIDI Note ON (09). */
static const uint8_t kMIDIChanCmd_NoteOn        = 0x09;
/** @brief MIDI Key Pressure (10). */
static const uint8_t kMIDIChanCmd_KeyPress     = 0x0A;
/** @brief MIDI Control Change (11). */
static const uint8_t kMIDIChanCmd_Control       = 0x0B;
/** @brief MIDI Program Change (12). */
static const uint8_t kMIDIChanCmd_ProgramChange = 0x0C;
/** @brief MIDI Channel Pressure (13). */
static const uint8_t kMIDIChanCmd_ChannelPress  = 0x0D;
/** @brief MIDI Pitch Bend (14). */
static const uint8_t kMIDIChanCmd_PitchWheel    = 0x0E;
// Control Commands
/** @brief MIDI Control: Sustain Pedal (64). */
static const uint8_t kMIDIControl_Sustain       = 0x40;
/** @brief MIDI Control: Sustain Level to on/off. */
static const uint8_t kMIDIControl_Sustain_level = 64;
/** @brief MIDI Control: Effects 1 Depth - Reverb Send Level (91). */
static const uint8_t kMIDIControl_Reverb        = 0x5B;
// System Commands
/** @brief MIDI Channel Command mask. */
static const uint8_t kMIDISysCmdChan            = 0xF0;
/** @brief MIDI System: SysEx (240). */
static const uint8_t kMIDISysCmd_SysEx          = 0xF0;
/** @brief MIDI System: End Of SysEx (247). */
static const uint8_t kMIDISysCmd_EndOfSysEx     = 0xF7;
/** @brief MIDI System: Active Sensing (254). */
static const uint8_t kMIDISysCmd_ActiveSensing  = 0xFE;
/** @brief MIDI System: Reset (255). */
static const uint8_t kMIDISysCmd_Reset          = 0xFF;

#endif //ANDROID_MIDI_SYNTH_MIDISPEC_H

