// Note here that the pin needs to be defined that is connected to the IR Receiver Signal
const int irReceiverPin = 23; 

volatile int objectCount = 0;
volatile bool objectDetected = false;

unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 200; // this is for changing the delays - 200ms 


// This function runs automatically when the sensor triggers
void IRAM_ATTR handleBeamBreak() {
  unsigned long currentTime = millis();
  
  // Check if enough time has passed since the last count (Debouncing)
  if ((currentTime - lastDebounceTime) > debounceDelay) {
    objectCount++;
    objectDetected = true; 
    lastDebounceTime = currentTime;
  }
}

void setup() {
  // Start Serial Monitor to view the count
  Serial.begin(115200);
  
  // Set the IR pin as Input. 
  pinMode(irReceiverPin, INPUT_PULLUP);

  // Attach the interrupt
  // RISING: Triggers when signal goes LOW to HIGH (Beam Broken)
  // FALLING: Triggers when signal goes HIGH to LOW
  attachInterrupt(digitalPinToInterrupt(irReceiverPin), handleBeamBreak, RISING);

  Serial.println("System Ready. Waiting for objects...");
}

void loop() {
  if (objectDetected) {
    Serial.print("Beam Broken! Total Count: ");
    Serial.println(objectCount);
    
    // Reset the flag
    objectDetected = false;
  }
}