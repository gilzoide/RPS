#define CPU_SCISSORS

// Compile with '-lpanel -lncurses'

#include <ncurses.h>
#include <panel.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

#define BGhud 10
#define BGhelp 11
#define FGred 3
#define FGyellow 4

#define HELP_WIDTH 37
#define CHOICES_y0 (LINES/2 - 5)
#define CHOICES_x0 (COLS/2 - 20)
#define PLAYER_y0 (CHOICES_y0 - 1)
#define PLAYER_x0 10
#define CPU_y0 (CHOICES_y0 - 1)
#define CPU_x0 (COLS - 26)

int player_score = 0, cpu_score = 0;	// scores
unsigned int best_of = 0;	// it's a best of ?? game
/*
  ____			        ____
_/  __)_____	  _____(__  \_
       _____)	 (_____
       ______)	(______
       _____)	 (_____
__________)		   (__________
			PAPER

  ____			       ____
_/  __)_____	 _____(__  \_
     _______)	(_______
    (__)		     (__)
    (__)__		   __(__)
__________)		  (__________
			ROCK

  ____			        ____
_/  __)_____	  _____(__  \_
       _____)	 (_____
    _________)	(_________
   (___)		      (___)
___(__)			       (__)___
			SCISSORS
*/

/* Displays the help (in a created window and panel, for going back to the normal field after) */
void Help (int *c)
{
	WINDOW *help;
	PANEL *up;
	MEVENT event;

	help = newwin (14, HELP_WIDTH, 1, 0);
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
	mvwaddstr (help, 6, 1, "'c':");
	mvwaddstr (help, 7, 1, "Tab:");
	mvwaddstr (help, 8, 1, "'q':");

	mvwaddstr (help, 10, 1, "Rock");
	mvwaddstr (help, 11, 1, "Paper");
	mvwaddstr (help, 12, 1, "Scissors");

// mouse clickable
	wattron (help, A_UNDERLINE);
	mvwaddstr (help, 6, HELP_WIDTH - 13, "<click here>");
	mvwaddstr (help, 7, HELP_WIDTH - 13, "<click here>");
	mvwaddstr (help, 8, HELP_WIDTH - 13, "<click here>");

	wattroff (help, A_BOLD | A_UNDERLINE);
	mvwaddstr (help, 1, 20, "left/right");
	mvwaddstr (help, 2, 20, "choose");
	mvwaddstr (help, 3, 6, "Rock");
	mvwaddstr (help, 4, 6, "Paper");
	mvwaddstr (help, 5, 6, "Scissors");
	mvwaddstr (help, 6, 6, "change color");
	mvwaddstr (help, 7, 6, "toggle animations");
	mvwaddstr (help, 8, 6, "quit");

	mvwaddstr (help, 10, 6, "beats Scissors");
	mvwaddstr (help, 11, 7, "beats Rock");
	mvwaddstr (help, 12, 10, "beat Paper");


// writes the help window, wait for some key to be pressed and delete the help window
	wrefresh (help);
	if (getch () == KEY_MOUSE) {
		getmouse (&event);
		if (wmouse_trafo (help, &event.y, &event.x, FALSE)) {
			if (event.y == 6)
				*c = 'c';
			else if (event.y == 7)
				*c = '\t';
			else if (event.y == 8)
				*c = 'q';
		}
	}

	wstandend (help);
	werase (help);
	wrefresh (help);
	del_panel (up);
	delwin (help);
}


/* Rewrite the scores [if someone won a match] */
void ReScore ()
{
	standend ();

	mvprintw (LINES - 1, 0, "Player: %2d", player_score);
	mvprintw (LINES - 1, COLS/2 - 5, "Best of: %2d", best_of);
	mvprintw (LINES - 1, COLS - 7, "Cpu: %2d", cpu_score);
}


