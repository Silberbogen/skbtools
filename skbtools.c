/*
 * =====================================================================================
 *
 *       Filename:  skbtools.c
 *
 *    Description:  saschakb tools - Sammlung für ncurses-Anwendungen
 *
 *        Version:  0.003
 *    letzte Beta:  0.000
 *        Created:  14.09.2011 11:42:00
 *          Ended:  00.00.0000 00:00:00
 *       Revision:  none
 *       Compiler:  c99/c11
 *        compile:  make
 *
 *         Author:  Sascha K. Biermanns (skbierm), skbierm@gmail.com
 *        Company:
 *        License:  ISC
 *
 *   Copyright (C)  2011-2016, Sascha K. Biermanns
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
 *                Es wurden die Funktionen
 *                - nstring()
 *                - nstrlen()
 *                - nstrlencorr
 *                - nstrcorr()
 *                neu aufgenommen
 *   - 17.09.2011 Textausgabe wurde vereinfacht
 *                Diverse Funktionen haben keine vordefinierte Farbwahl mehr
 *   - 14.11.2012 Vereinfachung bei Strukturnamen, kleine Korrekturen
 *   - 19.11.2012 Kleine Überarbeitung der Routinen, enum Konstanten korrigiert
 *   - 20.11.2012 malloc, sizeof und return werden aufgeräumt
 *
 * =====================================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h> // uint8_t
#include <time.h> // Zufallsgenerator
#include <string.h>
#include <ncurses.h> // Farbige Grafische Ausgabe
#include <locale.h>
#include <stdarg.h> // Für die VA-Liste
#include <ctype.h> // toupper
#include "skbtools.h" // Die Modul-Beschreibung

static enum farben vfarbe = FARBE_WEISS; // Vordergrundfarbe
static enum farben hfarbe = FARBE_SCHWARZ; // Hintergrundfarbe

// -----------
// Algorithmen
// -----------

// swap - ein generischer swap-Algorithmus
void swap(void *va /* vektor object a */,
		  void *vb /* vektor object b */,
		  size_t i /* length of the objects */ )
{
	uint8_t temp;		// address for buffering one byte
	uint8_t * a = va;	// bytewise moving pointer to va
	uint8_t * b = vb;	// bytewise moving pointer to vb
	// Counting down from the length of the objects to 1
	// Reaching zero stops the while loop, all addresses
	// copied one to one.
	while ( i-- ) temp = a[i], a[i] = b[i], b[i] = temp;
}

// -----------------------------------
// Implementation des ncurses-Bereichs
// -----------------------------------


// Funktion: Beenden mit farbiger Statusmeldung
void beenden(enum farben f, int status, char * text, ...) {
    // Reservierung für den maximalen Speicherplatz, den rest benötigt
    // -------------------------------------------------------------------
    char * cp = malloc( (sizeof text + 100) * sizeof text[0]);
    if ( !cp ) {
        vordergrundfarbe(FARBE_ROT);
        printw("Fehler!\nSicherheitszeiger in beenden() erhielt keinen Speicher!\n");
        vordergrundfarbe(FARBE_WEISS);
        exit(EXIT_FAILURE);
    }
    char *umgewandeltertext = cp;
    // Verarbeitung der Parameterliste und Umwandlung der Parameter + Text zu einem String
    // -----------------------------------------------------------------------------------
    va_list par; // Parameterliste
    va_start(par, text);
    vsprintf(umgewandeltertext, text, par);
    va_end(par);
    // -----------------------------------------------------------------------------------   
    hinweis(f, umgewandeltertext);
    // cp löschen, sonst gibt es üble Speicherlöcher ;)
    if ( !cp ) free(cp);
    exit(status);
}

// Funktion: Hintergrundfarbe ändern
void hintergrundfarbe(enum farben hf) {
  hfarbe = hf;
  color_set((8 * vfarbe) + hfarbe + 1, 0);
}

// Funktion: Hinweis - für Fehlermeldungen oder ähnliches
void hinweis(enum farben f, char * text, ...) {
    // Reservierung für den maximalen Speicherplatz, den rest benötigt
    // -------------------------------------------------------------------
    char * cp = malloc( (sizeof text  + 100) * sizeof text[0]);
    if ( !cp ) {
        vordergrundfarbe(FARBE_ROT);
        printw("Fehler!\nsicherheitszeiger in hinweis() erhielt keinen Speicher!\n");
        vordergrundfarbe(FARBE_WEISS);
        exit(EXIT_FAILURE);
    }
    char *umgewandeltertext = cp;
    // Verarbeitung der Parameterliste und Umwandlung der Parameter + Text zu einem String
    // -----------------------------------------------------------------------------------
    va_list par; // Parameterliste
    va_start(par, text);
    vsprintf(umgewandeltertext, text, par);
    va_end(par);
    // -----------------------------------------------------------------------------------
    vordergrundfarbe(f);
    textausgabe(umgewandeltertext);
    weiter();
    // cp löschen, sonst gibt es üble Speicherlöcher ;)
    if ( !cp ) free(cp);
}

