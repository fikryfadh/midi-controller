
#include <Arduino.h>
#include <Adafruit_TinyUSB.h>
#include <MIDI.h>

// USB MIDI object
Adafruit_USBD_MIDI usb_midi;

// Create a new instance of the Arduino MIDI Library,
// and attach usb_midi as the transport.
MIDI_CREATE_INSTANCE(Adafruit_USBD_MIDI, usb_midi, MIDI);

// Button and LED pin definitions
const int button1Pin = 37;   // Button 1 on GPIO 37
const int button2Pin = 39;   // Button 2 on GPIO 39
const int ledPin = 15;       // Internal LED on GPIO 15

void setup() {
  // Manual begin() is required on core without built-in support e.g. mbed rp2040
  if (!TinyUSBDevice.isInitialized()) {
    TinyUSBDevice.begin(0);
  }

  pinMode(ledPin, OUTPUT);
  pinMode(button1Pin, INPUT_PULLUP);  // Use internal pull-up resistors
  pinMode(button2Pin, INPUT_PULLUP);  // Use internal pull-up resistors

  usb_midi.setStringDescriptor("TinyUSB MIDI");

  // Initialize MIDI, and listen to all MIDI channels
  MIDI.begin(MIDI_CHANNEL_OMNI);

  Serial.begin(115200);
}

void loop() {
  #ifdef TINYUSB_NEED_POLLING_TASK
  // Manual call tud_task since it isn't called by Core's background
  TinyUSBDevice.task();
  #endif

  // Not enumerated/mounted yet: nothing to do
  if (!TinyUSBDevice.mounted()) {
    return;
  }

  // Check if Button 1 is pressed
  if (digitalRead(button1Pin) == LOW) {
    MIDI.sendControlChange(60, 0, 1);  // Send Control Change on channel 1, CC 60, value 0
    digitalWrite(ledPin, HIGH);        // Turn on LED
    delay(500);                        // Debounce delay
  }

  // Check if Button 2 is pressed
  if (digitalRead(button2Pin) == LOW) {
    MIDI.sendControlChange(60, 1, 1);  // Send Control Change on channel 1, CC 60, value 1
    digitalWrite(ledPin, HIGH);        // Turn on LED
    delay(500);                        // Debounce delay
  }

  // Turn off the LED when no buttons are pressed
  if (digitalRead(button1Pin) == HIGH && digitalRead(button2Pin) == HIGH) {
    digitalWrite(ledPin, LOW);         // Turn off LED
  }

  // Read any new MIDI messages
  MIDI.read();
}
