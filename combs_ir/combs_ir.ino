// This sketch has been adapted from the tutorial at http://www.ladyada.net/learn/sensors/ir.html
// It pulses out a sequence of IR signals to turn on the TV, hdmi switcher, and select the right source
// The main goal is to automate the powering on of all devices once AppleTV turns on,
// so music can start streaming through airplay with no further intervention
// this code is public domain, please enjoy!


const int lgTvDiscreteOn[17*3] = {1, 8918, 4472,
                                  2, 546, 572,
                                  1, 546, 1690,
                                  5, 546, 572,
                                  2, 546, 1690,
                                  1, 546, 572,
                                  5, 546, 1690,
                                  2, 546, 572,
                                  1, 546, 1690,
                                  3, 546, 572,
                                  4, 546, 1690,
                                  1, 546, 572,
                                  3, 546, 1690,
                                  2, 546, 572,
                                  1, 546, 39260,
                                  1, 8820, 2200,
                                  1, 546, 28620};

const int lgTvDiscreteHDMI1[17*3] =  {1, 8918, 4472,
                                      2, 546, 572,
                                      1, 546, 1690,
                                      5, 546, 572,
                                      2, 546, 1690,
                                      1, 546, 572,
                                      5, 546, 1690,
                                      1, 546, 572,
                                      3, 546, 1690,
                                      2, 546, 572,
                                      3, 546, 1690,
                                      3, 546, 572,
                                      2, 546, 1690,
                                      2, 546, 572,
                                      1, 546, 39260,
                                      1, 8820, 2200,
                                      1, 546, 28620};

const int lgTvDiscreteHDMI2[17*3] =  {1, 8918, 4472,
                                      2, 546, 572,
                                      1, 546, 1690,
                                      5, 546, 572,
                                      2, 546, 1690,
                                      1, 546, 572,
                                      5, 546, 1690,
                                      2, 546, 572,
                                      2, 546, 1690,
                                      2, 546, 572,
                                      4, 546, 1690,
                                      2, 546, 572,
                                      2, 546, 1690,
                                      2, 546, 572,
                                      1, 546, 39260,
                                      1, 8820, 2200,
                                      1, 546, 28620};

const int lgTvDiscreteOff[21*3] = {1, 8918, 4446,
                                   2, 572, 546,
                                   1, 572, 1638,
                                   5, 572, 546,
                                   2, 572, 1638,
                                   1, 572, 546,
                                   6, 572, 1638,
                                   1, 572, 546,
                                   1, 572, 1638,
                                   3, 572, 546,
                                   2, 572, 1638,
                                   1, 572, 546,
                                   1, 572, 1638,
                                   1, 572, 546,
                                   3, 572, 1638,
                                   2, 572, 546,
                                   1, 572, 39598,
                                   1, 8892, 2210,
                                   1, 2210, 572,
                                   1, 8892, 2210,
                                   1, 2210, 572};


const bool debugMode = true;

const int IRledPin = 4;
const int statusLedPin = 12;
const int num_resends = 4;

const int photocellPin = 0;
int photocellReading;     
int appletvState = 0;
int mightBeOn = 0;
int mightBeOff = 0;

// set these to customize sensitivity
const int appletvOnThresh = 200;
const int appletvOffThresh = 10;
const int numOnThreshChecksNeeded = 1000;
const int numOffThreshChecksNeeded = 5000;

// button stuff
const int blackButtonPin = 8;
const int redButtonPin = 13;

int blackButtonState = 0;
int redButtonState = 0;

void setup() {                
    pinMode(IRledPin, OUTPUT);
    pinMode(statusLedPin, OUTPUT);
    pinMode(blackButtonPin, INPUT);
    pinMode(redButtonPin, INPUT);
    if (debugMode) {
        Serial.begin(9600);
    }
}

void loop() {
    //testIRSignal(lgTvDiscreteOn, sizeof(lgTvDiscreteOn)/sizeof(int), "lgTvDiscreteOn");

    // poll stuff    
    photocellReading = analogRead(photocellPin);
    redButtonState = digitalRead(redButtonPin);
    blackButtonState = digitalRead(blackButtonPin);

    if (appletvState == 0) {
        if (photocellReading > appletvOnThresh) {
            mightBeOn++;
        } else if (mightBeOn > 0) {
            mightBeOn--;
        }
        if (mightBeOn == numOnThreshChecksNeeded) {
            mightBeOn = 0;
            appletvState = 1;
            sendAppleTVAllOnSequence();
        }
    }
    
    if (appletvState == 1) {
        if (photocellReading < appletvOffThresh) {
            mightBeOff++;
        } else if (mightBeOff > 0) {
            mightBeOff--;
        }
        if (mightBeOff == numOffThreshChecksNeeded) {
            mightBeOff = 0;
            appletvState = 0;
            sendAppleTVAllOffSequence();
        }
    }
  
    if (blackButtonState == HIGH) {
        sendAppleTVAllOffSequence();
    } else if (redButtonState == HIGH) {
        sendAppleTVAllOnSequence();
    }
}

