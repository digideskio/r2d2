#include <PCM.h>
#include "pitches.h"
#include "sounddata-r2d2-excited.h"
#include "sounddata-r2d2-veryexcited.h"
#include "emperor.h"
#include "starwars.h"

#define SPEAKER_PIN 11
#define LED_WHITE 8
#define LED_RED 7
#define LED_BLUE 6

// audio jack
#define AJ_BTN_BLUE 13
#define AJ_BTN_RED 12 

// light modes
#define OFF 0
#define SLOW 1
#define RANDOM 2

// GLOBAL VARIABLES
int BTN_BLUE = LOW;
int BTN_BLUE_PREV = LOW;
unsigned long BTN_BLUE_STARTED = 0; // time since a button press started
bool BTN_BLUE_SHORT_PRESS = false; // true only for one run
bool BTN_BLUE_MEDIUM_PRESS = false; // true only for one run
bool BTN_BLUE_LONG_PRESS = false; // true only for one run

int BTN_RED = LOW;
int BTN_RED_PREV = LOW;
unsigned long BTN_RED_STARTED = 0; // time since a button press started
bool BTN_RED_SHORT_PRESS = false;
bool BTN_RED_MEDIUM_PRESS = false;
bool BTN_RED_LONG_PRESS = false;

// pointer to current sound
int CURRENT_R2D2_SOUND = 0;
int CURRENT_SONG = 0;

// define button press times
unsigned long BTN_MEDIUM_LONG = 500;
unsigned long BTN_LONG = 2000;

int LIGHTMODE = SLOW;

void setup(){
    // DEBUG ONLY - communication with computer
    Serial.begin(9600);

    // sets LED pins for output
    pinMode(LED_WHITE, OUTPUT);
    pinMode(LED_RED, OUTPUT);
    pinMode(LED_BLUE, OUTPUT);

    // Audio-Jack controlled buttons
    pinMode(AJ_BTN_BLUE, INPUT_PULLUP);
    pinMode(AJ_BTN_RED, INPUT_PULLUP);
}

void loop(){
    delay(50);
    read_buttons();    

    //print out the value of the pushbutton
    if (BTN_BLUE_SHORT_PRESS){
        play_next_sound();
    } else if (BTN_BLUE_MEDIUM_PRESS || BTN_BLUE_LONG_PRESS) {
        play_next_song();
    }
    if (BTN_RED_SHORT_PRESS || BTN_RED_MEDIUM_PRESS || BTN_RED_LONG_PRESS){
        set_next_light_mode();
    }

    update_leds(); // once per run
}

void set_next_light_mode(){
    LIGHTMODE = (LIGHTMODE > 1) ? 0 : LIGHTMODE+1;

    if (!LIGHTMODE){
        play_off_sound();
    }
}

void play_next_song() {
    if (CURRENT_SONG == 0){
        play_melody(emperor, sizeof(emperor), emperorDuration, 1800);
    } else {
        play_melody(starwars, sizeof(starwars), starwarsDuration, 1600);
    }
    
    // update song pointer
    CURRENT_SONG = (CURRENT_SONG > 0) ? 0 : CURRENT_SONG+1; 
}

void play_off_sound(){
    tone(SPEAKER_PIN, N_C3, 100);
    delay(100);
    noTone(SPEAKER_PIN);
}

void play_sound(const unsigned char soundData[], int size) {
    // Play a given sound
    startPlayback(soundData, size);
}

void play_next_sound(){
    // Play the next sound in the loop
    if (CURRENT_R2D2_SOUND == 0){
        play_sound(sounddata_veryexcited_data, sizeof(sounddata_veryexcited_data));
    } else {
        play_sound(sounddata_excited_data, sizeof(sounddata_excited_data));
    }

    // update sound pointer
    CURRENT_R2D2_SOUND = (CURRENT_R2D2_SOUND > 0) ? 0 : CURRENT_R2D2_SOUND+1; 
}

void play_melody(int melody[], int melodySize, int noteDurations[], int speed){
    // iterate over the notes of the melody:
    stopPlayback(); // stop any PCM playback
    int BTN_BLUE_TEMP;
    for (int thisNote = 0; thisNote < (melodySize/sizeof(int)); thisNote++) {

        // to calculate the note duration, take one second 
        // divided by the note type.
        //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
        int noteDuration = speed/noteDurations[thisNote];
        tone(SPEAKER_PIN, melody[thisNote], noteDuration);

        // to distinguish the notes, set a minimum time between them.
        // the note's duration + 30% seems to work well:
        int pauseBetweenNotes = noteDuration * 1.30;
        delay(pauseBetweenNotes);
        // stop the tone playing:
        noTone(SPEAKER_PIN);

        // stop playback in case of a blue button press
        BTN_BLUE_TEMP = digitalRead(AJ_BTN_BLUE);
        if(BTN_BLUE_TEMP == LOW) {
            break;
        }
    }
}