// Implementation: Ja-Nein-Frage
bool janeinfrage(char * frage) {
    textausgabe(frage);
	char eingabe = taste();
	return toupper( eingabe ) == 'J' ? true : false;
}

// Initialisierung der ncurses-Umgebung
void ncurses_init( void (*funktion)() ) {	
	setlocale(LC_ALL, ""); // Umgebungsvariablen setzen
	initscr(); // beginne ncurses
	keypad(stdscr, true); // Keymapping aktivieren
	cbreak(); // kein Warten bei der Eingabe auf ENTER
	echo(); // Cursort-Echo
	scrollok(stdscr, true); // Automatisches Scrollen aktivieren
	start_color(); // Beginne mit Farben
	// Initialisierung aller Farbpaare
	for(int x = FARBE_SCHWARZ; x <= FARBE_WEISS; ++x)
		for(int y = FARBE_SCHWARZ; y <= FARBE_WEISS; ++y)
			init_pair((8 * x) + y + 1, x, y);
	// Standardfarben setzen
   vordergrundfarbe(FARBE_WEISS);
   hintergrundfarbe(FARBE_SCHWARZ);
	clear(); // Bildschirm löschen
	curs_set(0); // Cursor in die linke obere Ecke
	atexit( funktion ); // Routine, die bei der Beendung ausgeführt wird
}

// Implementation: Taste
int taste(void) {
	noecho(); // Cursorecho ausschalten
	int zeichen = getch(); // Taste holen
	echo(); // Cursorecho einschalten
	return zeichen;
}