// This procedure sends a 38KHz pulse to the IRledPin 
// for a certain # of microseconds. We'll use this whenever we need to send codes
void pulseIR(long microsecs) {
    // we'll count down from the number of microseconds we are told to wait
    cli();  // this turns off any background interrupts
    while (microsecs > 0) {
        // 38 kHz is about 13 microseconds high and 13 microseconds low
        digitalWrite(IRledPin, HIGH);  // this takes about 3 microseconds to happen
        delayMicroseconds(10);         // hang out for 10 microseconds
        digitalWrite(IRledPin, LOW);   // this also takes about 3 microseconds
        delayMicroseconds(10);         // hang out for 10 microseconds
        // so 26 microseconds altogether
        microsecs -= 26;
    }
    sei();  // this turns them back on
}

void testIRSignal(int *arr, int size, char* name) {
    Serial.println("Testing ");
    Serial.println(name);
    sendIRSignal(arr, size);
    delay(1000);
}

void sendIRSignal(int *arr, int size) {
    int i, j;
    digitalWrite(statusLedPin, HIGH);
    for(i = 0; i < size/3; i++) {
        for(j = 0; j < arr[i*3]; j++) {
            //printf("doing pulse %i , delay %i\n", arr[(i*3)+1], arr[(i*3)+2]);
            pulseIR(arr[(i*3)+1]);
            delayMicroseconds(arr[(i*3)+2]);
        }
    }
    digitalWrite(statusLedPin, LOW);
}

void sendAppleTVAllOnSequence() {
    if (debugMode) {
        Serial.print("Sending Apple TV All On Sequence...");
    }
    for(int i = 0; i < num_resends; i++) {
        sendIRSignal(lgTvDiscreteOn, sizeof(lgTvDiscreteOn)/sizeof(int));
        delay(100);
        sendIRSignal(lgTvDiscreteHDMI2, sizeof(lgTvDiscreteHDMI2)/sizeof(int));
        delay(100);
        sendHKDiscreteOn();
        delay(100);
        sendHKDiscreteVid1();
        delay(500);
    }
    // wait for tv to turn on if it was off 
    delay(10000);
    for(int i = 0; i < num_resends; i++) {
        sendIRSignal(lgTvDiscreteOn, sizeof(lgTvDiscreteOn)/sizeof(int));
        delay(100);
        sendIRSignal(lgTvDiscreteHDMI2, sizeof(lgTvDiscreteHDMI2)/sizeof(int));
        delay(100);
        sendHKDiscreteOn();
        delay(100);
        sendHKDiscreteVid1();
        delay(500);
    }
    if (debugMode) {
        Serial.println("Done");
    }
}

void sendAppleTVAllOffSequence() {
    if (debugMode) {
        Serial.print("Sending Apple TV All Off Sequence...");
    }
    for(int i = 0; i < num_resends; i++) {
        sendIRSignal(lgTvDiscreteHDMI1, sizeof(lgTvDiscreteHDMI1)/sizeof(int));
        delay(100);
    }
    if (debugMode) {
        Serial.println("Done");
    }
}

