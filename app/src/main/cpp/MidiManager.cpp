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
 * @file cpp/MidiManager.cpp
 * @brief Implementation of MidiManager class.
 *
 * @author Robson Martins (https://www.robsonmartins.com)
 */
// -----------------------------------------------------------------------------------------------

#include <unistd.h>

#include <cstdio>
#include <string>
#include <sstream>

#include "MidiSpec.h"
#include "MidiManager.h"
#include "SynthManager.h"

/* @brief Buffer size to receive data from MIDI, in bytes. */
static const size_t kMidiMaxBytesToReceive = 128;

// -----------------------------------------------------------------------------------------------

MidiManager* MidiManager::instance = nullptr;
JavaVM* MidiManager::jvm = nullptr;
SynthManager* MidiManager::synthManager = nullptr;
jobject MidiManager::callbackObj = nullptr;
jmethodID MidiManager::callback = nullptr;

MidiManager::MidiManager(JNIEnv* env, jobject midiDeviceObj, jint portNumber)
                : reading(false), sustain(false) {
    synthManager = SynthManager::getInstance();
    AMidiDevice* device;
    AMidiDevice_fromJava(env, midiDeviceObj, &device);
    nativeReceiveDevice = device;
    AMidiOutputPort* port;
    AMidiOutputPort_open(nativeReceiveDevice, portNumber, &port);
    midiOutputPort = port;
    pthread_t thread;
    pthread_create(&thread, nullptr, readThreadRoutine, this);
    readThread = thread;
}

MidiManager::~MidiManager() {
    reading = false;
    pthread_join(readThread, nullptr);
    AMidiDevice_release(nativeReceiveDevice);
}

MidiManager* MidiManager::getInstance(JNIEnv* env, jobject midiManagerObj,
                                      jobject midiDeviceObj, jint portNumber) {
    if (!instance) {
        // setup the receive data callback (into Java)
        env->GetJavaVM(&jvm);
        jclass objectClass = env->GetObjectClass(midiManagerObj);
        callbackObj = env->NewGlobalRef(midiManagerObj);
        callback = env->GetMethodID(objectClass, "onNativeMessageReceive", "([B)V");
        instance = new MidiManager(env, midiDeviceObj, portNumber);
    }
    return instance;
}

void MidiManager::freeInstance() {
    if (instance) {
        delete instance;
        instance = nullptr;
    }
}

void MidiManager::parseMidiData(const uint8_t *data, size_t numBytes) {
    if (numBytes < 3) return;
    uint8_t status = data[0] & 0xFF;
    uint8_t note = data[1] & 0xFF;
    uint8_t velocity = data[2] & 0xFF;
    std::ostringstream oss;
    switch ((status & kMIDISysCmdChan) >> 4) {
        case kMIDIChanCmd_NoteOff:
            oss.clear(); oss << "Note OFF: " << (int)note;
            sendToCallback(oss);
            if (!sustain) {
                synthManager->noteOff(note);
                sustainNotes.erase(note);
            }
            playNotes.erase(note);
            break;
        case kMIDIChanCmd_NoteOn:
            oss.clear(); oss << "Note ON: " << (int)note << " vel: " << (int)velocity;
            sendToCallback(oss);
            if (sustain) sustainNotes.insert(note);
            playNotes.insert(note);
            synthManager->noteOn(note, velocity);
            break;
        case kMIDIChanCmd_Control:
            parseMidiCmdControl(note, velocity);
            break;
        case kMIDIChanCmd_KeyPress:
            oss.clear();
            oss << "Key Press: "
                << (int)note << " vel: " << (int)velocity << " status: " << (int)status;
            sendToCallback(oss);
            break;
        case kMIDIChanCmd_ProgramChange:
            oss.clear();
            oss << "Program Change: "
                << (int)note << " vel: " << (int)velocity << " status: " << (int)status;
            sendToCallback(oss);
            break;
        case kMIDIChanCmd_ChannelPress:
            oss.clear();
            oss << "Channel Press: "
                << (int)note << " vel: " << (int)velocity << " status: " << (int)status;
            sendToCallback(oss);
            break;
        case kMIDIChanCmd_PitchWheel:
            oss.clear();
            oss << "Pitch Wheel: "
                << (int)note << " vel: " << (int)velocity << " status: " << (int)status;
            sendToCallback(oss);
            break;
        default:
            oss.clear();
            oss << "Unparsed: "
                << (int)note << " vel: " << (int)velocity << " status: " << (int)status;
            sendToCallback(oss);
            break;
    }
}

void MidiManager::parseMidiCmdControl(uint8_t controller, uint8_t value) {
    std::ostringstream oss;
    switch (controller) {
        case kMIDIControl_Sustain:
            if (value >= kMIDIControl_Sustain_level) {
                oss.clear(); oss << "Sustain ON: level: " << (int)value;
                sendToCallback(oss);
                sustain = true;
                if (!playNotes.empty()) {
                    sustainNotes.insert(playNotes.begin(), playNotes.end());
                }
            } else {
                oss.clear(); oss << "Sustain OFF: level: " << (int)value;
                sendToCallback(oss);
                sustain = false;
                // Stop all sustained notes
                for (const auto n: sustainNotes) {
                    if (playNotes.find(n) == playNotes.end()) synthManager->noteOff(n);
                }
                sustainNotes.clear();
            }
            break;
        case kMIDIControl_Reverb:
            oss.clear(); oss << "Reverb: level: " << (int)value;
            sendToCallback(oss);
            synthManager->reverb(value);
            break;
        default:
            oss.clear();
            oss << "Unparsed command: controller: "
                << (int)controller << " value: " << (int)value;
            sendToCallback(oss);
            synthManager->sendCC(controller, value);
            break;
    }
}

void MidiManager::sendToCallback(uint8_t* data, int size) {
    JNIEnv* env;
    jvm->AttachCurrentThread(&env, nullptr);
    if (!env) return;
    // allocate the Java array and fill with received data
    jbyteArray ret = env->NewByteArray(size);
    env->SetByteArrayRegion (ret, 0, size, (jbyte*)data);
    // send it to the (Java) callback
    env->CallVoidMethod(callbackObj, callback, ret);
}

void MidiManager::sendToCallback(const char *str) {
    if (!str || str[0] == 0) return;
    sendToCallback((uint8_t*)str, (int)strlen(str) + 1);
}

void MidiManager::sendToCallback(const std::ostringstream& oss) {
    sendToCallback(oss.str().c_str());
}

// -----------------------------------------------------------------------------------------------

/* This routine polls the input port and parses received data. */
static void* readThreadRoutine(void *context) {
    auto *manager = (MidiManager*)context;
    manager->reading = true;
    AMidiOutputPort* outputPort = manager->midiOutputPort;
    uint8_t incomingMessage[kMidiMaxBytesToReceive];
    int32_t opcode;
    size_t numBytesReceived;
    int64_t timestamp;
    ssize_t numMessagesReceived;
    while (manager->reading) {
        numMessagesReceived = AMidiOutputPort_receive(
                outputPort, &opcode, incomingMessage, kMidiMaxBytesToReceive,
                &numBytesReceived, &timestamp);

        if (numMessagesReceived < 0) {
            // failure receiving MIDI data: exit the thread
            manager->reading = false;
        }
        if (numMessagesReceived > 0 && numBytesReceived >= 0 &&
            opcode == AMIDI_OPCODE_DATA &&
            (incomingMessage[0] & kMIDISysCmdChan) != kMIDISysCmdChan) {
            manager->parseMidiData(incomingMessage, numBytesReceived);
        }
        // AMidiOutputPort_receive is non-blocking
        usleep(500);
    }
    return nullptr;
}

// -----------------------------------------------------------------------------------------------

extern "C" {

/**
 * @brief   Native implementation of MidiManager.startReadingMidi() method.
 * @details Opens the first "output" port from specified MIDI device for reading.
 * @param   env            JNI Env pointer.
 * @param   midiManagerObj MidiManager (Java) object.
 * @param   midiDeviceObj  MidiDevice (Java) object.
 * @param   portNumber     The index of the "output" port to open.
 */
JNIEXPORT void JNICALL
Java_com_robsonmartins_androidmidisynth_MidiManager_startReadingMidi(
        JNIEnv* env, jobject midiManagerObj, jobject midiDeviceObj, jint portNumber) {
    // starts the midi manager
    MidiManager::getInstance(env, midiManagerObj, midiDeviceObj, portNumber);
}

/**
 * @brief  Native implementation of the (Java) MidiManager.stopReadingMidi() method.
 * @details Stops MIDI device for reading.
 * @param  (unnamed)   JNI Env pointer.
 * @param  (unnamed)   MidiManager (Java) object.
 */
JNIEXPORT void JNICALL
Java_com_robsonmartins_androidmidisynth_MidiManager_stopReadingMidi(
        JNIEnv*, jobject) {
    MidiManager::freeInstance();
}

} // extern "C"
