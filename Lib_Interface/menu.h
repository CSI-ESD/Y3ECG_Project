/**********************************************************************
*   File name: menuSimple.h
*   File owner: Luke Parsons
*   Most recent editor: Luke Parsons
*
*   This is the header file declarations for data and functions
*   defined in menuSimple.c These are used in all operations relating
*   to the MENU displayState.
*
*   Disclosure: This is an initial version of the menu system which
 *  does not reflect the design specified in the low level design
 *  document for the User interface. It is a simpler, more hardcoded
 *  version that is meant to emulate what the system would look like.
 *
 *  Created on: 23 Nov 2017
 *
*   Version number: 0.3
*
*   major changes
***********************************************************************/
#include "../Lib_Inputs/buttons.h"

#ifndef MENUSIMPLE_H_
#define MENUSIMPLE_H_

/*******************************************
 *          MENU DECLARATIONS
 ******************************************/

#define MAX_STARTUP_MSG_CHARS 144

extern const unsigned char startupMessage1[MAX_STARTUP_MSG_CHARS];
extern const unsigned char startupMessage2[MAX_STARTUP_MSG_CHARS];
extern const unsigned char helpMessageButtons[MAX_STARTUP_MSG_CHARS];
extern const unsigned char helpMessageButtons2[MAX_STARTUP_MSG_CHARS];

/// @brief  screen dimensions
#define MAX_PIXELS_VERT 96
#define MAX_PIXELS_HORIZ 96

/*
 * used by writeLineOfChars though
 * works as a pretty good abstraction in any context
 * */
typedef enum{
    VERTICAL = 0,
    HORIZONTAL = 1
}LineDirection;

/// @brief  standard size of a character on the screen
#define CHARFONT_SIZE_VERTICAL 8
#define CHARFONT_SIZE_HORIZONTAL 1

/*
 * refer to low level design for UI
 * Usable Character Area: values apply per box. E.g
 * type 1 x 117 = 117
 * type 2 x 50 = 100
 * type 3 x 30 = 90
 */
#define TYPE1_MAX_UCA 117
#define TYPE2_MAX_UCA 50
#define TYPE3_MAX_UCA 30

/*******************************************
 *     MENU OPTION STRUCT
 ******************************************/

///
/// @brief  this structure makes up the primary menuOption structure that's used to create the interface menu.
///         contains a textual & integer/floating point values that can be used to manage and maintain components
///         such as time. Contains pointers to menuOptions which link to other options based on the button pressed.
///
typedef struct{
    char    text[16];
    int     intval;
    float   floatval;
    struct  MenuOption* prevMenu;    //Navigate to previous menu level
    struct  MenuOption* nextMenu;    //Navigate to next menu level
    struct  Menuoption* nextOption;   //Navigate to next option within the same menu level
}MenuOption;

/*******************************************
 *         FUNCTION PROTOTYPES
 *
 *  Function descriptions included in both
 *  .h and .c files to accommodate various
 *                  IDE's
 ******************************************/

///
/// @brief  writes a line of charToWrite starting from position and dictated by lineDirection
///
/// @param[in] charToWrite      Char to write to display buffer
/// @param[in] position         position from which to begin writing the chars
/// @param[in] lineDir          Either vertical or Horizontal
///
void writeLineOfChars(char charToWrite, short position, LineDirection lineDir);

///
/// @brief  Writes text to display buffer using wordwrap determined by maxCharsPerLine
///
/// @param[in] inText           Text to write to screen
/// @param[in] size             size of the text to write out
/// @param[in] positionX        the X position from which to begin writing & wrap cursor to on word wrap
/// @param[in] positionY        the Y position from which to begin writing
/// @param[in] maxCharsPerLine  Maximum number of characters that can be written per horizontal line
/// @param[in] inverted         determines whether the text is the highlighted element or the background
///
void writeFormattedText(const unsigned char *inText, int size, int positionX,int positionY, short maxCharsPerLine, bool inverted);

///
/// @brief  writes the Horizontal borders to the buffer based on the type
///
/// @param[in] type = { 1, 2, 3 }             determines number of borders
///
void writeHorizontalBorders( short type );

///
/// @brief  writes the vertical borders to the buffer based on the type
///
/// @param[in] type = { 1, 2, 3 }             determines number of borders
///
int writeVerticalBorders( short type );

///
/// @brief  writes both the vertical and horizontal borders
///
/// @param[in] type = { 1, 2, 3 }             determines number of text boxes
///
int writeTextBoxes( short type );

///
/// @brief  populate the previously generated text boxes with MenuText based on the below parameters
///
/// @param[in] menuText         pointer to text that will be written to the display buffer
/// @param[in] type             Determines the number of boxes on screen
/// @param[in] boxNum           which box number to insert text into
/// @param[in] inverted         determines whether the text is the highlighted element or the background
///
int populateTextBox(const char *menuText, short type, short boxNum, bool inverted);

///
/// @brief  Called on startup as well as whenever entering or re-entering the menu from a different displayState.
///         once in the menu and initialiseMenuBoxes has been called, updateMenuBoxes should be called each button input
///
/// @param[in] type = { 1, 2, 3 }             determines number of text boxes
///
void initialiseMenuBoxes( int type );

///
/// @brief  skeleton function to initialise the menuOption structure [NOT USED YET]
///
/// @param[in] type = { 1, 2, 3 }             determines number of borders
///
void initMenu(int type);

///
/// @brief  Function called to update menu boxes based on buttonPress passed in
///
/// @param[in] buttonPress      Enumeration of button presses to keep consistent across code base
///
int updateMenuBoxes( enum Buttons buttonPress );


#endif /* MENUSIMPLE_H_ */
