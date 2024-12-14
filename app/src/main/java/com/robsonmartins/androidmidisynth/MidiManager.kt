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
 * @file MidiManager.kt
 * @brief Kotlin Implementation of MidiManager.
 *
 * @author Robson Martins (https://www.robsonmartins.com)
 */
// -----------------------------------------------------------------------------------------------

package com.robsonmartins.androidmidisynth

import android.content.Context
import android.media.midi.MidiDevice
import android.media.midi.MidiDeviceInfo
import android.media.midi.MidiManager

/**
 * @brief MidiManager class.
 * @details The MidiManager encapsulates a MIDI listener.
 * @param context The context object.
 * @param onMidiMessageReceived Method callback to receive MIDI messages.
 */
class MidiManager(context: Context,
                  private val onMidiMessageReceived: (String) -> Unit) {

    /* @brief Android.MIDI.MidiManager instance. */
    private val midiManager = context.getSystemService(Context.MIDI_SERVICE) as MidiManager

    /** @brief Finalize the instance. */
    fun finalize()  { stopReadingMidi() }

    /** @brief Start the MIDI listener. */
    fun start() {
        // scan MIDI devices
        val deviceInfos = midiManager.devices
        for (deviceInfo in deviceInfos)  { openMidiDevice(deviceInfo) }
        // register addDevice and removeDevice listeners
        midiManager.registerDeviceCallback(
            object : MidiManager.DeviceCallback() {
                override fun onDeviceAdded(device: MidiDeviceInfo) {
                    // open MIDI device
                    openMidiDevice(device)
                }
                override fun onDeviceRemoved(device: MidiDeviceInfo) {
                    onMidiMessageReceived(
                        "Disconnect: ${device.properties.getString("product")}"
                    )
                }
        }, null)
    }

    @Suppress("unused")
    /** @brief Stop the MIDI listener. */
    fun stop() {
        stopReadingMidi()
    }

    /*
     * @brief Open the MIDI device.
     * @param deviceInfo MIDI device info.
     */
    private fun openMidiDevice(deviceInfo: MidiDeviceInfo) {
        // ignore FluidSynth MIDI device
        if (deviceInfo.properties.getString("product")?.lowercase() == "fluidsynth") return
        // open MIDI device
        midiManager.openDevice(deviceInfo, {
            onMidiMessageReceived(
                "Open: ${deviceInfo.properties.getString("product")}"
            )
            // stop current polling thread
            stopReadingMidi()
            // start new polling thread
            startReadingMidi(it, 0)
        }, null)
    }

    @Suppress("unused")
    /*
     * @brief OnNativeMessageReceive callback method.
     * @param message Received message.
     */
    private fun onNativeMessageReceive(message: ByteArray) {
        onMidiMessageReceived(String(message).trim())
    }

    /*
     * @brief   Import of the native implementation of MidiManager.startReadingMidi() method.
     * @details Opens the first "output" port from specified MIDI device for reading.
     * @param   receiveDevice  MidiDevice (Java) object.
     * @param   portNumber     The index of the "output" port to open.
     */
    private external fun startReadingMidi(receiveDevice: MidiDevice, portNumber: Int)
    /*
     * @brief  Import of the native implementation of the MidiManager.stopReadingMidi() method.
     * @details Stops MIDI device for reading.
     */
    private external fun stopReadingMidi()
}