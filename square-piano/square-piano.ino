/**
 * Square Wave Piano: A basic monophonic square wave piano using an Arduino Mega
 * 
 * - 12 piano keys/switches (C to B) connected to digital input pins
 * - 1 potentiometer for selecting the octave connected to an analog pin
 * - 1 passive buzzer connected to a digital output pin
 * - Real-time interrupts and timers for generating square waves, compatible
 *   with the TimerThree library or equivalent.
 * 
 * This sketch uses the TimerThree library by Paul Stoffregen to generate
 * square waves in real-time for playing notes on a buzzer. The notes are
 * played by pressing the piano keys, which are connected to digital pins. The
 * octave can be selected using a potentiometer. It also uses the Arduino Pin
 * library by Alec Fenichel for fast pin I/O operations.
 * 
 * The sketch also includes serial debugging for monitoring the state of the
 * piano keys and the currently playing note, which can be enabled or disabled
 * by changing the ENABLE_SERIAL macro.
 * 
 * The sketch was tested on an Elegoo MEGA2560 R3 board with a passive buzzer.
 * 
 * Author: Francisco Javier Araujo
 * Version: 1.0
 * Date: 2024-10-16
 * License: GPL-3.0
 * See also:
 * - Arduino Pin library: https://github.com/fenichelar/Pin
 * - TimerThree library: https://github.com/PaulStoffregen/TimerThree
 */

#include <Pin.h>
#include <TimerThree.h>

#define ENABLE_SERIAL 1 // Set to 1 to enable serial debugging, 0 to disable

#if ENABLE_SERIAL
#define dbg_serial_begin(...) Serial.begin(__VA_ARGS__);
#define dbg_print(...) Serial.print(__VA_ARGS__);
#define dbg_println(...) Serial.println(__VA_ARGS__);
#else
#define dbg_serial_begin(...)
#define dbg_print(...)
#define dbg_println(...)
#endif

// Definition of the main notes in an octave
#define NOTE_NONE -1
#define NOTE_C 0
#define NOTE_C_SHARP 1
#define NOTE_D 2
#define NOTE_D_SHARP 3
#define NOTE_E 4
#define NOTE_F 5
#define NOTE_F_SHARP 6
#define NOTE_G 7
#define NOTE_G_SHARP 8
#define NOTE_A 9
#define NOTE_A_SHARP 10
#define NOTE_B 11

// Pinout for the piano keys. Change as needed.
// Note that TimerThree might use pins 2, 3 and 5 for the timer, so avoid using
// those pins or switch to TimerOne or equivalent library.
#define PIN_C 23
#define PIN_C_SHARP 25
#define PIN_D 27
#define PIN_D_SHARP 29
#define PIN_E 31
#define PIN_F 33
#define PIN_F_SHARP 35
#define PIN_G 37
#define PIN_G_SHARP 39
#define PIN_A 41
#define PIN_A_SHARP 43
#define PIN_B 45

#define PIN_OCTAVE PIN_A8   // Pin for the octave selector potentiometer
#define PIN_BUZZER 13       // Pin for the buzzer
#define SCAN_TIME_MS 33     // Time between scans of the piano keys/main loop

// Lowest/highest octave accessible, limited by the buzzer specs and wave
// generation/switching speed.
// Tested for Elegoo MEGA2560 R3 - B6 starts to sound bad/too loud
#define BASE_OCTAVE 2
#define MAX_OCTAVE 6

// Note names (for printing/debugging)
String const NOTES[] = {"C", "C#", "D", "D#", "E", "F",
                        "F#", "G", "G#", "A","A#", "B"};

// Global vars for pin access and status
Pin g_buzzer = Pin(PIN_BUZZER);
Pin g_octave = Pin(PIN_OCTAVE);
Pin g_keys[] = {
    Pin(PIN_C),
    Pin(PIN_C_SHARP),
    Pin(PIN_D),
    Pin(PIN_D_SHARP),
    Pin(PIN_E),
    Pin(PIN_F),
    Pin(PIN_F_SHARP),
    Pin(PIN_G),
    Pin(PIN_G_SHARP),
    Pin(PIN_A),
    Pin(PIN_A_SHARP),
    Pin(PIN_B)
};
int8_t volatile g_note_playing = NOTE_NONE;
uint8_t volatile g_octave_playing = NOTE_NONE;