// Implementation: Textausgabe
void textausgabe(char * t, ...) {
    // Reservierung für den maximalen Speicherplatz, den rest benötigt
    // -------------------------------------------------------------------
    char *cp = malloc( (sizeof t + 100) * sizeof(t[0]));
    if ( !cp ) {
        vordergrundfarbe(FARBE_ROT);
        printw("Fehler!\ncp in textausgabe() erhielt keinen Speicher!\n");
        exit( EXIT_FAILURE );
    }
    char *rest = cp;
    // -------------------------------------------------------------------
    // Verarbeitung der Parameterliste und Umwandlung der Parameter + Text zu einem String
    // -----------------------------------------------------------------------------------
    va_list par; // Parameterliste
    va_start(par, t);
    vsprintf(rest, t, par);
    va_end(par);
    // -----------------------------------------------------------------------------------
	char textzeile[COLS]; // Ausgabezeile	
	bool erstausgabe = true; // Wir haben bisher noch nichts ausgegeben
	int zeile = 0; // Damit wir immer wissen, in welcher Zeile wir gerade sind	
	// Zuerst eine Sicherheitslöschung, sonst gibt es Fehler bei der Leerzeilenausgabe
	for (int i = 0; i < COLS; ++i) textzeile[i] = '\0';
	// Solange der Text länger als eine Zeile ist, begeben wir uns in die while-Schleife
	while (strlen(rest) > (COLS - 1)) {
		int i, j; // Schleifenzähler
		for ( i = (COLS - 1); (rest[i] != ' ') && (i > 0); --i );
		if ( !i ) i = (COLS - 1); // Das Wort ist so länger als die verdammte Zeile
		for ( j = 0; (rest[j] != '\n') && (j < i); ++j );		
		if ( j < i ) i = j; // Auf das Zeilenendezeichen verkürzen
		strncpy(textzeile, rest, i); // Den Textteil kopieren
		rest += i+1;
		while ( *rest == ' ' ) ++rest;		
		// Prüfen, ob wir in der vorletzten Zeile angekommen sind
		if ( (erstausgabe) && ( getcury( stdscr ) >= (LINES - 1)) ) {
			weiter();
			zeile = 0;
			erstausgabe = false;
		} else if ( zeile == (LINES - 1) ) {
			weiter();
			zeile = 0;
		}		
		printw("%s\n", textzeile); // Ziel erreicht, wir können den Text ausgeben		
		// Sicherheitslöschung, sonst gibt es Fehler bei der Leerzeilenausgabe
		for ( i = 0; i < COLS; ++i ) textzeile[i] = '\0';
		zeile += 1;
	}	
	if ( zeile == (LINES - 1) ) { // Prüfen, ob wir in der vorletzten Zeile angekommen sind
			weiter();
			zeile = 0;
	}
	printw("%s\n", rest); // Der restliche Text ist kürzer als eine Zeile.	
	refresh();    
    if ( !cp ) free(cp); // cp löschen, sonst gibt es üble Speicherlöcher ;)
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
int waehle(char * beschreibung, int maxzahl) {
	int ergebnis = 0;
    while( ergebnis < 1 || ergebnis > maxzahl ) {
        textausgabe(beschreibung);
        char eingabe[20];    
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
	getch(); // Auf Tastendruck warten
	clear(); // Bildschirm löschen
	curs_set(0); // Cursor in die linke obere Ecke
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
  time_t jetzt = time((time_t *) NULL);
  srand((unsigned) jetzt);
}

// --------------------------------------------------------------
// Entfernt unnütze Tab- oder Newlinezeichen, z.B. für eine Suche
// --------------------------------------------------------------
char * str_strip(const char * s) {
    char * p = malloc( strlen(s) + 1);
    if ( p ) {
    	char * p2 = p;
    	while ( * s != '\0' )
			if ( * s != '\t' && * s != '\n' && * s != '\v' )
            *p2++ = *s++;
         else
            ++s;
    	*p2 = '\0';	
    }
    return p;
}

// --------------------------------
// Implementation des nstr_s-Bereichs
// --------------------------------

char * nstring(const nstr_s t) {
    return t.str;
}

int nstrlen(const nstr_s t) {
    return t.len;
}

bool nstrlencorr(nstr_s * t) {
    if(t->len != (strlen(t->str) + 1)) {
        t->len = strlen(t->str) + 1;
        return false;
    }
    return true;
}

bool nstrcorr(nstr_s * t) {
    if( t->len == (strlen(t->str) + 1) ) return true; // Alles okay
    if(t->len > (strlen(t->str) + 1)) { // Es ist Information verloren gegangen, die Länge wird gekürzt.
        t->len = strlen(t->str) + 1;
        return false;
    }
    t->str[t->len - 1] = '\0'; // \0 wird an der gespeicherten Position erzwungen
    t = nstrset(t, t->str); // Die Zeichenkette wird neu gesetzt, vermutlich an neuer Speicherstelle
    return false;
}

nstr_s * nstrnew(const char * t) {
	nstr_s * r = malloc( sizeof(nstr_s) );
    if ( !r ) return(r); // Abbruch - und NULL-Zeiger zurückgeben
    r->len = strlen(t) + 1; // Länge von t + 1 für das abschließende \0
	char * n = malloc( r->len * sizeof(char));
    if ( !n ) {        // Kein Speicherplatz für Zeichenkette - Abbruch
        r = NULL;  // r zum NULL-Pointer ändern
        return(r);
    }
	n = strncpy(n, t,  r->len - 1);
	n[r->len - 1] = '\0'; // Jeden String mit einem \0 abschließen
	r->str = n;
	return r;
}

bool nstrdel(nstr_s * t) {
	if ( !t ) return false; // NULL-Pointer-Behandlung
	if ( !t->str ) {	// Zeiger ist verlorengegangen
		free(t); // Struktur befreien und Fehler mitteilen
		return false;
	}
	free( t->str ); // Normale Behandlung
	free( t );
	return true;
}

nstr_s * nstradd( nstr_s * t,  const char * c) {
	int l = strlen(c);
	char *cp;
    cp = realloc( t->str, t->len + l );
	if ( !cp ) {
		fputs("Fehler in Funktion nstradd(nstr_s *,  const char *), Bibliothek nstr_s.c: Reallokation von t endete in einem NULL-Zeiger, es konnte also kein Speicher alloziert werden!\n",  stderr);
		return(t);
	}
    // Zuweisen des neuen nstr_s und kopieren des alten Inhalts
	t->str = cp;
	t->str = strncat( t->str, c, l );
	t->str[t->len + l - 1] = '\0';
	t->len += l;
	return t;
}

int nstrcmp( const nstr_s *s1, const nstr_s *s2 ) {
	return strcmp(s1->str, s2->str);
}

int nstrcoll( const nstr_s *s1, const nstr_s *s2 ) {
	return strcoll(s1->str, s2->str);
}

nstr_s * nstrset(nstr_s * t,  const char * c) {
	int l = strlen(c);
	char *cp;
	// Reallocates the memory
	// If there isn't enough memory anymore, we get a NULL pointer
	cp = realloc(t->str,  l + 1);
	if ( !cp ) {
		fputs("Fehler in Funktion nstrset(nstr_s *,  const char *), Bibliothek nstr_s.c: Reallokation von t endete in einem NULL-Zeiger, es konnte also kein Speicher alloziert werden!\n",  stderr);
		return(t);
	}
    // Zuweisen des neuen nstr_s und kopieren des neuen Inhalts
	t->str = cp;
	t->str = strncpy(t->str, c, l);
	t->str[l] = '\0';
	t->len = l + 1;
	return t;
}

char * nstrpbrk(nstr_s * t, const char * searchchars) {
	return strpbrk(t->str, searchchars);
}

char * nstrrchr( nstr_s * t, const int searchchar ) {
	return strrchr(t->str, searchchar);
}

// --------------------------------
// Implementation des narr_s-Bereichs
// --------------------------------

narr_s * narrnew(const unsigned int n) {
	narr_s *r = malloc( sizeof(narr_s) );
	if ( !r ) {
		fputs("Fehler: r konnte nicht erstellt werden in Funktion narrnew, Bibliothek nstr_s.c\n",  stderr);
		return r;
	}
	// That was the arraystructure - now to the array of elements
	r->elm = malloc( sizeof(nstr_s) * n);
	if ( !r ) {
		fputs("Fehler: r->elm[] konnte nicht erstellt werden in Funktion narrnew, Bibliothek nstr_s.c\n",  stderr);
		return r;
	}
	r->cnt = n;
	// Now initialise all the stringelements
	for(unsigned int i = 0;  i < n;  ++i) {
		r->elm[i] = nstrnew("");
		if ( !r->elm[i] ) {
			fprintf(stderr, "Fehler: r->elm[%d] konnte nicht erstellt werden in Funktion narrnew, Bibliothek nstr_s.c\n", i);
			return r;
		}
	}
	// All done!
	return r;
}

bool narrdel( narr_s * t ) {
	if ( !t ) return false; // NULL-Zeiger = Abbruch
	bool ok = true; // Hält fest, ob alles glatt gegangen ist
	// freeing all the substrings
	for ( unsigned int i=0;  i < t->cnt;  ++i )
		if ( !nstrdel(t->elm[i]) ) ok = false; 
	// and last freeing the t itself
	free( t );
	// done - now return if there have been any problems
	return ok;
}

narr_s * narradd( narr_s * t,  const unsigned int n ) {
	nstr_s **cp; // for realloc
	// Ist der geforderte Index größer als die Anzahl an Elementen?
	if ( (t->cnt + n) < (t->cnt || n) ) {
		fputs("Fehler: n ist größer als die mögliche Anzahl an Elementen in Funktion narradd, Bibliothek nstr_s.c\n",  stdout);
		return(t);
	}
	// Speicher neu zuweisen
	cp = realloc( t->elm, t->cnt * sizeof(nstr_s) );
	if ( !cp ) {
		fputs("Fehler: cp ist ein NULL-Zeiger in Funktion narradd, Bibliothek nstr_s.c\n",  stderr);
		return t;
	}
	t->elm = cp;
	// Now initialise all the stringelements
	for ( unsigned int i = t->cnt;  i < (t->cnt + n);  ++i ) {
		t->elm[i] = nstrnew("");
		if ( !t->elm[i] ) {
			fprintf(stderr, "Fehler: t->elm[%d] konnte nicht erstellt werden in Funktion narradd, Bibliothek nstr_s.c\n", i);
			return t;
		}
	}
	// Ah,  all went well! Adding the new strings
	t->cnt += n;
	// All done!
	return t;
}

bool narrrmv( narr_s * t,  const unsigned int n ) {
	nstr_s * cp;
	if ( n > t->cnt ) {
			fprintf(stderr, "Fehler: n to remove from t was outside the stringarrays range in function strinarrayremove,  library nstringarray.c\n");
			return false;
	}
	cp = t->elm[n];
	for ( int i=n;   i < (t->cnt - 1); ++i ) t->elm[i] = t->elm[i+1];
	if ( !nstrdel(cp) ) {
			fprintf(stderr, "Error: Couldn't remove nstr_s from safetypt in function strinarrayremove,  library nstringarray.c\n");
			return false;
	}
	t->cnt -= 1;
	return true;
}

