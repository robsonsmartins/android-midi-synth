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
 * @file SynthManager.kt
 * @brief Kotlin Implementation of SynthManager.
 *
 * @author Robson Martins (https://www.robsonmartins.com)
 */
// -----------------------------------------------------------------------------------------------

package com.robsonmartins.androidmidisynth

import android.content.Context
import androidx.appcompat.app.AppCompatActivity.MODE_PRIVATE
import java.io.IOException

/**
 * @brief SynthManager class.
 * @details The SynthManager encapsulates a FluidSynth synthesizer.
 * @param context The context object.
 */
class SynthManager(private val context: Context) {

    /* @brief Soundfont file path. */
    private var soundFontPath: String? = null

    /** @brief Initialize the instance. */
    init { fluidsynthInit() }

    /** @brief Finalize the instance. */
    fun finalize()  { fluidsynthFree() }

    /**
     * @brief Load a soundfont file.
     * @param filename The soundfont filename.
     * @param program Program number to select (default = 0).
     */
    fun loadSF(filename: String, program: Int = 0) {
        try {
            soundFontPath = copyAssetToTmpFile(filename)
            if (fluidsynthLoadSF(soundFontPath, program) < 0) {
                throw IOException("Error loading $filename")
            }
        } catch (e: IOException) {
            throw RuntimeException(e)
        }
    }

    /**
     * @brief Set synth volume.
     * @param volume The volume level.
     */
    fun setVolume(volume: Int) {
        fluidsynthCC(7, volume)
    }

    @Throws(IOException::class)
    /*
     * @brief Copy asset file to the temporary directory.
     * @param filename Asset filename.
     * @return The filename in temporary directory.
     */
    private fun copyAssetToTmpFile(filename: String): String {
        context.assets.open(filename).use { `is` ->
            val tempFilename = "tmp_$filename"
            context.openFileOutput(tempFilename, MODE_PRIVATE).use { fos ->
                var bytesRead: Int
                val buffer = ByteArray(4096)
                while ((`is`.read(buffer).also { bytesRead = it }) != -1) {
                    fos.write(buffer, 0, bytesRead)
                }
            }
            val filesDir = context.filesDir
            return "$filesDir/$tempFilename"
        }
    }

    /*
     * @brief   Import of the native implementation of SynthManager.fluidsynthInit() method.
     * @details Initializes the FluidSynth library.
     */
    private external fun fluidsynthInit()
    /*
     * @brief   Import of the native implementation of SynthManager.fluidsynthLoadSF() method.
     * @details Loads a soundfont file.
     * @param   soundfontPath The soundfont filename full path.
     * @param   program       The number of the program
     */
    private external fun fluidsynthLoadSF(soundfontPath: String?, program: Int): Int
    /*
     * @brief   Import of the native implementation of SynthManager.fluidsynthFree() method.
     * @details Finalizes the FluidSynth library.
     */
    private external fun fluidsynthFree()
    /*
     * @brief   Import of the native implementation of SynthManager.fluidsynthNoteOn() method.
     * @details Plays the note.
     * @param   note      The note to be played.
     * @param   velocity  The velocity of the note to be played.
     */
    private external fun fluidsynthNoteOn(note: Int, velocity: Int)
    /*
     * @brief   Import of the native implementation of SynthManager.fluidsynthNoteOff() method.
     * @details Stops the playing note.
     * @param   note The note to be stopped.
     */
    private external fun fluidsynthNoteOff(note: Int)
    /*
     * @brief   Import of the native implementation of SynthManager.fluidsynthCC() method.
     * @details Sends a control command via MIDI.
     * @param   controller Number of the controller.
     * @param   value      Value to send.
     */
    private external fun fluidsynthCC(controller: Int, value: Int)
    /*
     * @brief   Import of the native implementation of SynthManager.fluidsynthReverb() method.
     * @details Sets the reverb level.
     * @param   level The reverb level (0 to 127).
     */
    private external fun fluidsynthReverb(level: Int)
}