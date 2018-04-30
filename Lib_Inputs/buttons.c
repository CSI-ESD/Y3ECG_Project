/**********************************************************************
*   File name: buttons.c
*   File owner: Milan Alden
*   Most recent editor: Milan Alden
*   This is the implementation file for buttons.c
***********************************************************************/

#include <msp430.h>
#include "buttons.h"

static void buttonReleasedEnter( Button *currentButton, EventQueue *queue );
static void buttonReleasedTimer( Button *currentButton, EventQueue *queue );
static void buttonMaybePressedEnter( Button *currentButton, EventQueue *queue );
static void buttonMaybePressedTimer( Button *currentButton, EventQueue *queue );
static void buttonPressedEnter( Button *currentButton, EventQueue *queue );
static void buttonPressedTimer( Button *currentButton, EventQueue *queue);
static void enterButtonState( enum ButtonState newState, Button *currentButton, EventQueue *queue );

#define MINIMUM_PRESS_TIME 100
#define MAXIMUM_RELEASE_TIME 100
#define BUTTON_HELD_TIME 200

#define TRUE 1
#define FALSE 0

int intervalTime = 5;

int isButtonPressed( Button *currentButton, EventQueue *queue ) {
    Event e;

    if( readQ( queue, &e ) ) {
        if( e.event == ButtonEventPress ) {
            return 1;
        }
    }

    return 0;
}

/**********************************************************************
*   Function name: setButtonIntervalTime
*   Desc: set the interval time this function is called from,
*         allowing the button handler to time each button press
***********************************************************************/
void setButtonIntervalTime( int buttonIntervalTime ) {
    intervalTime = buttonIntervalTime;
}

/**********************************************************************
*   Function name: currentButtonPressed
*   Desc: Check if the current button has been pressed, returning true
*         if pressed and false if not pressed
***********************************************************************/
int currentButtonPressed( Button *currentButton ) {
    int buttonPressed = FALSE;

    if( currentButton->buttonNum == Button1 && ( P1IN & BIT1 ) == 0 ) {
        buttonPressed = TRUE;
    }
    else if ( currentButton->buttonNum == Button2 && (P4IN & BIT5) == 0 ) {
        buttonPressed = TRUE;
    }

    return buttonPressed;
}
/**********************************************************************
*   Function name: buttonReleasedEnter
*   Desc: perform initialisation for the ButtonReleased state
***********************************************************************/
void buttonReleasedEnter( Button *currentButton, EventQueue *queue ) {
}

/**********************************************************************
*   Function name: buttonReleasedTimer
*   Desc: change state to ButtonMaybePressed if current button is
*         pressed
***********************************************************************/
void buttonReleasedTimer( Button *currentButton, EventQueue *queue ) {
    if( currentButtonPressed( currentButton ) ) {
        enterButtonState( ButtonMaybePressed, currentButton, queue );
    }
}

/**********************************************************************
*   Function name: buttonMaybePressedEnter
*   Desc: perform initialisation for the ButtonMaybePressed state
***********************************************************************/
void buttonMaybePressedEnter( Button *currentButton, EventQueue *queue ) {
    currentButton->pressTime = 0;
}

/**********************************************************************
*   Function name: buttonMaybePressedTimer
*   Desc: de-bounce button press, changing state to ButtonReleased if
*         released and to ButtonPressed if button remains pressed for
*         a certain amount of time
***********************************************************************/
void buttonMaybePressedTimer( Button *currentButton, EventQueue *queue ) {
    const Event e = { ButtonEventPress, 0, 0, 0 };

    currentButton->pressTime++;
    if( !currentButtonPressed( currentButton ) ) {
        if ( currentButton->pressTime >= ( MINIMUM_PRESS_TIME / intervalTime ) ) {

            if (!writeQ(queue, e)) {
                //queue full
            }
        }
        enterButtonState( ButtonReleased, currentButton, queue );
    }
    else {
        if( currentButton->pressTime >= ( BUTTON_HELD_TIME / intervalTime ) ) {
            enterButtonState( ButtonPressed, currentButton, queue );
        }
    }
}

/**********************************************************************
*   Function name: buttonPressedEnter
*   Desc: perform initialisation for the button pressed state
***********************************************************************/
void buttonPressedEnter( Button *currentButton, EventQueue *queue ) {
    const Event e = { ButtonEventPress, 0, 0, 0 };

    currentButton->pressTime = 0;

    if ( !writeQ( queue, e ) ) {
        //queue full
    }
}

/**********************************************************************
*   Function name: buttonPressedTimer
*   Desc: increment button press after a set time, and change state to
*         ButtonMaybeReleased if button is released
***********************************************************************/
void buttonPressedTimer( Button *currentButton, EventQueue *queue ) {
    currentButton->pressTime++;
    if( !currentButtonPressed( currentButton ) ) {
        enterButtonState( ButtonMaybeReleased, currentButton, queue );
    }
    else {
        if( currentButton->pressTime >= (BUTTON_HELD_TIME/intervalTime )) {
            enterButtonState( ButtonPressed, currentButton, queue );
        }
    }
}


/**********************************************************************
*   Function name: button_maybe_released_enter
*   Desc: perform initialisation for the button_maybe_released state
***********************************************************************/
void button_maybe_released_enter( Button *currentButton, EventQueue *queue ) {
    currentButton->releaseTime = 0;
}

/**********************************************************************
*   Function name: button_maybe_released_timer
*   Desc: de-bounce button release, changing state to ButtonPressed if
*         pressed and to ButtonReleased if button remains released for
*         a certain amount of time
***********************************************************************/
void button_maybe_released_timer( Button *currentButton, EventQueue *queue ) {
    currentButton->releaseTime++;

    if( !currentButtonPressed( currentButton )) {
        if (currentButton->releaseTime >= ( MAXIMUM_RELEASE_TIME / intervalTime )) {
            enterButtonState( ButtonReleased, currentButton, queue );
        }
    }
    else {
        enterButtonState( ButtonPressed, currentButton, queue );
    }
}

/**********************************************************************
*   Function name: enterButtonState
*   Desc: enter a new state (can be same state, re-initialising state
*         variables
***********************************************************************/
void enterButtonState( enum ButtonState newState, Button *currentButton, EventQueue *queue ) {
    switch( newState ) {
        case ButtonNull:
            break;
        case ButtonReleased:
            buttonReleasedEnter( currentButton, queue );
            break;
        case ButtonMaybePressed:
            buttonMaybePressedEnter( currentButton, queue );
            break;
        case ButtonPressed:
            buttonPressedEnter( currentButton, queue );
            break;
        case ButtonMaybeReleased:
            button_maybe_released_enter( currentButton, queue );
        default:
            break;
    }

    currentButton->currentState = newState;
}

/**********************************************************************
*   Function name: buttonTimer
*   Desc: enter current state
***********************************************************************/
void buttonTimer( Button *currentButton, EventQueue *queue ) {

    switch ( currentButton->currentState ) {
        case ButtonNull:
            break;
        case ButtonReleased:
            buttonReleasedTimer( currentButton, queue );
            break;
        case ButtonMaybePressed:
            buttonMaybePressedTimer( currentButton, queue );
            break;
        case ButtonPressed:
            buttonPressedTimer( currentButton, queue );
            break;
        case ButtonMaybeReleased:
            button_maybe_released_timer( currentButton, queue );
        default:
            break;
    }
}
