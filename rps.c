// Compile with '-lpanel -lmenu -lncurses'

#include <ncurses.h>
#include <panel.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define BGhud 10
#define BGhelp 11

#define HELP_WIDTH 31
#define CHOICES_y0 (LINES/2 - 3)
#define CHOICES_x0 (COLS/2 - 20)
/*
  ____
_/  __)_____
       _____)
       ______)
       _____)
__________)

        ____
  _____(__  \_
 (_____
(______
 (_____
   (__________
   PAPER

  ____
_/  __)_____
     _______)
    (__)
    (__)__
__________)

       ____
 _____(__  \_
(_______
     (__)
   __(__)
  (__________
   ROCK

  ____
_/  __)_____
       _____)
    _________)
   (___)
___(__)

        ____
  _____(__  \_
 (_____
(_________
      (___)
       (__)___
   SCISSORS
*/
/* Displays the help (in a created window and panel, for going back to the normal field after) */
void Help ()
{
	WINDOW *help;
	PANEL *up;

	help = newwin (12, HELP_WIDTH, 1, 0);
	up = new_panel (help);
	update_panels ();
	doupdate ();

	box (help, 0, 0);
	wbkgd (help, COLOR_PAIR (BGhelp));
	wrefresh (help);
	mvwaddstr (help, 0, HELP_WIDTH/2 - 2, "HELP");

	wattron (help, A_BOLD);
	mvwaddstr (help, 1, 1, "Arrow Keys or A,D:");
	mvwaddstr (help, 2, 1, "Enter or Mouse B1:");
	mvwaddstr (help, 3, 1, "'r':");
	mvwaddstr (help, 4, 1, "'p':");
	mvwaddstr (help, 5, 1, "'s':");
	mvwaddstr (help, 6, 1, "'q':");

	mvwaddstr (help, 8, 1, "Rock");
	mvwaddstr (help, 9, 1, "Paper");
	mvwaddstr (help, 10, 1, "Scissors");

	wattroff (help, A_BOLD);
	mvwaddstr (help, 1, 20, "left/right");
	mvwaddstr (help, 2, 20, "choose");
	mvwaddstr (help, 3, 6, "Rock");
	mvwaddstr (help, 4, 6, "Paper");
	mvwaddstr (help, 5, 6, "Scissors");
	mvwaddstr (help, 6, 6, "quit");

	mvwaddstr (help, 8, 6, "beats Scissors");
	mvwaddstr (help, 9, 7, "beats Rock");
	mvwaddstr (help, 10, 10, "beats Paper");


// writes the help window, wait for some key to be pressed and delete the help window
	wrefresh (help);
	getch ();

	wstandend (help);
	werase (help);
	wrefresh (help);
	del_panel (up);
	delwin (help);
}


/* Menu for choosing the color */
int Colors ()
{
	MEVENT event;
	int i, x[8], c;
	char *colors[] = {
		"White",
		"Black",
		"Red",
		"Yellow",
		"Green",
		"Cyan",
		"Blue",
		"Magenta"
	};

	x[0] = 18;
	for (i = 1; i < 8; i++)
		x[i] = x[i - 1] + strlen (colors[i - 1]) + 1;

	mvaddstr (9, 0, "Pick your color >");

	attron (A_BOLD);
	for (i = 1; i < 8; i++) {
		attron (COLOR_PAIR (i + 1));
		mvaddstr (9, x[i], colors[i]);
	}

	i = 0;
	attron (COLOR_PAIR (1) | A_REVERSE);
	mvaddstr (9, x[i], colors[i]);

	while (1) {
		c = getch ();

		switch (c) {
			case KEY_LEFT: case 'a':
				if (i > 0) {
					attroff (A_REVERSE);
					mvaddstr (9, x[i], colors[i]);
					i--;
					attron (A_REVERSE | COLOR_PAIR (i + 1));
					mvaddstr (9, x[i], colors[i]);
				}
				break;

			case KEY_RIGHT: case 'd':
				if (i < 7) {
					attroff (A_REVERSE);
					mvaddstr (9, x[i], colors[i]);
					i++;
					attron (A_REVERSE | COLOR_PAIR (i + 1));
					mvaddstr (9, x[i], colors[i]);
				}
				break;

			case '?':
				Help ();
				break;

			case '\n':
				return i + 1;

			case KEY_MOUSE:
				getmouse (&event);
				if (event.bstate & BUTTON1_CLICKED) {
// if inside the color names area
					if (event.y == 9 && event.x >= x[0] && event.x < (x[7] + strlen (colors[7]))) {
// check: when x is greater than initial x, that's the color
						for (i = 7; i >= 0; i--) {
							if (event.x >= x[i]) {
								return i + 1;
							}
						}
					}
				}

		}
	}
}


/* Print a paper hand */
void PrintPaper (WINDOW *win, int y, int x, char reverse)
{
	if (reverse) {
		mvwaddstr (win, y, x,     "        ____");
		mvwaddstr (win, y + 1, x, "  _____(__  \\_");
		mvwaddstr (win, y + 2, x, " (_____");
		mvwaddstr (win, y + 3, x, "(______");
		mvwaddstr (win, y + 4, x, " (_____");
		mvwaddstr (win, y + 5, x, "   (__________");
	}
	else {
		mvwaddstr (win, y, x,     "  ____");
		mvwaddstr (win, y + 1, x, "_/  __)_____");
		mvwaddstr (win, y + 2, x, "       _____)");
		mvwaddstr (win, y + 3, x, "       ______)");
		mvwaddstr (win, y + 4, x, "       _____)");
		mvwaddstr (win, y + 5, x, "__________)");
	}
	wattrset (win, A_BOLD);
	mvwaddstr (win, y + 7, x, "    PAPER");

	wrefresh (win);
}


