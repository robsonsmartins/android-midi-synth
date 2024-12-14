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
 * @file cpp/SynthManager.cpp
 * @brief Implementation of SynthManager class.
 *
 * @author Robson Martins (https://www.robsonmartins.com)
 */
// -----------------------------------------------------------------------------------------------

#include <jni.h>
#include <unistd.h>

#include "MidiSpec.h"
#include "SynthManager.h"

/* @brief Default sample rate of the FluidSynth, in kHz. */
static const int kFluidSynthSampleRate = 44100;
/* @brief Default latency of the FluidSynth, in ms. */
static const int kFluidSynthLatency = 10;

/* @brief Calculate the buffer size based in latency value (ms). */
#define LATENCY_TO_BUFFER_SIZE(x) (kFluidSynthSampleRate * (x) / 1000.0)

// -----------------------------------------------------------------------------------------------

SynthManager* SynthManager::instance = nullptr;

SynthManager::SynthManager(): soundfontId(-1) {
    // setup synthesizer
    settings = new_fluid_settings();
    if (settings == nullptr) return;
    fluid_settings_setint(settings, "synth.cpu-cores", 4);
    fluid_settings_setnum(settings, "synth.gain", 0.6);
    fluid_settings_setstr(settings, "audio.oboe.performance-mode", "LowLatency");
    fluid_settings_setstr(settings, "audio.oboe.sharing-mode", "Exclusive");
    fluid_settings_setnum(settings, "synth.sample-rate", kFluidSynthSampleRate);
    setLatency(kFluidSynthLatency);
    synth = new_fluid_synth(settings);
    if (synth == nullptr) {
        delete_fluid_settings(settings);
        return;
    }
    driver = new_fluid_audio_driver(settings, synth);
    if (driver == nullptr) {
        delete_fluid_synth(synth);
        delete_fluid_settings(settings);
        return;
    }
}

SynthManager::~SynthManager() {
    // clean up
    if (synth && soundfontId != -1) fluid_synth_sfunload(synth, soundfontId, 1);
    if (driver) delete_fluid_audio_driver(driver);
    if (synth) delete_fluid_synth(synth);
    if (settings) delete_fluid_settings(settings);
}

SynthManager* SynthManager::getInstance() {
    if (!instance) instance = new SynthManager();
    return instance;
}

void SynthManager::freeInstance() {
    if (instance) {
        delete instance;
        instance = nullptr;
    }
}

bool SynthManager::loadSF(const char *soundfontPath, int program) {
    if (synth == nullptr) return false;
    // load soundfont
    int id = fluid_synth_sfload(synth, soundfontPath, 0);
    if (id == FLUID_FAILED) return false;
    fluid_synth_sfont_select(synth, 0, id);
    fluid_synth_program_change(synth, 0, program);
    soundfontId = id;
    return true;
}

void SynthManager::noteOn(int note, int velocity) {
    if (synth == nullptr) return;
    fluid_synth_noteon(synth, 0, note, velocity);
}

void SynthManager::noteOff(int note) {
    if (synth == nullptr) return;
    fluid_synth_noteoff(synth, 0, note);
}

void SynthManager::reverb(int level) {
    if (synth == nullptr) return;
    fluid_synth_reverb_on(synth, -1, level > 0);
    fluid_synth_set_reverb_group_level(synth, -1, level / 127.0);
}

void SynthManager::sendCC(int controller, int value) {
    if (synth == nullptr) return;
    fluid_synth_cc(synth, 0, controller, value);
}

void SynthManager::setLatency(int ms){
    double bufferSizeInSamples = LATENCY_TO_BUFFER_SIZE(ms);
    fluid_settings_setnum(settings, "audio.period-size", bufferSizeInSamples);
    fluid_settings_setint(settings, "audio.periods", 2);
}

// -----------------------------------------------------------------------------------------------

extern "C" {

/**
 * @brief   Native implementation of SynthManager.fluidsynthInit() method.
 * @details Initializes the FluidSynth library.
 * @param   env            JNI Env pointer.
 * @param   (unnamed)      SynthManager (Java) object.
 */
JNIEXPORT void JNICALL
Java_com_robsonmartins_androidmidisynth_SynthManager_fluidsynthInit(
        JNIEnv *env, jobject) {
    SynthManager::getInstance();
}

/**
 * @brief   Native implementation of SynthManager.fluidsynthLoadSF() method.
 * @details Loads a soundfont file.
 * @param   env            JNI Env pointer.
 * @param   (unnamed)      SynthManager (Java) object.
 * @param   jSoundfontPath The soundfont filename full path.
 * @param   program        The number of the program
 */
JNIEXPORT int JNICALL
Java_com_robsonmartins_androidmidisynth_SynthManager_fluidsynthLoadSF(
        JNIEnv *env, jobject, jstring jSoundfontPath, int program) {
    // convert Java string to C string
    const char *soundfontPath = env->GetStringUTFChars(jSoundfontPath, nullptr);
    return SynthManager::getInstance()->loadSF(soundfontPath, program) ? 0 : -1;
}

/**
 * @brief   Native implementation of SynthManager.fluidsynthFree() method.
 * @details Finalizes the FluidSynth library.
 * @param   env            JNI Env pointer.
 * @param   (unnamed)      SynthManager (Java) object.
 */
JNIEXPORT void JNICALL
Java_com_robsonmartins_androidmidisynth_SynthManager_fluidsynthFree(
        JNIEnv *env, jobject) {
    SynthManager::freeInstance();
}

/**
 * @brief   Native implementation of SynthManager.fluidsynthNoteOn() method.
 * @details Plays the note.
 * @param   env            JNI Env pointer.
 * @param   (unnamed)      SynthManager (Java) object.
 * @param   note           The note to be played.
 * @param   velocity       The velocity of the note to be played.
 */
JNIEXPORT void JNICALL
Java_com_robsonmartins_androidmidisynth_SynthManager_fluidsynthNoteOn(
        JNIEnv *env, jobject, int note, int velocity) {
    SynthManager::getInstance()->noteOn(note, velocity);
}

/**
 * @brief   Native implementation of SynthManager.fluidsynthNoteOff() method.
 * @details Stops the playing note.
 * @param   env            JNI Env pointer.
 * @param   (unnamed)      SynthManager (Java) object.
 * @param   note           The note to be stopped.
 */
JNIEXPORT void JNICALL
Java_com_robsonmartins_androidmidisynth_SynthManager_fluidsynthNoteOff(
        JNIEnv *env, jobject, int note) {
    SynthManager::getInstance()->noteOff(note);
}

/**
 * @brief   Native implementation of SynthManager.fluidsynthCC() method.
 * @details Sends a control command via MIDI.
 * @param   env            JNI Env pointer.
 * @param   (unnamed)      SynthManager (Java) object.
 * @param   controller     Number of the controller.
 * @param   value          Value to send.
 */
JNIEXPORT void JNICALL
Java_com_robsonmartins_androidmidisynth_SynthManager_fluidsynthCC(
        JNIEnv *env, jobject, int controller, int value) {
    SynthManager::getInstance()->sendCC(controller, value);
}

/**
 * @brief   Native implementation of SynthManager.fluidsynthReverb() method.
 * @details Sets the reverb level.
 * @param   env            JNI Env pointer.
 * @param   (unnamed)      SynthManager (Java) object.
 * @param   level          The reverb level (0 to 127).
 */
JNIEXPORT void JNICALL
Java_com_robsonmartins_androidmidisynth_SynthManager_fluidsynthReverb(
        JNIEnv *env, jobject, int level) {
    SynthManager::getInstance()->reverb(level);
}

} // extern "C"
