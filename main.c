/**********************************************************************
*   File name: main.c
*   File owner: Tobias Madge
*   Most recent editor: Tobias Madge
 *  Created on: 23 Nov 2017
***********************************************************************/

#include <msp430.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "Lib_Utils/event_queue.h"
#include "Lib_Inputs/buttons.h"
#include "Lib_Inputs/ADC.h"
#include "Lib_Display/display.h"
#include "Lib_Interface/menu.h"

int buttonpressedflag;
int button_sampling_rate = 2;
int sensors_sampling_rate = 1;
int ECGstate = 0;
int adcReading = 1;
unsigned int IntialWaveFormSetup = 0;
int WaveformAdcData[24]; //saves all the points (every 4 pixels) of on-screen data on the board

/*below variable and code takes up too much space in memory. Work out a way around this in future
int tempWaveformDataBuffer[96];        // THIS is holding the row, and remembering the column number
                                          // this is to hold the waveform, it'll hold the previous waveform data -
                                          // so it can be wiped from the previous set without needing to access EVERY part of the data buffed
                                          // ALWAYS save to this when writing to the databuffer for the wave form.
 this code crashes everything - why?         temp = tempWaveformDataBuffer[x*8]; //find ref point for where to wipe
                                                 displayBuffer[x][temp] = 0xFF; //wipe old waveform

 this code crashes everything - why?        tempWaveformDataBuffer[x*8] = WaveformAdcData[x];//backup data in temp waveform for next runthroughs wipe =*/

/*
 * This int is a way of keeping track of what 'state' the screen is in - this way the scheduler can skip over unnesseary code
 *  Realistically this should be moved into its own function (which I can make) which will serve as an overlal controller for
 *  what state the board is in right now, being either startup, waveform or menu.
 *  Right now the numbers work as follows : 0 is startup/off (is the default but should be swiftly changed) 1 is waveform, 2 is menu.
 */
long int systemtimer = 0; //set this back to 0 before demoing
/* This counts how many times the timer has triggered and is a good way of keeping track of how long the system has been
 * running for. It's not only used for debugging, it's also used to decide when the system comes out of the default 'startup' screen
 * Nothing currently resets this value (although probably should). So technically this value will eventually go over the limit and risk crashing the system,
 * however this should be many cycles into the system, so this doesn't need to be implemented for the prototype phase.
 */


Button button1;
Button button2;

EventQueue button1_q;
EventQueue button2_q;

int sample_button_timer = 1;
int sample_sensor_timer = 1;
int button1pushed = 0;
int button2pushed = 0;
Event e;

int BPM_timer_ms = 0;
int BPM_timer_s = 0;

int button_process_event = 0;
#define BUTTON_PROCESS_EVENT_TIME 50
#define MIN_HEART_BEAT_SPIKE 150
#define SECONDS_IN_MINUTE 60

#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer0_A0 (void) {  /* Timer0 A0 1ms interrupt service routine */

    systemtimer++; /* this is currently ticking far too often, it may pay to put it in its own timer that triggers less often */
    sample_button_timer++;
    sample_sensor_timer++;
    if (sample_button_timer >= button_sampling_rate) {

        buttonTimer( &button1, &button1_q );
        buttonTimer( &button2, &button2_q );
        sample_button_timer = 0;
        button_process_event++;

        if( button_process_event >= BUTTON_PROCESS_EVENT_TIME ) {
            button_process_event = 0;
            button1pushed = isButtonPressed( &button1, &button1_q );
            button2pushed = isButtonPressed( &button2, &button2_q );
        }
    }

    if (sample_sensor_timer >= sensors_sampling_rate) {

        sample_sensor_timer = 0;
    }

    /* Time to take a reading */
    adcReading++;
    if (adcReading >= 40) {
        adcReading = 1;
    }

    BPM_timer_ms++;

    if( BPM_timer_ms == 1000 )
    {
        BPM_timer_s++;
        BPM_timer_ms = 0;
    }
}

