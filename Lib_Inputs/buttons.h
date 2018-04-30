/**********************************************************************
*   File name: buttons.h
*   File owner: Milan Alden
*   Most recent editor: Milan Alden
*   This is the header file for buttons.c
***********************************************************************/

#ifndef BUTTONS_H_
#define BUTTONS_H_

#include "../Lib_Utils/event_queue.h"

enum ButtonEvent { ButtonEventNull = 0,
                    ButtonEventPress,
                    NumButtonEvents };

enum Buttons { Button1 = 0, Button2, NumButtons, NoButton };

enum ButtonState{ ButtonNull = 0,
                   ButtonReleased,
                   ButtonMaybePressed,
                   ButtonPressed,
                   ButtonMaybeReleased,
                   NumButtonStates };

typedef struct
{
    enum Buttons buttonNum;
    enum ButtonState currentState;
    unsigned int pressTime;
    unsigned int releaseTime;
} Button;

int isButtonPressed( Button *currentButton, EventQueue *queue );
void setButtonIntervalTime( int buttonIntervalTime );
void buttonTimer( Button *currentButton, EventQueue *queue );

#endif /* BUTTONS_H_ */