/* Menu for choosing the color */
int Colors (int current)
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

	standend ();

	mvaddstr (6, 0, "Pick your color >");

	attron (A_BOLD | A_UNDERLINE);
	for (i = 0; i < 8; i++) {
		attron (COLOR_PAIR (i + 1));
		mvaddstr (6, x[i], colors[i]);
	}

	i = current - 1;
	attron (COLOR_PAIR (current) | A_REVERSE);
	mvaddstr (6, x[i], colors[i]);

	while (c != '\n') {
		c = getch ();

		switch (c) {
			case KEY_LEFT: case 'a':
				if (i > 0) {
					attroff (A_REVERSE);
					mvaddstr (6, x[i], colors[i]);
					i--;
					attron (A_REVERSE | COLOR_PAIR (i + 1));
					mvaddstr (6, x[i], colors[i]);
				}
				break;

			case KEY_RIGHT: case 'd':
				if (i < 7) {
					attroff (A_REVERSE);
					mvaddstr (6, x[i], colors[i]);
					i++;
					attron (A_REVERSE | COLOR_PAIR (i + 1));
					mvaddstr (6, x[i], colors[i]);
				}
				break;

			case KEY_MOUSE:
				getmouse (&event);
				if (event.bstate & BUTTON1_CLICKED)
// if inside the color names area
					if (event.y == 6 && event.x >= x[0] && event.x < (x[7] + strlen (colors[7])))
// check: when x is greater than initial x, that's the color
						for (i = 7; i >= 0; i--)
							if (event.x >= x[i]) {
								c = '\n';
								break;
							}
		}
	}

	move (6, 0);
	clrtoeol ();
	refresh ();

	return i + 1;
}


/* Print hands based on the movement */
void PrintMovement (WINDOW *win, int y, int x, char type, char reverse)
{
	switch (type) {
// unprint, for the closed fist
		case 0:
			mvwaddstr (win, y, x,     "              ");
			mvwaddstr (win, y + 1, x, "              ");
			mvwaddstr (win, y + 2, x, "              ");
			mvwaddstr (win, y + 3, x, "              ");
			mvwaddstr (win, y + 4, x, "              ");
			mvwaddstr (win, y + 5, x, "              ");
			break;
// Rocketry! [print the rocket part and a closed fist, no break =P]
		case 'R':
			if (reverse) {
				mvwaddstr (win, y + 1, x + 12, "/| ");
				mvwaddstr (win, y + 2, x + 12, "|| ");
				mvwaddstr (win, y + 3, x + 12, "|| ");
				mvwaddstr (win, y + 4, x + 12, "|| ");
				mvwaddstr (win, y + 5, x + 12, "|| ");
				mvwaddstr (win, y + 6, x + 12, "\\| ");
			}
			else {
				mvwaddstr (win, y + 1, x, " |\\");
				mvwaddstr (win, y + 2, x, " ||");
				mvwaddstr (win, y + 3, x, " ||");
				mvwaddstr (win, y + 4, x, " ||");
				mvwaddstr (win, y + 5, x, " ||");
				mvwaddstr (win, y + 6, x, " |/");
			}
// closed fist
		case 'c':
			if (reverse) {
				mvwaddstr (win, y, x + 6,       "____ ");
				mvwaddstr (win, y + 1, x + 5,  "(__  \\_");
				mvwaddstr (win, y + 2, x + 4, "(__) ");
				mvwaddstr (win, y + 3, x + 4, "(__) ");
				mvwaddstr (win, y + 4, x + 4, "(__) ");
				mvwaddstr (win, y + 5, x + 4, "(__)____");
			}
			else {
				mvwaddstr (win, y, x + 3,     "  ____");
				mvwaddstr (win, y + 1, x + 3, "_/  __)");
				mvwaddstr (win, y + 2, x + 3, "    (__)");
				mvwaddstr (win, y + 3, x + 3, "    (__)");
				mvwaddstr (win, y + 4, x + 3, "    (__)");
				mvwaddstr (win, y + 5, x + 3, "____(__)");
			}
			break;
// rock
		case 'r':
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
			mvwaddstr (win, y + 7, x,     "    ROCK");
			break;
// paper
		case 'p':
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
			mvwaddstr (win, y + 7, x,     "    PAPER");
			break;
// scissors
		case 's':
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
			mvwaddstr (win, y + 7, x,     "    SCISSORS");
			break;
	}

	wrefresh (win);
}


