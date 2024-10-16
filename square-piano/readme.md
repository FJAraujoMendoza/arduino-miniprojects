# Square Wave Piano

A basic monophonic square wave piano using an Arduino Mega 2560 or similar.

## ✨Features✨

- 12 piano keys/switches (C to B) to play any song you love! <sup><sub>As long as you don't like chords.</sub></sup>
- Multiple octaves at the twist of a potentiometer.
- Extreme quality sound <sup><sub>...For an early 1980's buzzer</sub></sup>
- No amp or filter required: Just plug your highest quality passive buzzer/speaker and play!
- Serial debugging for checking the state of the piano keys and the currently playing note and octave. <sup><sub>Snazzy!</sub></sup>
- Real-time interrupts and timers for generating square waves, compatible with the `TimerThree` library or equivalent.


## ⚙️ Hardware Requirements

- **Arduino Mega 2560** or compatible board (with at least 13 available digital pins + 1 analog pin + 1 timer)
- 12 push buttons or switches
- 1 potentiometer
- 1 passive buzzer
- Wires and breadboard


## 🔌 Default Pin Configuration

These are the pin bindings set on the sketch file, feel free to modify as needed:
- Piano keys: Digital pins `23`, `25`, `27`, `29`, `31`, `33`, `35`, `37`, `39`, `41`, `43`, `45`
- Octave selector potentiometer: Analog pin `A8`
- Buzzer: Digital pin `13` (for activity LED)


## 📚 Libraries Used

- [TimerThree by Paul Stoffregen](https://github.com/PaulStoffregen/TimerThree), to generate and control the square wave synthesis via interrupts.
- [Pin by Alec Fenichel](https://github.com/fenichelar/Pin), to be able to toggle pins on/off fast enough not to distort the wave (built-in `digitalWrite()` can be painfully slow when you use it thousands of times a second)


## 🪄 Setup
Check the images in the [`./pictures`](./pictures/) folder as a general idea of how it _could_ look like (I'm not a breadboard master and I was short in push buttons 🫠)
1. Connect the piano keys to the specified digital pins.
2. Connect the potentiometer to analog pin A8.
3. Connect the passive buzzer to digital pin 13.
4. Install the required libraries in your IDE of choice.
5. Upload the `square-piano.ino` sketch to your Arduino board.


## 🎹 Usage

- Press the piano keys to play notes (only the highest pressed note will play).
- Use the potentiometer to change the octave.
- Enable or disable the _✨fancy✨_ serial debugging and display by setting the `ENABLE_SERIAL` macro in the sketch.


## 📜 License

This project is licensed under the GPL-3.0 License.


## 🕺🏻 Author

Francisco Javier Araujo [(@FJAraujoMendoza)](https://github.com/FJAraujoMendoza)


## 🔎 References

- [TimerThree library](https://github.com/PaulStoffregen/TimerThree)
- [Arduino Pin library](https://github.com/fenichelar/Pin)