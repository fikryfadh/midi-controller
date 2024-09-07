#include <UHS2-MIDI.h>

// MIDI and USB setup
USB Usb;
UHS2MIDI_CREATE_DEFAULT_INSTANCE(&Usb);

// Button and LED pin definitions
const int buttonPin1 = 37;  // Button connected to GPIO 13
const int buttonPin2 = 39;  // Button connected to GPIO 12
const int ledPin = 15;      // Built-in LED

void setup() {
  Serial.begin(115200);  // Start serial communication

  // Setup buttons and LED
  pinMode(buttonPin1, INPUT_PULLUP);  // Button input with pull-up resistor
  pinMode(buttonPin2, INPUT_PULLUP);  // Button input with pull-up resistor
  pinMode(ledPin, OUTPUT);            // LED as output
  digitalWrite(ledPin, LOW);          // Turn off LED initially

  // Initialize MIDI
  MIDI.begin(1);    

  // Initialize USB and MIDI
  if (Usb.Init() == -1) {
    Serial.println(F("USB Initialization failed"));
    while (1);  // Halt if USB initialization fails
  }
  delay(200);
}

void loop() {
  Usb.Task();  // Process USB tasks

  // Handle Button 1 (1A - Bank)
  if (digitalRead(buttonPin1) == LOW) {  // Button pressed
    digitalWrite(ledPin, HIGH);          // Turn on LED

    // Send a MIDI Control Change message
    byte channel = 1;      // MIDI Channel 1
    byte controller = 60;  // Controller number 60
    byte value = 0;        // Value for the controller

    MIDI.sendControlChange(controller, value, channel);
    Serial.println("1A ");  // Print message to Serial Monitor

    delay(500);             // Debounce the button and delay the next message
    digitalWrite(ledPin, LOW);  // Turn off the LED
  }

  // Handle Button 2 (1B - Bank)
  else if (digitalRead(buttonPin2) == LOW) {  // Button pressed
    digitalWrite(ledPin, HIGH);               // Turn on LED

    // Send a MIDI Control Change message
    byte channel = 1;      // MIDI Channel 1
    byte controller = 60;  // Controller number 60
    byte value = 1;        // Value for the controller

    MIDI.sendControlChange(controller, value, channel);
    Serial.println("1B");  // Print message to Serial Monitor

    delay(500);             // Debounce the button and delay the next message
    digitalWrite(ledPin, LOW);  // Turn off the LED
  }
}