void initialise_button1() {
    button1.buttonNum = Button1;
    button1.currentState = ButtonReleased;
    button1.pressTime = 0;
    button1.releaseTime = 0;
}

void initialise_button2() {
    button2.buttonNum = Button2;
    button2.currentState = ButtonReleased;
    button2.pressTime = 0;
    button2.releaseTime = 0;
}

void startuphandling() {

    if( systemtimer <= 10000 ) {

        writeFormattedText(startupMessage1, strlen(startupMessage1), 0, 0, 12, true);
        if( ( button2pushed == 1 )|| ( button1pushed == 1 ) ) {
            systemtimer = 10000;
        }
    }
    if( systemtimer >= 10000 && systemtimer <= 20000 ) {

        if( ( button2pushed == 1 ) || ( button1pushed == 1 ) ) {
            systemtimer = 20000;
        }
        writeFormattedText(startupMessage2, strlen(startupMessage2), 0, 0, 12, true);
    }
    if( systemtimer >= 20000 && systemtimer <= 25000 ) {

        if( ( button2pushed == 1 ) || ( button1pushed == 1 ) ) {
            systemtimer = 25000;
        }
        writeFormattedText(helpMessageButtons, strlen(helpMessageButtons), 0, 0, 12, true);
    }
    if( systemtimer >= 25000 && systemtimer <= 30000 ) {
        if( ( button2pushed == 1 ) || ( button1pushed == 1 ) ) {
            systemtimer = 30000;
         }
         writeFormattedText(helpMessageButtons2, strlen(helpMessageButtons2), 0, 0, 12, true);
    }
    if( ( systemtimer >= 30000 ) ) {
        ECGstate = 2;       /* current it defaults to going to the menu for the prototype however should probabaly go to the waveform mode by default */
    }
}

int MenuCurrentSelection = 0;
enum Buttons buttonPressMask = NoButton;

void menuflowhandling() {

     if ( ( button1pushed == 1 ) && ( button2pushed == 0 ) ) { /* scroll down one section */

         buttonPressMask = Button1;
     }
     else if ( ( button2pushed == 1 ) && ( button1pushed == 0 ) ) { /* 'enter' key */

         buttonPressMask = Button2;                   /* Buttons.Button2; */
     }
     else if((button2pushed == 1 )&&(button1pushed == 1)) { /* this needs to be here to 'overwrite' it being set to button 2 */

         buttonPressMask = NumButtons;     /*only happens if bothbuttons are pressed together */
     }
     else {
         buttonPressMask = NoButton;
     }
     updateMenuBoxes( buttonPressMask );
}

void formatCurrentBpm( char *averageBpmChar, int averageBpm ) {
    if ( averageBpm < 100 ) { //ensure 2 digit value starts with 0
        averageBpmChar[0] = '0';
        sprintf (averageBpmChar + 1, "%d", averageBpm );
    }
    else {
        sprintf (averageBpmChar, "%d", averageBpm );
    }
}

void writeBpmReadingToDisplay( int averageBpm ) {
    char averageBpmChar[3] = {0,0,0};

    if( BPM_timer_s >= 5 ) {
        formatCurrentBpm( averageBpmChar, averageBpm);
        writeText( averageBpmChar, sizeof(averageBpmChar) / sizeof(averageBpmChar[0]), 9, 0, false);
    }
}

int calculateAverageBpm( int numHeartBeats ) {
    if( BPM_timer_s >= 3 ) {    //provide time for readings to balance. Only start logging readings 3 seconds in
        return (numHeartBeats * SECONDS_IN_MINUTE) / BPM_timer_s - 3;
    }
    return 0;
}

