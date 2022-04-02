const char addr[] = {22, 24, 26, 28, 30, 32, 34, 36, 38, 40, 42, 44, 46, 48, 50, 52};
const char data[] = {39, 41, 43, 45, 47, 49, 51, 53};

bool record = false;

#define CLOCK 2
#define RW 3

void setup() {
    // put your setup code here, to run once:
    for(int n = 0; n < 16; n++) {
        pinMode(addr[n], INPUT);
    }
    for(int n = 0; n < 8; n++) {
        pinMode(data[n], INPUT);
    }
    pinMode(CLOCK, INPUT);
    pinMode(RW, INPUT);

    attachInterrupt(digitalPinToInterrupt(CLOCK), onClock, RISING);
    Serial.begin(57600);
}

void onClock() {
    char output[15];
    unsigned address = 0;
    for(int n = 0; n < 16; n++) {
        unsigned bit = digitalRead(addr[n]) ? 1 : 0;
        address = (address << 1) + bit;
    }

    unsigned datta = 0;
    for(int n = 0; n < 8; n++) {
        unsigned bit = digitalRead(data[n]) ? 1 : 0;
        datta = (datta << 1) + bit;
        if(datta == 0xd8) { //cld which is the start of every program
            record = true;
        } else if(datta == 0xdb) {  //stp which is the end of every program
            record = false;
        }
    }

    sprintf(output, "%04x  %c  %02x", address, digitalRead(RW) ? 'R' : 'W', datta);
    if(record) {
        Serial.println(output);
    }
}

void loop() {
    // put your main code here, to run repeatedly:
}