/* Print a scissors hand */
void PrintScissors (WINDOW *win, int y, int x, char reverse)
{
	if (reverse) {
		mvwaddstr (win, y, x,     "        ____");
		mvwaddstr (win, y + 1, x, "  _____(__  \\_");
		mvwaddstr (win, y + 2, x, " (_____");
		mvwaddstr (win, y + 3, x, "(_________");
		mvwaddstr (win, y + 4, x, "      (___)");
		mvwaddstr (win, y + 5, x, "       (__)___");
	}
	else {
		mvwaddstr (win, y, x,     "  ____");
		mvwaddstr (win, y + 1, x, "_/  __)_____");
		mvwaddstr (win, y + 2, x, "       _____)");
		mvwaddstr (win, y + 3, x, "    _________)");
		mvwaddstr (win, y + 4, x, "   (___)");
		mvwaddstr (win, y + 5, x, "___(__)");
	}
	wattrset (win, A_BOLD);
	mvwaddstr (win, y + 7, x, "   SCISSORS");

	wrefresh (win);
}


/* Print a rock hand */
void PrintRock (WINDOW *win, int y, int x, char reverse)
{
	if (reverse) {
		mvwaddstr (win, y, x,     "       ____");
		mvwaddstr (win, y + 1, x, " _____(__  \\_");
		mvwaddstr (win, y + 2, x, "(_______");
		mvwaddstr (win, y + 3, x, "     (__)");
		mvwaddstr (win, y + 4, x, "   __(__)");
		mvwaddstr (win, y + 5, x, "  (__________");

	}
	else {
		mvwaddstr (win, y, x,     "  ____");
		mvwaddstr (win, y + 1, x, "_/  __)_____");
		mvwaddstr (win, y + 2, x, "     _______)");
		mvwaddstr (win, y + 3, x, "    (__)");
		mvwaddstr (win, y + 4, x, "    (__)__");
		mvwaddstr (win, y + 5, x, "__________)");
	}
	wattrset (win, A_BOLD);
	mvwaddstr (win, y + 7, x, "    ROCK");

	wrefresh (win);
}


/* Create the choices window, that's shown for player to choose his move */
WINDOW *CreateChoices (int color)
{
	WINDOW *win;

	win = newwin (8, 42, CHOICES_y0, CHOICES_x0);

	wattrset (win, COLOR_PAIR (color) | A_BOLD);
	PrintRock (win, 0, 0, 0);
	wattrset (win, COLOR_PAIR (color) | A_BOLD);
	PrintPaper (win, 0, 14, 0);
	wattrset (win, COLOR_PAIR (color) | A_BOLD);
	PrintScissors (win, 0, 28, 0);

	return win;
}


/* Get the movement by mouse click */
char MouseChoose (WINDOW *choices, MEVENT event)
{
	if (event.x >= CHOICES_x0 + 28)
		return 's';
	else if (event.x >= CHOICES_x0 + 14)
		return 'p';
	else
		return 'r';
}


/* Who won? Who's next? */
void Game (char c)
{

}



int main ()
{
	char choice;	// 'R'ock, 'P'aper or 'S'cissors
	unsigned int best_of = 0;
	int color, c;
	PANEL *panel;
	WINDOW *hud, *player, *cpu, *choices;
	MEVENT event;

	srand (time (NULL));

	initscr ();
	keypad (stdscr, TRUE);
	start_color ();
	use_default_colors ();

	init_pair (1, COLOR_WHITE, -1);	// colors
	init_pair (2, COLOR_BLACK, -1);
	init_pair (3, COLOR_RED, -1);	// for
	init_pair (4, COLOR_YELLOW, -1);
	init_pair (5, COLOR_GREEN, -1);	// customizing
	init_pair (6, COLOR_CYAN, -1);
	init_pair (7, COLOR_BLUE, -1);	// your hand
	init_pair (8, COLOR_MAGENTA, -1);
	init_pair (BGhud, COLOR_WHITE, COLOR_GREEN);	// hud color
	init_pair (BGhelp, COLOR_WHITE, COLOR_BLUE);	// help color

	mousemask (BUTTON1_CLICKED, NULL);

	hud = subwin (stdscr, 1, COLS, 0, 0);
	wbkgd (hud, COLOR_PAIR (BGhud) | A_BOLD);
	mvwaddstr (hud, 0, COLS/2 - 10, "ROCK-PAPER-SCISSORS");
	wrefresh (hud);

	mvaddstr (6, 0, "Best of [odd number; max 99] >");
	do {
		move (6, 31);
		clrtoeol ();
		mvscanw (6, 31, "%d", &best_of);
	} while (best_of > 99 || best_of % 2 == 0);

	curs_set (0);
	noecho ();
	cbreak ();

	mvwaddstr (hud, 0, 0, "'?': Help");
	wrefresh (hud);

	color = Colors ();

	move (6, 0);
	clrtobot ();
	refresh ();

	choices = CreateChoices (color);
	panel = new_panel (choices);

	while (c != 'q') {
		c = getch ();

		if (c == KEY_MOUSE) {
			getmouse (&event);
			if (event.bstate & BUTTON1_CLICKED) {
// asked for help?
				if (wenclose (hud, event.y, event.x) && event.x < 9)
					c = '?';
// choose a movement; will you win?
				else if (wenclose (choices, event.y, event.x))
					c = MouseChoose (choices, event);
			}
		}

		switch (c) {
			case '?':
				Help ();
				break;

			case 'r': case 's': case 'p':
				Game (c);
		}
		addch (c);
	}

	endwin ();
	return 0;
}