/* Create the choices window, that's shown for player to choose his move */
void PrintChoices (WINDOW *win, int color)
{
	wattrset (win, COLOR_PAIR (color) | A_BOLD);
	PrintMovement (win, 0, 0, 'r', 0);
	wattrset (win, COLOR_PAIR (color) | A_BOLD);
	PrintMovement (win, 0, 14, 'p', 0);
	wattrset (win, COLOR_PAIR (color) | A_BOLD);
	PrintMovement (win, 0, 28, 's', 0);
}


/* Get the movement by mouse click */
char MouseChoose (WINDOW *choices, MEVENT event, int *move)
{
	if (event.x >= CHOICES_x0 + 28) {
		*move = 2;
		return 's';
	}
	else if (event.x >= CHOICES_x0 + 14) {
		*move = 1;
		return 'p';
	}
	else {
		*move = 0;
		return 'r';
	}
}


/* Switch movement choice color */
void SwitchMove (WINDOW *choices, int i, int color)
{
	char *move[] = {
		"ROCK",
		"PAPER",
		"SCISSORS"
	};

	mvwaddstr (choices, 7, 0, "    ROCK          PAPER         SCISSORS");

	wattrset (choices, COLOR_PAIR (color) | A_REVERSE);

	mvwaddstr (choices, 7, 4 + (i * 14), move[i]);

	wattrset (choices, A_BOLD);

	wrefresh (choices);
}


/* You won! YAY */
void Winner (WINDOW *player, WINDOW *cpu, int player_color)
{
	int i, j;
	
	sleep (1);
	
	attrset (COLOR_PAIR (player_color) | A_BOLD);
	PrintMovement (stdscr, PLAYER_y0 + 1, PLAYER_x0 + 1, 0, 0);
	PrintMovement (stdscr, PLAYER_y0 + 1, PLAYER_x0, 'R', 0);
	for (i = 0; i < 7; ++i) {
// rocket fire [first red]
		attron (COLOR_PAIR (FGred));
		for (j = 1; j <= 6; j++) {
			mvaddch (PLAYER_y0 + 1 + j, PLAYER_x0, '<');
		}
		refresh ();
		usleep (6e4);
// then yellow
		attron (COLOR_PAIR (FGyellow));
		for (j = 1; j <= 6; j++) {
			mvaddch (PLAYER_y0 + 1 + j, PLAYER_x0, '<');
		}
		refresh ();
		usleep (6e4);
	}
	
	for (i = PLAYER_x0; i < CPU_x0 + 6; i++) {
		attrset (COLOR_PAIR (player_color) | A_BOLD);
		PrintMovement (stdscr, PLAYER_y0 + 1, i, 'R', 0);
// rocket fire [first red]
		attron (COLOR_PAIR (FGred));
		for (j = 1; j <= 6; j++) {
			mvaddch (PLAYER_y0 + 1 + j, i, '<');
		}
		refresh ();
		usleep (6e4);
// then yellow
		attron (COLOR_PAIR (FGyellow));
		for (j = 1; j <= 6; j++) {
			mvaddch (PLAYER_y0 + 1 + j, i, '<');
		}
		refresh ();
		usleep (6e4);
		
		for (j = 1; j <= 6; j++) {
			mvaddch (PLAYER_y0 + 1 + j, i, ' ');
		}
	}
	
	nodelay (stdscr, FALSE);
}


