/*
 * =====================================================================================
 *
 *       Filename:  skbtools.c
 *
 *    Description:  saschakb tools - Sammlung für ncurses-Anwendungen
 *    				Dieser Quelltext versucht die Fähigkeiten von C auszuschöpfen, daher
 *    				ist C99 oder neuer notwendig, um ihn zu kompilieren.
 *
 *        Version:  0.001
 *    letzte Beta:  0.000
 *        Created:  14.09.2011 11:42:00
 *          Ended:  00.00.0000 00:00:00
 *       Revision:  none
 *       Compiler:  clang
 *        compile:  Nicht zur eigenständigen Verwendung
 *
 *         Author:  Sascha K. Biermanns (saschakb), http://privacybox.de/saschakb.msg
 *        Company:
 *        License:  ISC
 *
 *   Copyright (C)  2011, Sascha K. Biermanns
 *
 * ====================================================================================
 *
 *   Permission to use, copy, modify, and/or distribute this software for any
 *   purpose with or without fee is hereby granted, provided that the above
 *   copyright notice and this permission notice appear in all copies.
 *
 *   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 *   WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 *   MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 *   ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 *   WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 *   ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 *   OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * =====================================================================================
 *
 *   Letze Änderungen:
 *   - 14.09.2011 Beginn an der Arbeit des Moduls
 *
 * =====================================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h> // Zufallsgenerator
#include <string.h>
#include <ncurses.h> // Farbige Grafische Ausgabe
#include <locale.h>
#include <stdarg.h> // Für die VA-Liste
#include "skbtools.h" // Die Modul-Beschreibung

static int vfarbe = weiss; // Vordergrundfarbe
static int hfarbe = schwarz; // Hintergrundfarbe

// -----------------------------------
// Implementation des ncurses-Bereichs
// -----------------------------------


// Funktion: Beenden mit farbiger Statusmeldung
void beenden(enum farben f, int status, char* text, ...) {
    // Reservierung für den maximalen Speicherplatz, den resttext benötigt
    // -------------------------------------------------------------------
    char *sicherheitszeiger = (char*) malloc( (sizeof(text) + 100) * sizeof(char));
    if(!sicherheitszeiger) {
        vordergrundfarbe(rot);
        printw("Fehler!\nsicherheitszeiger in beenden() erhielt keinen Speicher!\n");
        vordergrundfarbe(weiss);
        exit(EXIT_FAILURE);
    }
    char *umgewandeltertext = sicherheitszeiger;

    // Verarbeitung der Parameterliste und Umwandlung der Parameter + Text zu einem String
    // -----------------------------------------------------------------------------------
    va_list par; // Parameterliste
    va_start(par, text);
    vsprintf(umgewandeltertext, text, par);
    va_end(par);
    // -----------------------------------------------------------------------------------
    
    hinweis(f, umgewandeltertext);
    // Sicherheitszeiger löschen, sonst gibt es üble Speicherlöcher ;)
    if(!sicherheitszeiger)
        free(sicherheitszeiger);
    exit(status);
}

// Funktion: Hintergrundfarbe ändern
void hintergrundfarbe(enum farben hf) {
  hfarbe = hf;
  color_set((8 * vfarbe) + hfarbe + 1, 0);
}

// Funktion: Hinweis - für Fehlermeldungen oder ähnliches
void hinweis(enum farben f, char* text, ...) {
    // Reservierung für den maximalen Speicherplatz, den resttext benötigt
    // -------------------------------------------------------------------
    char *sicherheitszeiger = (char*) malloc( (sizeof(text) + 100) * sizeof(char));
    if(!sicherheitszeiger) {
        vordergrundfarbe(rot);
        printw("Fehler!\nsicherheitszeiger in hinweis() erhielt keinen Speicher!\n");
        vordergrundfarbe(weiss);
        exit(EXIT_FAILURE);
    }
    char *umgewandeltertext = sicherheitszeiger;

    // Verarbeitung der Parameterliste und Umwandlung der Parameter + Text zu einem String
    // -----------------------------------------------------------------------------------
    va_list par; // Parameterliste
    va_start(par, text);
    vsprintf(umgewandeltertext, text, par);
    va_end(par);
    // -----------------------------------------------------------------------------------
    
    vordergrundfarbe(f);
    textausgabe(umgewandeltertext);
    vordergrundfarbe(weiss);
    weiter();
    // Sicherheitszeiger löschen, sonst gibt es üble Speicherlöcher ;)
    if(!sicherheitszeiger)
        free(sicherheitszeiger);
}

// Implementation: Ja-Nein-Frage
bool janeinfrage(char *frage) {
	char eingabe;
	vordergrundfarbe(zyan);	textausgabe(frage);
	eingabe = taste();
	vordergrundfarbe(weiss);
	if((eingabe == 'j') || (eingabe == 'J'))
		return true;
	else
		return false;
}

// Initialisierung der ncurses-Umgebung
void ncurses_init(void (*funktion)()) {
	// Umgebungsvariable setzen
	setlocale(LC_ALL, "");

	initscr(); // beginne ncurses
	keypad(stdscr, true); // Keymapping aktivieren
	cbreak(); // kein Warten bei der Eingabe auf ENTER
	echo(); // Cursort-Echo
	scrollok(stdscr, true); // Automatisches Scrollen aktivieren
	start_color(); // Beginne mit Farben
        // Initialisierung aller Farbpaare
        for(int x = schwarz; x <= weiss; x++)
          for(int y = schwarz; y <= weiss; y++)
            init_pair((8 * x) + y + 1, x, y);
        vordergrundfarbe(weiss);
        hintergrundfarbe(schwarz);
	clear(); // Bildschirm löschen
	curs_set(0);
	atexit(funktion); // Routine, die bei der Beendung ausgeführt wird
}

// Implementation: Taste
char taste(void) {
//	int puffergroesse;
	char zeichen;

	noecho();
	zeichen = getch();
	echo();
	return(zeichen);
}


// Implementation: Textausgabe
void textausgabe(char *gesamttext, ...) {
	int zeilenlaenge = COLS; // COLS ist eine ncurses Variable
	int maxzeilen = LINES; // LINES ist eine ncurses Variable

	char textzeile[zeilenlaenge]; // Ausgabezeile
	int i; // Schleifenzähler
	int j; // Schleifenzähler
	int zeile = 0;
	bool erstausgabe = true;
	int x, y; // Speichern die Bildschirmkoordinaten (für getyx)

    // Reservierung für den maximalen Speicherplatz, den resttext benötigt
    // -------------------------------------------------------------------
    char *sicherheitszeiger = (char*) malloc( (sizeof(gesamttext) + 100) * sizeof(char));
    if(!sicherheitszeiger) {
        vordergrundfarbe(rot);
        printw("Fehler!\nsicherheitszeiger in textausgabe() erhielt keinen Speicher!\n");
        vordergrundfarbe(weiss);
        exit(EXIT_FAILURE);
    }
    char *resttext = sicherheitszeiger;
    // -------------------------------------------------------------------
    
    // Verarbeitung der Parameterliste und Umwandlung der Parameter + Text zu einem String
    // -----------------------------------------------------------------------------------
    va_list par; // Parameterliste
    va_start(par, gesamttext);
    vsprintf(resttext, gesamttext, par);
    va_end(par);
    // -----------------------------------------------------------------------------------

	for(i = 0; i < zeilenlaenge; i++)
		textzeile[i] = '\0'; // Sicherheitslöschung, sonst gibt es Fehler bei der Leerzeilenausgabe
	while(strlen(resttext) > (zeilenlaenge - 1)) {
		for(i = (zeilenlaenge - 1); (*(resttext+i) != ' ') && (i > 0); i--);
		if(!i)
			i = (zeilenlaenge - 1); // Das Wort ist so länger als die verdammte Zeile
		for(j = 0; (*(resttext+j) != '\n') && (j < i); j++);
		if(j < i)
			i = j; // Auf das Zeilenendezeichen verkürzen
		strncpy(textzeile, resttext, i); // Den Textteil kopieren
		resttext += i+1;
		while(*resttext == ' ')
			resttext++;
		// Prüfen, ob wir in der vorletzten Zeile angekommen sind
	  	getyx(stdscr, y, x); // Cursorposition feststellen
		if((erstausgabe == true) && (y >= (maxzeilen - 1))) {
			weiter();
			zeile = 0;
			erstausgabe = false;
		}
		if(zeile == (maxzeilen - 1)) {
				weiter();
				zeile = 0;
		}
		printw("%s\n", textzeile);
		for(i = 0; i < zeilenlaenge; i++)
			textzeile[i] = '\0'; // Sicherheitslöschung, sonst gibt es Fehler bei der Leerzeilenausgabe
		zeile += 1;
	}
	// Text ist kürzer als eine Zeile.
	// Prüfen, ob wir in der vorletzten Zeile angekommen sind
	if(zeile == (maxzeilen - 1)) {
			weiter();
			zeile = 0;
	}
	printw("%s\n", resttext);
	refresh();
    // Sicherheitszeiger löschen, sonst gibt es üble Speicherlöcher ;)
    if(!sicherheitszeiger)
        free(sicherheitszeiger);
}

// Implementation: Texteingabe
void texteingabe(char *eingabe, unsigned int laenge) {
  attrset(A_BOLD);
  getnstr(eingabe, laenge);
  attrset(A_NORMAL);
}

// Funktion: Vordergrundfarbe ändern
void vordergrundfarbe(enum farben vf) {
  vfarbe = vf;
  color_set((8 * vfarbe) + hfarbe + 1, 0);
}

// Implementation: waehle
int waehle(char* beschreibung, int maxzahl) {
	int ergebnis;
	char eingabe[20];

    while((ergebnis < 1) || (ergebnis > maxzahl)) {
        textausgabe(beschreibung);    
        texteingabe(eingabe, 20);  
		ergebnis = atoi(eingabe);
	}
	return ergebnis;
}

// Implementation: Weiter
void weiter(void) {
    attrset(A_BLINK);
	printw("--- Bitte ENTER-Taste drücken um fortzufahren ---");
    attrset(A_NORMAL);
	getch();
	clear(); // Bildschirm löschen
	curs_set(0);
}

// --------------------------------------
// Implementation des Mathematik-Bereichs
// --------------------------------------


// Implementation x-seitiger Würfel
int wuerfel(unsigned int maximalzahl) {
	// aus z.B. 0..5 wird 1..6
	return (rand() % (maximalzahl - 1)) + 1;
}

// Initialisierung der Zufallszahlen
void zufall_per_zeit(void) {
  time_t jetzt;
  jetzt = time((time_t *) NULL);
  srand((unsigned) jetzt);
}

// --------------------------------
// Implementation des nstr-Bereichs
// --------------------------------


// Function: nstrnew
// Implementation: This function creates the memorylocation for a new nstr - and adds an \0 to the nstr
// Returns: pointer on the nstr structure
nstr *nstrnew(const char *t) {
	nstr *r = malloc(sizeof(nstr));
    if(!r) // NULL-Zeiger?
        return(r); // Abbruch - und NULL-Zeiger zurückgeben
    r->len = strlen(t) + 1; // Länge von t + 1 für das abschließende \0
	char *n = malloc(r->len * sizeof(char));
    if(!n) {        // Kein Speicherplatz für Zeichenkette - Abbruch
        r == NULL;  // r zum NULL-Pointer ändern
        return(r);
    }
	n = strncpy(n, t,  r->len - 1);
	n[r->len - 1] = '\0'; // Jeden String mit einem \0 abschließen
	r->str = n;
	return(r);
}

// Function nstrdel
// Implementation free the memory from the nstr construction
// Returns: true for full success - or false for partial or complete failure
bool nstrdel(nstr *t) {
	if(!t)	// NULL-Pointer-Behandlung
		return false;
	if(!t->str) {	// Zeiger ist verlorengegangen
		free(t); // Struktur befreien und Fehler mitteilen
		return false;
	}
	free(t->str); // Normale Behandlung
	free(t);
	return true;
}

// Function nstradd
// Implementation of a function,  that adds more chars to a existing nstr
// Returns: pointer to the new nstr
nstr *nstradd(nstr *t,  const char *c) {
	int l = strlen(c);
	char *cp;
	
    cp = realloc(t->str, t->len + l);
	if(!cp) {
		fputs("Fehler in Funktion nstradd(nstr *,  const char *), Bibliothek nstr.c: Reallokation von t endete in einem NULL-Zeiger, es konnte also kein Speicher alloziert werden!\n",  stderr);
		return(t);
	}
    // Zuweisen des neuen nstr und kopieren des alten Inhalts
	t->str = cp;
	t->str = strncat(t->str, c, l);
	t->str[t->len + l - 1] = '\0';
	t->len += l;
	return(t);
}

// Function nstrcmp
// Implementation of a function to compare 2 given strings
// return: negativ if s1 < s2,  zero if s1  == s2,  positive if s1 > s2
int nstrcmp(const nstr *s1, const nstr *s2) {
	return(strcmp(s1->str, s2->str));
}

// Function nstrcoll
// Implementation of a function to compare 2 given strings by LC_COLLATE
// return: negativ if s1 < s2,  zero if s1  == s2,  positive if s1 > s2
int nstrcoll(const nstr *s1, const nstr *s2) {
	return(strcoll(s1->str, s2->str));
}

// Function nstrset
// Implementation of a function,  that set's a new set of chars to an existing nstr
// Returns: pointer to the new nstr
nstr *nstrset(nstr *t,  const char *c) {
	int l = strlen(c);
	char *cp;
	// Reallocates the memory
	// If there isn't enough memory anymore, we get a NULL pointer
	cp = realloc(t->str,  l + 1);
	if(!cp) {
		fputs("Fehler in Funktion nstrset(nstr *,  const char *), Bibliothek nstr.c: Reallokation von t endete in einem NULL-Zeiger, es konnte also kein Speicher alloziert werden!\n",  stderr);
		return(t);
	}
    // Zuweisen des neuen nstr und kopieren des neuen Inhalts
	t->str = cp;
	t->str = strncpy(t->str, c, l);
	t->str[l] = '\0';
	t->len = l + 1;
	return(t);
}

// Function nstrpbrk
// Implementation of a function, that returns the pointer the first found char of a list,  or NULL
// Returns: pointer to first found char or NULL
char *nstrpbrk(nstr *t, const char *searchchars) {
	return(strpbrk(t->str, searchchars));
}

// Function nstrrchr
// Implementation of a function, that returns the pointer to the last found char,  or NULL
// Returns: pointer to last found char or NULL
char *nstrrchr(nstr *t, const int searchchar) {
	return(strrchr(t->str, searchchar));
}

// --------------------------------
// Implementation des narr-Bereichs
// --------------------------------


// Function narrnew
// Implementation: This function creates an array of strings
// returns: pointer on the stringarraystructure
narr *narrnew(const unsigned int n) {
	narr *r = malloc(sizeof(narr));
	if(!r) {
		fputs("Fehler: r konnte nicht erstellt werden in Funktion narrnew, Bibliothek nstr.c\n",  stderr);
		return(r);
	}
	// That was the arraystructure - now to the array of elements
	r->elm = malloc(sizeof(nstr) * n);
	if(!r) {
		fputs("Fehler: r->elm[] konnte nicht erstellt werden in Funktion narrnew, Bibliothek nstr.c\n",  stderr);
		return(r);
	}
	r->cnt = n;
	// Now initialise all the stringelements
	for(unsigned int i = 0;  i < n;  ++i) {
		r->elm[i] = nstrnew("");
		if(!r->elm[i]) {
			fprintf(stderr, "Fehler: r->elm[%d] konnte nicht erstellt werden in Funktion narrnew, Bibliothek nstr.c\n", i);
			return(r);
		}
	}
	// All done!
	return(r);
}

// Function narrdel
// Implementation: This function deletes a complete narr
// returns: true if all went well and false something went wrong.
bool narrdel(narr *t) {
	bool ok = true; // Hält fest, ob alles glatt gegangen ist
	if(!t) // NULL-Zeiger = Abbruch
		return false;
	// freeing all the substrings
	for(unsigned int i=0;  i < t->cnt;  ++i)
		if(!nstrdel(t->elm[i]))
				ok = false;
	// and last freeing the t itself
	free(t);
	// done - now return if there have been any problems
	return(ok);
}

// Function narradd
// Implementation: This function adds some strings to a given narr
// returns: pointer on the stringarraystructure or NULL-Pointer(busted)
narr *narradd(narr *t,  const unsigned int n) {
	nstr **cp; // for realloc

	// Ist der geforderte Index größer als die Anzahl an Elementen?
	if((t->cnt + n) < (t->cnt || n)) {
		fputs("Fehler: n ist größer als die mögliche Anzahl an Elementen in Funktion narradd, Bibliothek nstr.c\n",  stdout);
		return(t);
	}
	// Speicher neu zuweisen
	cp = realloc(t->elm, t->cnt * sizeof(nstr));
	if(!cp) {
		fputs("Fehler: cp ist ein NULL-Zeiger in Funktion narradd, Bibliothek nstr.c\n",  stderr);
		return(t);
	}
	t->elm = cp;
	// Now initialise all the stringelements
	for(unsigned int i = t->cnt;  i < (t->cnt + n);  ++i) {
		t->elm[i] = nstrnew("");
		if(!t->elm[i]) {
			fprintf(stderr, "Fehler: t->elm[%d] konnte nicht erstellt werden in Funktion narradd, Bibliothek nstr.c\n", i);
			return(t);
		}
	}
	// Ah,  all went well! Adding the new strings
	t->cnt += n;
	// All done!
	return(t);
}

// Function narrrmv
// Implementation: This function removes a nstr from a given narr
// returns: bool true if all is okay or false if an error occured
bool narrrmv(narr *t,  const unsigned int n) {
	nstr *cp;

	if(n > t->cnt) {
			fprintf(stderr, "Fehler: n to remove from t was outside the stringarrays range in function strinarrayremove,  library nstringarray.c\n");
			return(false);
	}
	cp = t->elm[n];
	for(int i=n;   i < (t->cnt - 1); ++i)
		t->elm[i] = t->elm[i+1];
	if(!nstrdel(cp)) {
			fprintf(stderr, "Error: Couldn't remove nstr from safetypt in function strinarrayremove,  library nstringarray.c\n");
			return(false);
	}
	t->cnt -= 1;
	return(true);
}