int calculateCurrentBpm( char *numHeartBeats15S, int arraySize, int arrayIndex, int averageBpm ) {
    int i;
    int totalHeartBeats15S = 0;
    int averageBpm15S;

    // Count total heart beats within 15 seconds, using the index to count all but the current second
    for( i = 0; i < arraySize - 1; i++ ) {
        totalHeartBeats15S = totalHeartBeats15S + numHeartBeats15S[(arrayIndex + i) % arraySize];
    }

    if( BPM_timer_s >= 15 ) {
        //calculate average
        averageBpm15S = (totalHeartBeats15S * 60) / 15;
    }
    else if( BPM_timer_s >= 5) {
        //Use the total average since the readings are still to be populated
        averageBpm15S = averageBpm;
    }

    return averageBpm15S;
}

void cleanWaveform(){
    unsigned int x;
    unsigned int y;
    for(y = 12;y < 96; y++){
        for(x = 0; x< 12; x++){
            displayBuffer[x][y] = 0xFF;
        }
    }
}

void connectDots(int waveMode){
   unsigned int y, x, y2, waveConnectPoint, diff, gradiant;

if (waveMode == 1){
   for(x = 0;x < 23; x++){   //connect dots      -     make int its own function !!
       y = WaveformAdcData[x]+12;       // current y coordinate
       y2 = WaveformAdcData[x+1]+12;    // second y coordinate to connect to
       if(y2 == y){    // if there is no difference between the two points
          if (x % 2 == 0){
               displayBuffer[x/2][y] = 0x0F; //(displayBuffer[x][temp2+12] & 0x7F); // Connect as a line
           }
           else if (x % 2 == 1){
               displayBuffer[x/2][y] = 0xF0; //(displayBuffer[x][temp2+12] & 0xF7); // Connect as a line
           }
       }

       if (y2 > y){   // if the second point is below
                     diff = y - y2;
                  }
                  else{          // if the second point is above
                     diff = y2 - y;
                  }
       if (x % 2 == 0){      // test whether x is even or odd and allocate the initial case accordingly
          waveConnectPoint = 1; //messing with first  hex in char
       }
       else if (x % 2 == 1){     //messing with second hex in char
          waveConnectPoint = 5;
       }
       gradiant = diff/3;
       while(y2 != y){
              if (waveConnectPoint != 4 || waveConnectPoint != 8){
                  if(gradiant == 0){         //are we going one to the right, or not moving yet?
                      waveConnectPoint++; // one bit right
                      gradiant = diff/3;
                  }else{ //already correct place don't move
                      gradiant--;
                  }
              }
           switch(waveConnectPoint){    // switch between even odd cases and whichbyte we're modifying
               case 1:
                   displayBuffer[x/2][y] = displayBuffer[x/2][y] & 0xBF;
                   break;
               case 2:
                   displayBuffer[x/2][y] = displayBuffer[x/2][y] & 0xDF;
                   break;
               case 3:
                   displayBuffer[x/2][y] = displayBuffer[x/2][y] & 0xEF;
                   break;
               case 4: // this is the breakpoint case if it gets here there's an error
                   break;
               case 5:
                   displayBuffer[x/2][y] = displayBuffer[x/2][y] & 0xFB;
                   break;
               case 6:
                   displayBuffer[x/2][y] = displayBuffer[x/2][y] & 0xFD;
                   break;
               case 7:
                   displayBuffer[x/2][y] = displayBuffer[x/2][y] & 0xFE;
                   break;
               default:
                   //should never be here but should probabally add an error at some point for more resliant code
           }
           if (y2 > y){   // if the second point is below
              y++;
           }
           else{          // if the second point is above
              y--;
           }
       }
   }
}
else if(waveMode == 2){
    for(x = 0;x < 23; x++){   //connect dots      -     make int its own function !!
        y = WaveformAdcData[x]+12;       // current y coordinate
        y2 = WaveformAdcData[x+1]+12;    // second y coordinate to connect to
        if(y2 == y){    // if there is no difference between the two points
           if (x % 2 == 0){
                displayBuffer[x/2][y] = 0x0F; //(displayBuffer[x][temp2+12] & 0x7F); // Connect as a line
            }
            else if (x % 2 == 1){
                displayBuffer[x/2][y] = 0xF0; //(displayBuffer[x][temp2+12] & 0xF7); // Connect as a line
            }
        }

        if (y2 > y){   // if the second point is below
                      diff = y - y2;
                   }
                   else{          // if the second point is above
                      diff = y2 - y;
                   }
        if (x % 2 == 0){      // test whether x is even or odd and allocate the initial case accordingly
           waveConnectPoint = 1; //messing with first  hex in char
        }
        else if (x % 2 == 1){     //messing with second hex in char
           waveConnectPoint = 5;
        }
        gradiant = diff/3;
        while(y2 != y){
               if (waveConnectPoint != 4 || waveConnectPoint != 8){
                   if(gradiant == 0){         //are we going one to the right, or not moving yet?
                       waveConnectPoint++; // one bit right
                       gradiant = diff/3;
                   }else{
                       //already correct place don't move
                       gradiant--;
                   }
               }
            switch(waveConnectPoint){    // switch between even odd cases and whichbyte we're modifying
                case 1:
                    displayBuffer[x/2][y] = displayBuffer[x/2][y] & 0xBF;
                case 2:
                    displayBuffer[x/2][y] = displayBuffer[x/2][y] & 0xDF;
                case 3:
                    displayBuffer[x/2][y] = displayBuffer[x/2][y] & 0xEF;
                case 4:
                    // this is the breakpoint case if it gets here there's an error
                case 5:
                    displayBuffer[x/2][y] = displayBuffer[x/2][y] & 0xFB;
                case 6:
                    displayBuffer[x/2][y] = displayBuffer[x/2][y] & 0xFD;
                case 7:
                    displayBuffer[x/2][y] = displayBuffer[x/2][y] & 0xFE;
                default:
                    //should never be here but should probabally add an error at some point for more resliant code
            }
            if (y2 > y){   // if the second point is below
               y++;
            }
            else{          // if the second point is above
               y--;
            }
        }
    }
}
else{
// NOWAVEFORM!
}
}


