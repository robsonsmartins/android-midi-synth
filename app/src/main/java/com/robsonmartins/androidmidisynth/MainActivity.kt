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
 * @file MainActivity.kt
 * @brief Kotlin Implementation of MainActivity.
 *
 * @author Robson Martins (https://www.robsonmartins.com)
 */
// -----------------------------------------------------------------------------------------------

package com.robsonmartins.androidmidisynth

import android.os.Bundle
import android.view.WindowManager
import android.widget.ScrollView
import android.widget.TextView
import androidx.activity.enableEdgeToEdge
import androidx.activity.viewModels
import androidx.appcompat.app.AppCompatActivity
import androidx.core.view.ViewCompat
import androidx.core.view.WindowInsetsCompat
import androidx.core.widget.addTextChangedListener
import androidx.lifecycle.ViewModel

// -----------------------------------------------------------------------------------------------

/**
 * @brief MainViewModel class.
 * @details The MainViewModel stores the "View Model" of the Main Activity.
 */
class MainViewModel : ViewModel() {
    /** @brief Text content of the main component. */
    var textContent: String = ""
}

// -----------------------------------------------------------------------------------------------

/**
 * @brief MainActivity class.
 * @details The MainActivity encapsulates a main activity of the application.
 */
class MainActivity : AppCompatActivity() {

    companion object {
        /** @brief Initialize: Load the Native Library. */
        init { System.loadLibrary("synth-lib") }
    }

    /* @brief View Model instance. */
    private val viewModel: MainViewModel by viewModels()
    /* @brief SynthManager instance. */
    private lateinit var synthManager: SynthManager
    /* @brief MidiManager instance. */
    private lateinit var midiManager: MidiManager
    /* @brief TextView widget. */
    private lateinit var txtLog: TextView
    /* @brief ScrollView widget. */
    private lateinit var scrollView: ScrollView

    /**
     * @brief On create event.
     * @param savedInstanceState Saved instance state.
     */
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        // screen always on
        window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)
        enableEdgeToEdge()
        setContentView(R.layout.activity_main)
        ViewCompat.setOnApplyWindowInsetsListener(findViewById(R.id.main)) { v, insets ->
            val systemBars = insets.getInsets(WindowInsetsCompat.Type.systemBars())
            v.setPadding(systemBars.left, systemBars.top, systemBars.right, systemBars.bottom)
            insets
        }
        // setup GUI
        scrollView = findViewById(R.id.scrollView)
        txtLog = findViewById(R.id.txtLog)
        txtLog.text = viewModel.textContent
        txtLog.addTextChangedListener {
            viewModel.textContent = it.toString()
            scrollView.post { scrollView.fullScroll(ScrollView.FOCUS_DOWN) }
        }
        // setup SynthManager and MidiManager
        synthManager = SynthManager(this)
        synthManager.loadSF("KawaiStereoGrand.sf3")
        synthManager.setVolume(127)
        midiManager = MidiManager(this, ::onMidiMessageReceived)
        midiManager.start()
    }

    /** @brief On destroy event. */
    override fun onDestroy() {
        midiManager.finalize()
        synthManager.finalize()
        super.onDestroy()
    }

    /*
     * @brief On MIDI message received callback.
     * @param message Message received.
     */
    private fun onMidiMessageReceived(message: String) {
        // messages are received on some other thread, so switch to the UI thread
        // before attempting to access the UI
        runOnUiThread { txtLog.append(message + "\n") }
    }

}