void update_leds(){
    // control lights
    switch (LIGHTMODE){
        case OFF:
            update_leds_off();
            break;
        case SLOW:
            update_leds_slow();
            break;
        case RANDOM:
            update_leds_random();
            break;
        default:
            update_leds_slow();
    }
}

void update_leds_off(){
    analogWrite(LED_WHITE, 0);
    analogWrite(LED_RED, 0);
    analogWrite(LED_BLUE, 0);
}

unsigned long LAST_UPDATE_WHITE = 0;
int WHITE_UPDATE_INTERVAL = random(5000);
int BLUE_UPDATE_VALUE = 10;
int BLUE_SUM = 0;
void update_leds_slow(){
    if (millis() - LAST_UPDATE_WHITE > WHITE_UPDATE_INTERVAL){
        analogWrite(LED_WHITE, 127 + random(2));    // 127 - off
        LAST_UPDATE_WHITE = millis();
        WHITE_UPDATE_INTERVAL = random(5000);
    }
    
    analogWrite(LED_RED, 128); // keep if always on      // 127 - off

    // make blue LED glimmer slowly
    analogWrite(LED_BLUE, BLUE_SUM);
    BLUE_SUM = BLUE_SUM + BLUE_UPDATE_VALUE;
    BLUE_UPDATE_VALUE = (BLUE_SUM > 254 || BLUE_SUM < 1) ? BLUE_UPDATE_VALUE * -1 : BLUE_UPDATE_VALUE;
    BLUE_SUM = max(0, min(255, BLUE_SUM));
}

void update_leds_random(){
    analogWrite(LED_WHITE, 100+random(255));
    analogWrite(LED_RED, 100+random(128));
    analogWrite(LED_BLUE, random(128));
}

void read_buttons(){
    // clear old state
    BTN_BLUE_SHORT_PRESS = false;
    BTN_BLUE_MEDIUM_PRESS = false;
    BTN_BLUE_LONG_PRESS = false;
    BTN_RED_SHORT_PRESS = false;
    BTN_RED_MEDIUM_PRESS = false;
    BTN_RED_LONG_PRESS = false;

    // read sensor values - http://arduino.cc/en/Tutorial/InputPullupSerial
    BTN_BLUE = digitalRead(AJ_BTN_BLUE);
    BTN_RED = digitalRead(AJ_BTN_RED);
    // //ardprintf("%d - %d", BTN_BLUE, BTN_RED); // DEBUG ONLY
    unsigned long CURRENT_TIME = millis();

    // PRESS STARTED ===========================================================
    /*BLUE BTN press finished*/
    if (BTN_BLUE_PREV == HIGH && BTN_BLUE == LOW){ 
        BTN_BLUE_STARTED = CURRENT_TIME;
        //ardprintf("BLUE start %l", BTN_BLUE_STARTED); // DEBUG ONLY
    }

    /*RED BTN press finished*/
    if (BTN_RED_PREV == LOW && BTN_RED == HIGH){ 
        BTN_RED_STARTED = CURRENT_TIME;
        //ardprintf("RED start %l", BTN_RED_STARTED); // DEBUG ONLY
    }

    // PRESS FINISHED ==========================================================
    /*BLUE BTN press finished*/
    if (BTN_BLUE_PREV == LOW && BTN_BLUE == HIGH){
        unsigned long ELAPSED_TIME = CURRENT_TIME - BTN_BLUE_STARTED;
        if (ELAPSED_TIME < BTN_MEDIUM_LONG){
            BTN_BLUE_SHORT_PRESS = true;
        } else if (ELAPSED_TIME < BTN_LONG) {
            BTN_BLUE_MEDIUM_PRESS = true;
        } else {
            BTN_BLUE_LONG_PRESS = true;
        }
        //ardprintf("BLUE ended: %l; %l", CURRENT_TIME, ELAPSED_TIME);
    }

    /*RED BTN press finished*/
    if (BTN_RED_PREV == LOW && BTN_RED == HIGH){
        unsigned long ELAPSED_TIME = CURRENT_TIME - BTN_RED_STARTED;
        if (ELAPSED_TIME < BTN_MEDIUM_LONG){
            BTN_RED_SHORT_PRESS = true;
        } else if (ELAPSED_TIME < BTN_LONG) {
            BTN_RED_MEDIUM_PRESS = true;
        } else {
            BTN_RED_LONG_PRESS = true;
        }
        //ardprintf("RED ended: %l; %l", CURRENT_TIME, ELAPSED_TIME);
    }

    // assign values to old
    BTN_BLUE_PREV = BTN_BLUE;
    BTN_RED_PREV = BTN_RED;
}