void fullInit(){

    // Initialisation
    WDTCTL = WDTPW | WDTHOLD;  // Stop watchdog timer
    PM5CTL0 &= ~LOCKLPM5;      //Disable the power-on high-impedance mode

    //inputs
    P1DIR &= ~0x02;            // Setting P1.1 to input (switch2)
    P1REN |= 0x02;             // Set P1.1 resistor to be pull up enabled

    //outputs
    P1DIR |= 0x01;             //Set P1.0 to output
    P4DIR |=0x40;              //setting P4.0 to output

    //other
    TA0CCR0 = 1024;            // Count up to 1024
    TA0CCTL0 = 0x10;           // Enable counter interrupts, bit 4=1
    TA0CTL =  TASSEL_2 + MC_1; // Timer A using subsystem master clock, SMCLK(1.1 MHz)
                               // and count UP to create a 1ms interrupt

    // LCD
    P4DIR |= 0x04;             // Set P4.2 to output direction (LCD Power On)
    P4DIR |= 0x08;             // Set P4.3 to output direction (LCD Enable)

    // SPI Ports
    P1SEL0 &= ~0x40;           // Set P1.6 to output direction (SPI MOSI)
    P1SEL1 |= 0x40;            // Set P1.6 to output direction (SPI MOSI)
    P1DIR |= 0x40;             // Set P1.6 to output direction (SPI MOSI)
    P2SEL0 &= ~0x04;           // Set P2.2 to SPI mode (SPI CLK)
    P2SEL1 |= 0x04;            // Set P2.2 to SPI mode (SPI CLK)
    P2DIR |= 0x04;             // Set P2.2 to output direction (SPI CLK)
    P2DIR |= 0x10;             // Set P2.4 to output direction (SPI CS)

    // SPI Interface
    UCB0CTLW0 |= UCSWRST;
    UCB0CTLW0 &= ~(UCCKPH + UCCKPL + UC7BIT + UCMSB);
    UCB0CTLW0 &= ~(UCSSEL_3);
    UCB0CTLW0 |= UCSSEL__SMCLK;
    UCB0BRW = 8;
    UCB0CTLW0 |= (UCMSB + UCCKPH + 0x00 + UCMST + UCSYNC + UCMODE_0);
    UCB0CTLW0 &= ~(UCSWRST);
    P4OUT |= 0x04;             // Turn LCD Power On
    P4OUT |= 0x08;             // Enable LCD
    P1OUT &= ~0x01;            // Set P1.0 off (Green LED)

    P4DIR  &= ~BIT5;           // Set P4.5 as input
    P4OUT |= BIT5;             // Configure P4.5 for Pull-Up
    P4REN |= BIT5;             // Enable Pull Up of P4.5

    //P4DIR |= 0x40;           // Set P4.0 to output direction

    _BIS_SR(GIE);              // interrupts enabled

    TA0CCR0 = 1023;            // Count up to 1024
    TA0CCTL0 = 0x10;           // Enable counter interrupts, bit 4=1
    TA0CCTL1 = OUTMOD_3;       // TACCR1 set/reset
    TA0CCR1 = 1023;            // TACCR1 PWM Duty Cycle
    TA0CTL = TASSEL_2 + MC_1;  // Timer A using subsystem master clock, SMCLK(1.1 MHz)
                               // and count UP to create a 1ms interrupt
                               // PWM Period
    initAdc();
}