/**
 * Calculate the frequency of a note in Hz.
 * 
 * Uses a modified version of the formula for the equal tempered scale that
 * accommodates for octave addressing:
 * 
 * f = 440 * 2^((n - 9)/12 + o - 4)
 * 
 * where n is the note (NOTE_C to NOTE_B) and o is the octave number.
 * 
 * @param semitone The note (0 to 11 or NOTE_C to NOTE_B)
 * @param octave The octave number
 * @return The frequency in Hz
 */
unsigned inline long note_to_freq(uint8_t semitone, uint8_t octave) {
    return 440 * pow(2, (semitone - 9) / 12.0 + octave - 4);
}

/**
 * Calculate the period of a note in microseconds.
 * 
 * Uses a modified version of the formula for the equal tempered scale that
 * accommodates for octave addressing, simplified to reduce costly arithmetic
 * operations:
 * 
 * T = 25000 / 11 * 2^((n - 9)/12 + o - 4)
 * 
 * where n is the note (NOTE_C to NOTE_B) and o is the octave number.
 * 
 * @param semitone The note (0 to 11 or NOTE_C to NOTE_B)
 * @param octave The octave number
 * @return The period in microseconds
 */
unsigned inline long note_to_period_us(uint8_t semitone, uint8_t octave) {
    return 25000L / (11 * pow(2, ((semitone - 9) + (12*(octave - 4))) / 12.0));
}

/**
 * Get the current selected octave from the potentiometer.
 * 
 * The potentiometer is read and the value is mapped and adjusted to the
 * configured range of available octaves.
 * 
 * @return The current octave number
 */
uint8_t get_current_octave() {
    uint8_t number_of_octaves = MAX_OCTAVE - BASE_OCTAVE + 1;
    uint8_t octave = ((uint16_t) g_octave.getAnalogValue() * number_of_octaves) >> 10;
    return octave + BASE_OCTAVE;
}

/**
 * Check if a valid note is currently playing.
 * 
 * @return True if a note is playing, false otherwise
 */
bool is_playing() {
    return g_note_playing != NOTE_NONE && g_octave_playing != NOTE_NONE;
}

/**
 * Real-time interrupt function for generating square waves and playing notes.
 * 
 * This function is called by the TimerThree library as an interrupt, which must
 * be set to twice the desired note frequency to generate a square wave
 * (first call sets the high part of the wave, second call sets the low part).
 */
void rt_play() {
    if (is_playing())
        g_buzzer.toggleState();
    else
        g_buzzer.setLow();
}

/**
 * Setup function for the Arduino sketch.
 * 
 * Initializes the pins, interrupts and serial communication.
 */
void setup() {
    Timer3.initialize();
    Timer3.attachInterrupt(rt_play);

    g_buzzer.setOutput();

    g_octave.setInput();
    for (uint8_t i = 0; i < 12; i++) {
        g_keys[i].setInputPullupOn();
    }

    dbg_serial_begin(115200);
}

void loop() {
    delay(SCAN_TIME_MS);

    int8_t note = NOTE_NONE;
    uint8_t octave = get_current_octave();

    // Print the current state of the piano keys and find the highest pressed,
    // if any, to play it.
    dbg_print("Octave: ");
    dbg_print(octave);
    dbg_print(" --- ");
    for (uint8_t i = 0; i < 12; i++) {
        if (g_keys[i].getValue() == HIGH) {
            dbg_print("|--|");
        } else {
            note = i;
            dbg_print("|");
            dbg_print(NOTES[i]);
            switch(NOTES[i].length()) {
                case 1:
                    dbg_print(" |");
                    break;
                case 2:
                    dbg_print("|");
                    break;
            }
        }
    }
    dbg_print(" --- Playing: ");
    if (note == NOTE_NONE) {
        dbg_print("N/A\n");
    } else {
        dbg_print(NOTES[note]);
        dbg_print(octave);
        dbg_print("\n");
    }

    // If a key is pressed or changed to a different note, update the playing
    // note and octave.
    if (note != g_note_playing || octave != g_octave_playing) {
        // Disable interrupts temporarily to evade glitches in the square wave
        // generation
        noInterrupts();
        g_note_playing = note;
        g_octave_playing = octave;
        Timer3.setPeriod(note_to_period_us(note, octave)/2);
        interrupts();
    }
}