// Below are codes which were grabbed from IR Sensor
// TODO make the grabber ouput in more compressed standard format as above
void sendHKDiscreteOn() {
    digitalWrite(statusLedPin, HIGH);
    pulseIR(8820);
    delayMicroseconds(4460);
    pulseIR(520);
    delayMicroseconds(560);
    pulseIR(540);
    delayMicroseconds(1680);
    pulseIR(540);
    delayMicroseconds(1660);
    pulseIR(540);
    delayMicroseconds(580);
    pulseIR(520);
    delayMicroseconds(580);
    pulseIR(520);
    delayMicroseconds(580);
    pulseIR(520);
    delayMicroseconds(580);
    pulseIR(540);
    delayMicroseconds(1680);
    pulseIR(520);
    delayMicroseconds(580);
    pulseIR(520);
    delayMicroseconds(1680);
    pulseIR(520);
    delayMicroseconds(1680);
    pulseIR(520);
    delayMicroseconds(580);
    pulseIR(540);
    delayMicroseconds(1680);
    pulseIR(500);
    delayMicroseconds(1700);
    pulseIR(520);
    delayMicroseconds(1680);
    pulseIR(520);
    delayMicroseconds(580);
    pulseIR(520);
    delayMicroseconds(580);
    pulseIR(520);
    delayMicroseconds(580);
    pulseIR(540);
    delayMicroseconds(580);
    pulseIR(520);
    delayMicroseconds(580);
    pulseIR(520);
    delayMicroseconds(580);
    pulseIR(520);
    delayMicroseconds(580);
    pulseIR(520);
    delayMicroseconds(1700);
    pulseIR(520);
    delayMicroseconds(580);
    pulseIR(520);
    delayMicroseconds(1680);
    pulseIR(520);
    delayMicroseconds(1680);
    pulseIR(520);
    delayMicroseconds(1680);
    pulseIR(520);
    delayMicroseconds(1680);
    pulseIR(520);
    delayMicroseconds(1680);
    pulseIR(520);
    delayMicroseconds(1680);
    pulseIR(540);
    delayMicroseconds(580);
    pulseIR(520);
    delayMicroseconds(1680);
    pulseIR(520);
    delayMicroseconds(38940);
    pulseIR(8840);
    delayMicroseconds(2200);
    pulseIR(520);
    delayMicroseconds(28704);
    pulseIR(8860);
    delayMicroseconds(2200);
    pulseIR(520);
    delayMicroseconds(25116);
    pulseIR(8860);
    delayMicroseconds(4420);
    pulseIR(520);
    delayMicroseconds(580);
    pulseIR(520);
    delayMicroseconds(580);
    pulseIR(540);
    delayMicroseconds(1680);
    pulseIR(520);
    delayMicroseconds(580);
    pulseIR(520);
    delayMicroseconds(580);
    pulseIR(520);
    delayMicroseconds(580);
    pulseIR(540);
    delayMicroseconds(560);
    pulseIR(540);
    delayMicroseconds(1680);
    pulseIR(520);
    delayMicroseconds(580);
    pulseIR(520);
    delayMicroseconds(580);
    pulseIR(520);
    delayMicroseconds(1680);
    pulseIR(520);
    delayMicroseconds(580);
    pulseIR(540);
    delayMicroseconds(1680);
    pulseIR(520);
    delayMicroseconds(1680);
    pulseIR(520);
    delayMicroseconds(1680);
    pulseIR(520);
    delayMicroseconds(580);
    pulseIR(520);
    delayMicroseconds(580);
    pulseIR(520);
    delayMicroseconds(600);
    pulseIR(520);
    delayMicroseconds(580);
    pulseIR(520);
    delayMicroseconds(580);
    pulseIR(520);
    delayMicroseconds(1680);
    pulseIR(520);
    delayMicroseconds(1680);
    pulseIR(520);
    delayMicroseconds(1680);
    pulseIR(520);
    delayMicroseconds(600);
    pulseIR(520);
    delayMicroseconds(1680);
    pulseIR(520);
    delayMicroseconds(1680);
    pulseIR(520);
    delayMicroseconds(1680);
    pulseIR(520);
    delayMicroseconds(1680);
    pulseIR(520);
    delayMicroseconds(580);
    pulseIR(520);
    delayMicroseconds(600);
    pulseIR(520);
    delayMicroseconds(580);
    pulseIR(520);
    delayMicroseconds(1680);
    pulseIR(520);
    delayMicroseconds(41160);
    pulseIR(8820);
    delayMicroseconds(2200);
    pulseIR(520);
    delayMicroseconds(28724);
    pulseIR(8840);
    delayMicroseconds(2200);
    digitalWrite(statusLedPin, LOW);
}

