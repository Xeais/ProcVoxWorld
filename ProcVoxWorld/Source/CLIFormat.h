#pragma once //"CLI" stands for command-line interface.

/* ============== *
 * === How-to === *
 * ============== *
 * 
 * Print a line of bold, red text:
 * printf("%s%sHello, world!%s\n", FG_RED, BOLD, RESET); 
 * 
 * 
 * ============================== *
 * === Background Information === *
 * ============================== *
 *
 * -> ANSI Escape Sequences: https://gist.github.com/fnky/458719343aabd01cfb17a3a4f7296797
 * -> Build your own Command Line with ANSI Escape Codes: https://www.lihaoyi.com/post/BuildyourownCommandLinewithANSIescapecodes.html */


/* ============================ *
 * --- Character Formatting --- *
 * ============================ */

#define BOLD "\x1b[1m"    //Bold
#define NOBOLD "\x1b[22m" //Unbold

#define FAINT "\x1b[2m"    //Faint
#define NOFAINT "\x1b[22m" //Unfaint

#define ITALIC "\x1b[3m"    //Italic, frequently unsupported
#define NOITALIC "\x1b[23m" //Unitalicize, frequently unsupported

#define LINE "\x1b[4m"    //Underline 
#define NOLINE "\x1b[24m" //Un-underline 

#define BLINK "\x1b[5m"    //Blinking 
#define NOBLINK "\x1b[25m" //Unblinking

#define REV "\x1b[7m"    //Reverse colors (foreground -> background & vice versa).
#define NOREV "\x1b[27m" //Unreverse

#define HIDE "\x1b[8m"    //Hide
#define UNHIDE "\x1b[28m" //Unhide

#define STRIKE "\x1b[9m"    //Strikethrough
#define NOSTRIKE "\x1b[29m" //Un-strikethrough


/* ============== *
 * --- Colors --- *
 * ============== */

//Foreground:
#define FG_BLACK "\x1b[30m"
#define FG_RED "\x1b[31m"
#define FG_GREEN "\x1b[32m"
#define FG_YELLOW "\x1b[33m"
#define FG_BLUE "\x1b[34m"
#define FG_MAGENTA "\x1b[35m"
#define FG_CYAN "\x1b[36m"
#define FG_WHITE "\x1b[37m"
#define FG_DEFAULT "\x1b[39m"

//Background:
#define BG_BLACK "\x1b[40m"
#define BG_RED "\x1b[41m"
#define BG_GREEN "\x1b[42m"
#define BG_YELLOW "\x1b[43m"
#define BG_BLUE "\x1b[44m"
#define BG_MAGENTA "\x1b[45m"
#define BG_CYAN "\x1b[46m"
#define BG_WHITE "\x1b[47m"
#define BG_DEFAULT "\x1b[49m"
/* ------------------------------------------- *
 * Bright foreground (frequently unsupported): */
#define BFG_BLACK "\x1b[90m"
#define BFG_RED "\x1b[91m"
#define BFG_GREEN "\x1b[92m"
#define BFG_YELLOW "\x1b[93m"
#define BFG_BLUE "\x1b[94m"
#define BFG_MAGENTA "\x1b[95m"
#define BFG_CYAN "\x1b[96m"
#define BFG_WHITE "\x1b[97m"

//Bright background (frequently unsupported):
#define BBG_BLACK "\x1b[100m"
#define BBG_RED "\x1b[101m"
#define BBG_GREEN "\x1b[102m"
#define BBG_YELLOW "\x1b[103m"
#define BBG_BLUE "\x1b[104m"
#define BBG_MAGENTA "\x1b[105m"
#define BBG_CYAN "\x1b[106m"
#define BBG_WHITE "\x1b[107m"


/* ============= *
 * --- Reset --- *
 * ============= */

#define RESET "\x1b[0m"     //Total reset: Restores all default values.
#define FG_RESET "\x1b[39m" //Reset foreground color.
#define BG_RESET "\x1b[49m" //Reset background color.


 /* ========================= *
  * --- Clear Screen/Line --- *
  * ========================= */

//Screen:
#define CLEAR_CURSOR_END_SCREEN "\x1b[{0}J" 
#define CLEAR_CURSOR_BEGIN_SCREEN "\x1b[{1}J" 
#define CLEAR_ENTIRE_SCREEN "\x1b[{2}J" 

//Line:
#define CLEAR_CURSOR_END_LINE "\x1b[{0}K" 
#define CLEAR_CURSOR_BEGIN_LINE "\x1b[{1}K" 
#define CLEAR_ENTIRE_LINE "\x1b[{2}K" 


/* ======================== *
 * --- ASCII Characters --- *
 * ======================== */

#define BLOCK "█"
#define ARROW ""