#include <UHS2-MIDI.h>         // Include the USB Host MIDI library
#include <Adafruit_TinyUSB.h>  // Include the Adafruit TinyUSB library

// USB Host and Device objects
Adafruit_USBD_MIDI usb_midi;  // USB Device MIDI (TinyUSB)
MIDI_CREATE_INSTANCE(Adafruit_USBD_MIDI, usb_midi, MIDI);

USB Usb;  // USB Host (MAX3421E)
UHS2MIDI_NAMESPACE::uhs2MidiTransport uhs2MIDI2(&Usb, 0);
MIDI_NAMESPACE::MidiInterface<UHS2MIDI_NAMESPACE::uhs2MidiTransport> MIDI2((UHS2MIDI_NAMESPACE::uhs2MidiTransport&)uhs2MIDI2);

// Button and LED pin definitions
const int buttonPin1 = 37;  // Button 1 connected to GPIO 37
const int buttonPin2 = 39;  // Button 2 connected to GPIO 39
const int buttonPin3 = 18;  // Button 3 connected to GPIO 18 (Mode switch)
const int ledPin = 15;      // Built-in LED

enum Mode { BOTH, DEVICE, HOST };  // Modes enumeration
Mode currentMode = BOTH;           // Default mode

unsigned long button3PressTime = 0;   // To handle long press detection
const unsigned long longPressDuration = 2000;  // Long press duration (2 seconds)

void setup() {
  Serial.begin(115200);

  // Setup buttons and LED
  pinMode(buttonPin1, INPUT_PULLUP);  // Button input with pull-up resistor
  pinMode(buttonPin2, INPUT_PULLUP);  // Button input with pull-up resistor
  pinMode(buttonPin3, INPUT_PULLUP);  // Button input with pull-up resistor for mode switching
  pinMode(ledPin, OUTPUT);            // LED as output
  digitalWrite(ledPin, LOW);          // Turn off LED initially

  if (!TinyUSBDevice.isInitialized()) {
    TinyUSBDevice.begin(0);
  }
  
  usb_midi.setStringDescriptor("TinyUSB MIDI");

  // Initialize MIDI, and listen to all MIDI channels
  MIDI.begin(MIDI_CHANNEL_OMNI);
  
  // Initialize USB Host MIDI
  if (Usb.Init() == -1) {
    Serial.println(F("USB Host Initialization failed"));
    while (1);  // Halt if USB initialization fails
  } else {
    MIDI2.begin(1);  // Initialize USB Host MIDI
    Serial.println(F("USB Host MIDI initialized"));
  }
}

void loop() {
  handleMIDI();          // Handle MIDI tasks for both Device and Host based on the mode
  handleModeSwitch();    // Handle mode switching with button 3
}

// Function to handle MIDI based on the current mode
void handleMIDI() {
  #ifdef TINYUSB_NEED_POLLING_TASK
  TinyUSBDevice.task();  // Manual call if needed
  #endif

  // Only check for button presses if no button is currently processed
  static bool button1Handled = false;
  static bool button2Handled = false;

  // Read button states
  bool button1Pressed = digitalRead(buttonPin1) == LOW;
  bool button2Pressed = digitalRead(buttonPin2) == LOW;

  // Handle button 1 press
  if (button1Pressed && !button1Handled) {
    digitalWrite(ledPin, HIGH);  // Turn on LED

    // Send a MIDI Control Change message
    byte channel = 1;      // MIDI Channel 1
    byte controller = 60;  // Controller number 60
    byte value = 0;        // Value for the controller

    sendMIDIMessage(controller, value, channel);
    Serial.println("1A");  // Print message to Serial Monitor

    button1Handled = true;  // Mark button 1 as handled
    delay(100);             // Debounce the button and delay the next message
    digitalWrite(ledPin, LOW);  // Turn off the LED
  } 
  else if (button2Pressed && !button2Handled) {  // Handle button 2 press
    digitalWrite(ledPin, HIGH);  // Turn on LED

    // Send a MIDI Control Change message
    byte channel = 1;      // MIDI Channel 1
    byte controller = 60;  // Controller number 60
    byte value = 1;        // Value for the controller

    sendMIDIMessage(controller, value, channel);
    Serial.println("1B");  // Print message to Serial Monitor

    button2Handled = true;  // Mark button 2 as handled
    delay(100);             // Debounce the button and delay the next message
    digitalWrite(ledPin, LOW);  // Turn off the LED
  } 
  else {
    // Reset button handled flags if both buttons are released
    if (!button1Pressed) button1Handled = false;
    if (!button2Pressed) button2Handled = false;
  }

  // Read MIDI messages if mode includes Device or Both
  if (currentMode == DEVICE || currentMode == BOTH) {
    MIDI.read();
  }

  // Handle USB Host MIDI if mode includes Host or Both
  if (currentMode == HOST || currentMode == BOTH) {
    Usb.Task();  // Process USB tasks for USB Host MIDI
  }
}

// Send a MIDI Control Change message based on the current mode
void sendMIDIMessage(byte controller, byte value, byte channel) {
  if (currentMode == DEVICE || currentMode == BOTH) {
    MIDI.sendControlChange(controller, value, channel);
  }
  if (currentMode == HOST || currentMode == BOTH) {
    MIDI2.sendControlChange(controller, value, channel);
  }
}

// Function to handle mode switching with button 3
void handleModeSwitch() {
  bool button3Pressed = digitalRead(buttonPin3) == LOW;

  if (button3Pressed && button3PressTime == 0) {
    // Start timing when button 3 is pressed
    button3PressTime = millis();
  } 
  else if (!button3Pressed && button3PressTime > 0) {
    // Button 3 released, check the press duration
    unsigned long pressDuration = millis() - button3PressTime;
    button3PressTime = 0;  // Reset press time

    if (pressDuration >= longPressDuration) {
      // Long press detected, reset to BOTH mode
      currentMode = BOTH;
      Serial.println("Mode: BOTH (Reset via long press)");
    } else {
      // Toggle between Device, Host, and Both modes
      if (currentMode == BOTH) {
        currentMode = DEVICE;
      } else if (currentMode == DEVICE) {
        currentMode = HOST;
      } else {
        currentMode = BOTH;
      }
      Serial.print("Mode switched to: ");
      if (currentMode == BOTH) Serial.println("BOTH");
      else if (currentMode == DEVICE) Serial.println("DEVICE");
      else if (currentMode == HOST) Serial.println("HOST");
    }
  }
}