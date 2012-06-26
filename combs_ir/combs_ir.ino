// This sketch has been adapted from the tutorial at http://www.ladyada.net/learn/sensors/ir.html
// It pulses out a sequence of IR signals to turn on the TV, hdmi switcher, and select the right source
// The main goal is to automate the powering on of all devices once AppleTV turns on,
// so music can start streaming through airplay with no further intervention
// this code is public domain, please enjoy!


int lgTvDiscreteOn[17*3] = {1, 8918, 4472,
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

int lgTvDiscreteHDMI1[17*3] =  {1, 8918, 4472,
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

int lgTvDiscreteHDMI2[17*3] =  {1, 8918, 4472,
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


int IRledPin = 4;
int statusLedPin = 12;
int button1Pin = 8;
int button2Pin = 13;

int photocellPin = 0;
int photocellReading;     
int appletvState = 0;
int mightBeOn = 0;
int mightBeOff = 0;

// set these to customize sensitivity
int appletvOnThresh = 200;
int appletvOffThresh = 10;
int numThreshChecksNeeded = 5;

void setup() {                
    pinMode(IRledPin, OUTPUT);
    pinMode(statusLedPin, OUTPUT);
    pinMode(button1Pin, INPUT);
    pinMode(button2Pin, INPUT);
    //Serial.begin(9600);
}

void loop() {
    photocellReading = analogRead(photocellPin);
    if (appletvState == 0) {
        if (photocellReading > appletvOnThresh) {
            mightBeOn++;
        } else if (mightBeOn > 0) {
            mightBeOn--;
        }
        if (mightBeOn == numThreshChecksNeeded) {
            mightBeOn = 0;
            appletvState = 1;
            sendIRSignal(lgTvDiscreteOn, sizeof(lgTvDiscreteOn)/sizeof(int));
            delay(100);
            sendIRSignal(lgTvDiscreteHDMI1, sizeof(lgTvDiscreteHDMI1)/sizeof(int));
            delay(100);
        }
    }
    
    if (appletvState == 1) {
        if (photocellReading < appletvOffThresh) {
            mightBeOff++;
        } else if (mightBeOff > 0) {
            mightBeOff--;
        }
        if (mightBeOff == numThreshChecksNeeded) {
            mightBeOff = 0;
            appletvState = 0;
            sendIRSignal(lgTvDiscreteHDMI2, sizeof(lgTvDiscreteHDMI2)/sizeof(int));
            delay(500);
        }
    }

//  if (digitalRead(button1Pin)) {
//  }
//  if (digitalRead(button2Pin)) {
//  }
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