/* You lost! Sucka! */
void Loser (WINDOW *player, WINDOW *cpu, int cpu_color)
{
	int i, j;
	
	sleep (1);
	
	attrset (COLOR_PAIR (cpu_color) | A_BOLD);
	PrintMovement (stdscr, CPU_y0 + 1, CPU_x0 + 1, 0, 0);
	PrintMovement (stdscr, CPU_y0 + 1, CPU_x0, 'R', 1);
	for (i = 0; i < 7; ++i) {
// rocket fire [first red]
		attron (COLOR_PAIR (FGred));
		for (j = 1; j <= 6; j++) {
			mvaddch (CPU_y0 + 1 + j, CPU_x0 + 14, '>');
		}
		refresh ();
		usleep (6e4);
// then yellow
		attron (COLOR_PAIR (FGyellow));
		for (j = 1; j <= 6; j++) {
			mvaddch (CPU_y0 + 1 + j, CPU_x0 + 14, '>');
		}
		refresh ();
		usleep (6e4);
	}
	
	for (i = CPU_x0; i > PLAYER_x0 - 5; i--) {
		attrset (COLOR_PAIR (cpu_color) | A_BOLD);
		PrintMovement (stdscr, CPU_y0 + 1, i, 'R', 1);
// rocket fire [first red]
		attron (COLOR_PAIR (FGred));
		for (j = 1; j <= 6; j++) {
			mvaddch (CPU_y0 + 1 + j, i + 14, '>');
		}
		refresh ();
		usleep (6e4);
// then yellow
		attron (COLOR_PAIR (FGyellow));
		for (j = 1; j <= 6; j++) {
			mvaddch (CPU_y0 + 1 + j, i + 14, '>');
		}
		refresh ();
		usleep (6e4);
		
		for (j = 1; j <= 6; j++) {
			mvaddch (CPU_y0 + 1 + j, i + 14, ' ');
		}
	}
	
	nodelay (stdscr, FALSE);
}


/* Who won? Who's next? */
void Game (char player_choice, int player_color, char animations)
{
	WINDOW *player, *cpu, *count;
	char cpu_choice, cpu_color;
	int i;

// boxes for showing the moves: yours
	player = newwin (10, 16, CHOICES_y0 - 1, PLAYER_x0);
	box (player, 0, 0);
	mvwaddstr (player, 0, 5, "PLAYER");
	wattrset (player, COLOR_PAIR (player_color) | A_BOLD);
	PrintMovement (player, 1, 1, 'c', 0);
// and cpu's
	cpu = newwin (10, 16, CHOICES_y0 - 1, CPU_x0);
	box (cpu, 0, 0);
	mvwaddstr (cpu, 0, 6, "CPU");
	do {
		cpu_color = (rand () % 8) + 1;
	} while (cpu_color == player_color);
	wattrset (cpu, COLOR_PAIR (cpu_color) | A_BOLD);
	PrintMovement (cpu, 1, 1, 'c', 1);

// only 1 point from winning!
	attron (A_BOLD);
	if (player_score == best_of/2) {
		mvaddstr (PLAYER_y0 - 1, PLAYER_x0, "  MATCH POINT!");
		refresh ();
	}
	if (cpu_score == best_of/2) {
		mvaddstr (CPU_y0 - 1, CPU_x0, "  MATCH POINT!");
		refresh ();
	}

#ifndef CPU_SCISSORS
// random choice for cpu
	cpu_choice = rand () % 3;
	if (cpu_choice == 0)
		cpu_choice = 'r';
	else if (cpu_choice == 1)
		cpu_choice = 'p';
	else
		cpu_choice = 's';
#endif
#ifdef CPU_SCISSORS
	cpu_choice = 's';
#endif


// countdown
	count = newwin (3, 17, CHOICES_y0 + 11, COLS/2 - 8);
	mvwaddstr (count, 0, 0, "=-=-=-=-=-=-=-=-=");
	mvwaddstr (count, 1, 0, "-               -");
	mvwaddstr (count, 2, 0, "=-=-=-=-=-=-=-=-=");

// with animations it's even cooler! [but takes a while, so sometimes it annoyes]
	if (animations) {
		mvwaddstr (count, 1, 0, "-     READY?    -");
		wrefresh (count);
		sleep (1);

// a chance to count from 5 ¿why? Cuz I want so =P
		if (!(rand () % 30))
			i = 5;
		else
			i = 3;

		for (; i > 0; i--) {
// shake hands [closed fists]
			PrintMovement (player, 1, 1, 0, 0);
			PrintMovement (cpu, 1, 1, 0, 1);
			PrintMovement (player, 2, 1, 'c', 0);
			PrintMovement (cpu, 2, 1, 'c', 1);
// count
			mvwprintw (count, 1, 0, "-       %d       -", i);
			wrefresh (count);
			usleep (5e5);

			PrintMovement (player, 2, 1, 0, 0);
			PrintMovement (cpu, 2, 1, 0, 1);
			PrintMovement (player, 1, 1, 'c', 0);
			PrintMovement (cpu, 1, 1, 'c', 1);
			usleep (5e5);

// chance to forget counting =P
			if (!(rand () % 40)) {
				mvwprintw (count, 1, 0, "-WHAT'S AFTER %d?-", i);
				wrefresh (count);
				sleep (2);
				mvwprintw (count, 1, 0, "- OH, I GOT IT! -");
				wrefresh (count);
				sleep (1);
			}
		}
		mvwprintw (count, 1, 0, "-      GO!      -", i);
		wrefresh (count);
	}

	PrintMovement (player, 1, 1, 0, 0);
	PrintMovement (cpu, 1, 1, 0, 1);

	PrintMovement (player, 1, 1, player_choice, 0);
	PrintMovement (cpu, 1, 1, cpu_choice, 1);

	if (animations)
		usleep (8e5);

// you won!
	if ((player_choice == 'r' && cpu_choice == 's') || (player_choice == 'p' && cpu_choice == 'r') || (player_choice == 's' && cpu_choice == 'p')) {
		mvwaddstr (count, 1, 0, "-    YOU WON!   -");
		wrefresh (count);
		player_score++;
	}
// tie
	else if (player_choice == cpu_choice) {
		mvwaddstr (count, 1, 0, "-      TIE      -");
		wrefresh (count);
	}
// you lost
	else {
		mvwaddstr (count, 1, 0, "-   YOU LOST!   -");
		wrefresh (count);
		cpu_score++;
	}

	ReScore ();

// won the battle!
	if (player_score == best_of/2 + 1)
		Winner (player, cpu, player_color);
// or lost it =/
	else if (cpu_score == best_of/2 + 1)
		Loser (player, cpu, cpu_color);

	getch ();

	delwin (player);
	delwin (cpu);
	delwin (count);

	move (1, 0);
	clrtobot ();
	refresh ();

	ReScore ();
}