void sendHKDiscreteOff() {
    digitalWrite(statusLedPin, HIGH);
    pulseIR(8860);
    delayMicroseconds(4440);
    pulseIR(520);
    delayMicroseconds(560);
    pulseIR(540);
    delayMicroseconds(1660);
    pulseIR(560);
    delayMicroseconds(1660);
    pulseIR(520);
    delayMicroseconds(560);
    pulseIR(560);
    delayMicroseconds(580);
    pulseIR(520);
    delayMicroseconds(560);
    pulseIR(540);
    delayMicroseconds(580);
    pulseIR(540);
    delayMicroseconds(1660);
    pulseIR(540);
    delayMicroseconds(580);
    pulseIR(520);
    delayMicroseconds(1680);
    pulseIR(520);
    delayMicroseconds(1680);
    pulseIR(520);
    delayMicroseconds(580);
    pulseIR(520);
    delayMicroseconds(1680);
    pulseIR(540);
    delayMicroseconds(1640);
    pulseIR(560);
    delayMicroseconds(1660);
    pulseIR(540);
    delayMicroseconds(580);
    pulseIR(520);
    delayMicroseconds(1660);
    pulseIR(540);
    delayMicroseconds(580);
    pulseIR(520);
    delayMicroseconds(600);
    pulseIR(520);
    delayMicroseconds(580);
    pulseIR(520);
    delayMicroseconds(580);
    pulseIR(520);
    delayMicroseconds(560);
    pulseIR(540);
    delayMicroseconds(1680);
    pulseIR(540);
    delayMicroseconds(540);
    pulseIR(560);
    delayMicroseconds(580);
    pulseIR(520);
    delayMicroseconds(1680);
    pulseIR(520);
    delayMicroseconds(1680);
    pulseIR(520);
    delayMicroseconds(1680);
    pulseIR(520);
    delayMicroseconds(1680);
    pulseIR(540);
    delayMicroseconds(1660);
    pulseIR(520);
    delayMicroseconds(600);
    pulseIR(520);
    delayMicroseconds(1680);
    pulseIR(520);
    delayMicroseconds(38940);
    pulseIR(8840);
    delayMicroseconds(2180);
    pulseIR(540);
    delayMicroseconds(28724);
    pulseIR(8840);
    delayMicroseconds(2180);
    pulseIR(560);
    delayMicroseconds(25136);
    pulseIR(8840);
    delayMicroseconds(4400);
    pulseIR(540);
    delayMicroseconds(560);
    pulseIR(560);
    delayMicroseconds(540);
    pulseIR(560);
    delayMicroseconds(1640);
    pulseIR(560);
    delayMicroseconds(560);
    pulseIR(540);
    delayMicroseconds(560);
    pulseIR(540);
    delayMicroseconds(560);
    pulseIR(560);
    delayMicroseconds(560);
    pulseIR(540);
    delayMicroseconds(1660);
    pulseIR(540);
    delayMicroseconds(560);
    pulseIR(540);
    delayMicroseconds(560);
    pulseIR(560);
    delayMicroseconds(1640);
    pulseIR(560);
    delayMicroseconds(560);
    pulseIR(540);
    delayMicroseconds(1660);
    pulseIR(540);
    delayMicroseconds(1660);
    pulseIR(540);
    delayMicroseconds(1660);
    pulseIR(540);
    delayMicroseconds(580);
    pulseIR(540);
    delayMicroseconds(1660);
    pulseIR(540);
    delayMicroseconds(580);
    pulseIR(520);
    delayMicroseconds(580);
    pulseIR(520);
    delayMicroseconds(580);
    pulseIR(520);
    delayMicroseconds(1660);
    pulseIR(560);
    delayMicroseconds(1660);
    pulseIR(540);
    delayMicroseconds(1640);
    pulseIR(560);
    delayMicroseconds(560);
    pulseIR(540);
    delayMicroseconds(560);
    pulseIR(540);
    delayMicroseconds(1660);
    pulseIR(540);
    delayMicroseconds(1660);
    pulseIR(560);
    delayMicroseconds(1640);
    pulseIR(560);
    delayMicroseconds(580);
    pulseIR(520);
    delayMicroseconds(560);
    pulseIR(540);
    delayMicroseconds(560);
    pulseIR(560);
    delayMicroseconds(1660);
    pulseIR(520);
    delayMicroseconds(41140);
    pulseIR(8840);
    delayMicroseconds(2180);
    pulseIR(560);
    delayMicroseconds(28704);
    pulseIR(8840);
    delayMicroseconds(2180);
    digitalWrite(statusLedPin, LOW);
}