void  shiftWaveformAdcData(){
   int i, temp;
   for (i = 23; i > 0; i--){ //shift the array to the right once - only to 23 since we don't want to overflow the array
       temp = WaveformAdcData[i-1];
       WaveformAdcData[i] = temp;
   }
}

void displayDots(){
    int x, temp2;
    for(x = 0;x < 24; x++){ // print waveform points to display buffer make own function
         if (x % 2 == 0){
             temp2 = (WaveformAdcData[x])+12; // y coordinate
             displayBuffer[x/2][temp2] = displayBuffer[x][temp2+12] & 0x7F; //you need to add 12 to tempt 2 since the coordinates need to be 12 down/
         } //                                                           0x0111 1111
         else if (x % 2 == 1){
             temp2 = WaveformAdcData[x]+12; // y coordinate
             displayBuffer[x/2][temp2] = displayBuffer[x][temp2+12] & 0xF7; //you need to add 12 to tempt 2 since the coordinates need to be 12 down/
        } //                                                            0x1111 01111
    }
}

int main(void) {

    /* POST TESTS. THESE CAUSE BUTTON ISSUES SO ARE CURRENTLY DISABLED
    WDTCTL = WDTPW | WDTHOLD;       // Stop watchdog timer

    PM5CTL0 &= ~LOCKLPM5;           // Disable the GPIO power-on default high-impedance mode
                                    // to activate previously configured port settings

    // Initialisation - POST. This must be done before anything else in Main,
    //                        and inside the main function for POST checks to work correctly

    P1DIR |=  0x01;                 // Set P1.0 to output direction
    P4DIR |=  0x40;                 // Set P4.6 to output direction
    P1OUT &= ~0x01;                 // Set P1.0 off (Green LED)
    P4OUT &= ~0x40;                 // Set P4.6 off (Red LED)

    P4OUT |=  0x40;                 // Set P4.6 on  (Red LED)

    asm(
         "      mov.w #001C00h, R12 \n"
         "loop1: \n"
         "      mov.w #00000h, 0(R12) \n"
         "      cmp.w #00000h, 0(R12) \n"
         "      jne loop1 \n"
         "      add #2, R12 \n"
         "      cmp.w #002400h, R12 \n"
         "      jn loop1 \n"
        );

    P1OUT |=  0x01;                 // Set P1.0 on  (Green LED)
    P4OUT &= ~0x40;                 // Set P4.6 off (Red LED)

    asm(
         "      mov.w #001C00h, R12 \n"
         "loop2: \n"
         "      mov.w #0ffffh, 0(R12) \n"
         "      cmp.w #0ffffh, 0(R12) \n"
         "      jne loop2 \n"
         "      add #2, R12 \n"
         "      cmp.w #002400h, R12 \n"
         "      jn loop2 \n"
        );

    P1OUT &= ~0x01;                 // Set P1.0 off (Green LED)
    P4OUT |=  0x40;                 // Set P4.6 on  (Red LED)

    asm(
         "      mov.w #001C00h, R12 \n"
         "loop3: \n"
         "      mov.w #05555h, 0(R12) \n"
         "      cmp.w #05555h, 0(R12) \n"
         "      jne loop3 \n"
         "      add #2, R12 \n"
         "      cmp.w #002400h, R12 \n"
         "      jn loop3 \n"
        );

    P1OUT |=  0x01;                 // Set P1.0 on  (Green LED)
    P4OUT &= ~0x40;                 // Set P4.6 off (Red LED)

    asm(
         "      mov.w #001C00h, R12 \n"
         "loop4: \n"
         "      mov.w #0aaaah, 0(R12) \n"
         "      cmp.w #0aaaah, 0(R12) \n"
         "      jne loop4 \n"
         "      add #2, R12 \n"
         "      cmp.w #002400h, R12 \n"
         "      jn loop4 \n"
        );

    P1OUT &= ~0x01;                 // Set P1.0 off (Green LED)
    P4OUT |=  0x40;                 // Set P4.6 on  (Red LED)

    asm(
         "      mov.w #001C00h, R12 \n"
         "loop5: \n"
         "      mov.w R12, 0(R12) \n"
         "      add #2, R12 \n"
         "      cmp.w #002400h, R12 \n"
         "      jn loop5 \n"

         "      mov.w #001C00h, R12 \n"
         "loop6: \n"
         "      cmp.w R12, 0(R12) \n"
         "      jne loop6 \n"
         "      add #2, R12 \n"
         "      cmp.w #002400h, R12 \n"
         "      jn loop6 \n"
        );

    P1OUT |=  0x01;                 // Set P1.0 on  (Green LED)
    P4OUT &= ~0x40;                 // Set P4.6 off (Red LED)

    asm(
         "      mov.w #001C00h, R12 \n"
         "loop7: \n"
         "      mov.w #00000h, 0(R12) \n"
         "      add #2, R12 \n"
         "      cmp.w #002400h, R12 \n"
         "      jn loop7 \n"
        );

    P1OUT &= ~0x01;                 // Set P1.0 off (Green LED)
    P4OUT &= ~0x40;                 // Set P4.6 off (Red LED)
    */


    fullInit(); /* initlization hardware */

    initialise_button1();
    initialise_button2();

    setButtonIntervalTime( button_sampling_rate );
    int firstimemenu = 0;

    char doneReading = 0;

    int currentAdc = 0;
    int temp;
    int waveMode;

    int averageBpm = 0;
    int currentBpm = 0;
    int prevAdcVal = 0;
    int numHeartBeats = 0;
    char numHeartBeatsIn15S[16] = {0};
    int numHeartBeatsIn15SIndex = 0;
    int previousTimeSeconds = 0;
    bool Bpm_mode_is_curr = true;


    waveMode = 2;
   while(1)
   {
       /* this startup phase stuff should maybe moved to its own function to keep main clean. Needs feedback from group */
       if(ECGstate == 0) /* if in startup mode, this is only run once */
       {
           initDisplayBuffer(0xFF);
           startuphandling();
           firstimemenu = 1;
           outputDisplayBuffer();
       }

       if (ECGstate == 1)
       {  // if in 'Waveform mode'
           // send sensor data to waveform creating function
           // send waveform to display function
           // send number to screen directly via writeText or call a function that does so


           //writeText( "ECG data here ", 20, 1 , 20 , false);

           if( button1pushed == 1 && button2pushed == 1 )
           {
               ECGstate = 2;
               firstimemenu = 1;
           }
           else if(button2pushed == 1){
               Bpm_mode_is_curr = !Bpm_mode_is_curr;
           }
           else if(button1pushed == 1){
               if(waveMode < 3){
                   waveMode++;
               }
               else{
                   waveMode = 1;
               }
           }

           if (adcReading == 1) {
               if (doneReading == 0) {

                   //Extract ADC Reading
                   currentAdc = getAdcValue();

                   //check change in time
                   if( BPM_timer_s != previousTimeSeconds ) {
                       numHeartBeatsIn15SIndex++;
                       numHeartBeatsIn15S[numHeartBeatsIn15SIndex % ( sizeof(numHeartBeatsIn15S) / sizeof(numHeartBeatsIn15S[0]))] = 0;
                       previousTimeSeconds++;
                   }

                   //detect heart beat spike
                   if( prevAdcVal != 0 && ( currentAdc > ( prevAdcVal + MIN_HEART_BEAT_SPIKE ) ) ) {
                       numHeartBeats++;
                       numHeartBeatsIn15S[numHeartBeatsIn15SIndex % ( sizeof(numHeartBeatsIn15S) / sizeof(numHeartBeatsIn15S[0]))]++;
                   }

                   averageBpm = calculateAverageBpm(numHeartBeats);
                   currentBpm = calculateCurrentBpm( numHeartBeatsIn15S, sizeof(numHeartBeatsIn15S) / sizeof(numHeartBeatsIn15S[0]),
                                                                                                                 numHeartBeatsIn15SIndex, averageBpm );

                   // We only have space for 1 BPM reading type to be displayed
                   if( Bpm_mode_is_curr ) {
                       writeText( "CUR BPM: ", 9, 0, 0, false);
                       writeBpmReadingToDisplay( currentBpm );
                   }
                   else {
                       writeText( "AVG BPM: ", 9, 0, 0, false);
                       writeBpmReadingToDisplay( averageBpm );
                   }

                   prevAdcVal = currentAdc;

                   /////////////////////////////////////////////////////////////////////////////////////////////////

                   temp = currentAdc;

                   //Fit adc values to waveform. Expected range 1700-2400. This will be scaled to 0 - 1400
                   //temp = getAdcValue();
                   if (temp <= 2400 && temp >= 1700){
                       //currentAdc = temp - 1700; // purely for debugging
                       temp = temp - 1700;
                       temp = temp * 2;
                       currentAdc = temp;
                   }
                   else{
                       currentAdc = 0; // we should instead flag if there's an issue - but for now this should do
                   }

                       //sprintf (adcRdg, "%d", currentAdc ); //convert int of 5bytes into 5 seperate bytes
                       //writeText( adcRdg, sizeof(adcRdg) / sizeof(adcRdg[0]), 0, 01, false);

                       //writeText(":)" , 2, 6, 01, true);
                       ////NOTE TO SELF clean this up by putting it in its own function in display.c
                       //Below sets up the waveform data graph
                       if(IntialWaveFormSetup >= 24){  //this is to make sure the waveform will be ready - this will only need to be done once per runtime

                           shiftWaveformAdcData();
                           WaveformAdcData[0] = currentAdc/17;
                           cleanWaveform();
                           displayDots();
                           connectDots(waveMode);
                       }
                       else{
                           WaveformAdcData[IntialWaveFormSetup] = currentAdc/17.5;
                           IntialWaveFormSetup++;
                       }

                   }
               outputDisplayBuffer();
               doneReading = 1;
           }
           else{
           doneReading = 0;
           }
       }

       if (ECGstate == 2) // menu mode
       {
           if(firstimemenu == 1){
               initialiseMenuBoxes(3);
               firstimemenu = 0;
           }
           if(button1pushed == 1 || button2pushed == 1){
               menuflowhandling();
           }

           outputDisplayBuffer();

           if( ECGstate == 1 ) // clear the screen for waveform mode
                               // This is done here to prevent a screen wipe every waveform iteration
           {
               initDisplayBuffer(0xFF);
               BPM_timer_ms = 0;
               BPM_timer_s = 0;
               numHeartBeats = 0;
           }

       }
       //outputDisplayBuffer();
   }


return 0;
}