int main ()
{
	int color = 1, c, move = 0;
	char animations = 1;	// player want's the animations? (default = yes)
	PANEL *panel;
	WINDOW *hud, *choices;
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

	mvaddstr (3, 0, "Best of [odd number; max 99] >");
	do {
		move (3, 31);
		clrtoeol ();
		mvscanw (3, 31, "%d", &best_of);
	} while (best_of > 99 || best_of % 2 == 0);

	curs_set (0);
	noecho ();
	cbreak ();

	color = Colors (color);

	wattron (hud, A_UNDERLINE);
	mvwaddstr (hud, 0, 0, "'?': Help");
	wrefresh (hud);

	move (3, 0);
	clrtobot ();
	refresh ();

	ReScore (color);
	refresh ();

	choices = newwin (8, 42, CHOICES_y0, CHOICES_x0);
	PrintChoices (choices, color);
	SwitchMove (choices, move, color);
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
					c = MouseChoose (choices, event, &move);
			}
		}

		if (c == '?') {
			Help (&c);
			update_panels ();
			doupdate ();
		}

		switch (c) {
			case '\t':
				animations = !animations;
				break;

			case 'c':
				color = Colors (color);
				PrintChoices (choices, color);
				break;

			case KEY_LEFT: case 'a':
				if (move > 0) {
					move--;
				}
				break;

			case KEY_RIGHT: case 'd':
				if (move < 2) {
					move++;
				}
				break;

			case '\n':
				if (move == 0)
					c = 'r';
				else if (move == 1)
					c = 'p';
				else if (move == 2)
					c = 's';

			case 'r':
				move = 0;
			case 'p':
				if (c == 'p')
					move = 1;
			case 's':
				if (c == 's')
					move = 2;
// Round!
				hide_panel (panel);
				update_panels ();
				doupdate ();
				Game (c, color, animations);
		}

		if (player_score == best_of/2 + 1 || cpu_score == best_of/2 + 1) {
			c = 'q';
		}
// didn't lose nor win, so give player another move choosing
		else {
			SwitchMove (choices, move, color);
			show_panel (panel);
			update_panels ();
			doupdate ();
		}
	}

	endwin ();
	return 0;
}