void sendHKDiscreteVid1() {
    digitalWrite(statusLedPin, HIGH);
    pulseIR(120);
    delayMicroseconds(54928);
    pulseIR(8840);
    delayMicroseconds(4420);
    pulseIR(540);
    delayMicroseconds(580);
    pulseIR(520);
    delayMicroseconds(1680);
    pulseIR(520);
    delayMicroseconds(1680);
    pulseIR(520);
    delayMicroseconds(580);
    pulseIR(520);
    delayMicroseconds(580);
    pulseIR(540);
    delayMicroseconds(580);
    pulseIR(520);
    delayMicroseconds(580);
    pulseIR(520);
    delayMicroseconds(1680);
    pulseIR(520);
    delayMicroseconds(580);
    pulseIR(540);
    delayMicroseconds(1660);
    pulseIR(520);
    delayMicroseconds(1680);
    pulseIR(540);
    delayMicroseconds(580);
    pulseIR(520);
    delayMicroseconds(1680);
    pulseIR(520);
    delayMicroseconds(1680);
    pulseIR(520);
    delayMicroseconds(1680);
    pulseIR(520);
    delayMicroseconds(580);
    pulseIR(520);
    delayMicroseconds(1700);
    pulseIR(520);
    delayMicroseconds(1680);
    pulseIR(520);
    delayMicroseconds(580);
    pulseIR(520);
    delayMicroseconds(1680);
    pulseIR(520);
    delayMicroseconds(580);
    pulseIR(540);
    delayMicroseconds(560);
    pulseIR(540);
    delayMicroseconds(1680);
    pulseIR(520);
    delayMicroseconds(580);
    pulseIR(520);
    delayMicroseconds(580);
    pulseIR(520);
    delayMicroseconds(580);
    pulseIR(520);
    delayMicroseconds(1680);
    pulseIR(540);
    delayMicroseconds(580);
    pulseIR(520);
    delayMicroseconds(1680);
    pulseIR(520);
    delayMicroseconds(1680);
    pulseIR(520);
    delayMicroseconds(580);
    pulseIR(520);
    delayMicroseconds(1680);
    pulseIR(520);
    delayMicroseconds(38960);
    pulseIR(8820);
    delayMicroseconds(2200);
    pulseIR(540);
    delayMicroseconds(28724);
    pulseIR(8840);
    delayMicroseconds(2200);
    pulseIR(520);
    delayMicroseconds(25216);
    pulseIR(8840);
    delayMicroseconds(4420);
    pulseIR(520);
    delayMicroseconds(580);
    pulseIR(520);
    delayMicroseconds(580);
    pulseIR(540);
    delayMicroseconds(1660);
    pulseIR(540);
    delayMicroseconds(580);
    pulseIR(520);
    delayMicroseconds(580);
    pulseIR(520);
    delayMicroseconds(580);
    pulseIR(520);
    delayMicroseconds(600);
    pulseIR(520);
    delayMicroseconds(1660);
    pulseIR(540);
    delayMicroseconds(580);
    pulseIR(520);
    delayMicroseconds(580);
    pulseIR(520);
    delayMicroseconds(1680);
    pulseIR(520);
    delayMicroseconds(600);
    pulseIR(520);
    delayMicroseconds(1680);
    pulseIR(520);
    delayMicroseconds(1680);
    pulseIR(520);
    delayMicroseconds(1680);
    pulseIR(520);
    delayMicroseconds(580);
    pulseIR(520);
    delayMicroseconds(580);
    pulseIR(540);
    delayMicroseconds(580);
    pulseIR(520);
    delayMicroseconds(580);
    pulseIR(520);
    delayMicroseconds(580);
    pulseIR(520);
    delayMicroseconds(1680);
    pulseIR(520);
    delayMicroseconds(1680);
    pulseIR(540);
    delayMicroseconds(1680);
    pulseIR(520);
    delayMicroseconds(580);
    pulseIR(520);
    delayMicroseconds(1680);
    pulseIR(520);
    delayMicroseconds(1680);
    pulseIR(520);
    delayMicroseconds(1680);
    pulseIR(520);
    delayMicroseconds(1680);
    pulseIR(520);
    delayMicroseconds(580);
    pulseIR(540);
    delayMicroseconds(580);
    pulseIR(520);
    delayMicroseconds(580);
    pulseIR(520);
    delayMicroseconds(1680);
    pulseIR(520);
    delayMicroseconds(41160);
    pulseIR(8820);
    delayMicroseconds(2200);
    pulseIR(520);
    delayMicroseconds(28724);
    pulseIR(8840);
    delayMicroseconds(2200);
    digitalWrite(statusLedPin, LOW);
}
