
/*
 *	cldir2.c
 *	(c) 2020, Brian Stephenson
 *	brian@bstephen.me.uk
 *
 *	A program to test orthodox chess problems of the types:
 *
 *		directmates
 *		selfmates
 *		relfexmates
 *		helpmates
 *
 *	Input is taken from the program options and output is xml on stdout.
 *
 *	This is the module for classifying directmates in two.
 */

#include "sengine.h"
#include "utarray.h"
#include "utstring.h"

void setup_id_board(BOARD*, ID_BOARD*);

void start_class_2_xml(void);
void end_class_2_xml(void);
void start_static_class_xml(void);
void end_static_clas_xml(void);
void start_set_class_xml(void);
void end_set_class_xml(void);
void start_virtual_class_xml(void);
void end_virtual_class_xml(void);
void start_actual_class_xml(void);
void end_actual_class_xml(void);
void add_static_type(char*);
void add_added(int);
void add_removed(int);
void add_changed(int);
void add_up_flights(int);
void add_up_checks(int);
void add_up_fgivers(int);
void add_up_caps(int);
void add_tot_up(int);
void add_var(char*);
void start_try();
void end_try();
void add_key(char*);
void add_threat(char*);
void add_refut(char*);
char get_piece_type(enum COLOUR, BOARD*, unsigned char);
void update_id_board(enum COLOUR, BOARD*, ID_BOARD*, ID_BOARD*);
UT_string* get_mate_class(BOARD*, BOARD*, ID_BOARD*);
void classify_white_move(BOARD*, BOARD*, ID_BOARD*, ID_BOARD*);
void classify_vars(BOARDLIST*, BOARDLIST*, BOARD*, ID_BOARD*);

static void do_statics(DIR_SOL*, BOARD*);
static void do_sets(DIR_SOL*, BOARD*, ID_BOARD*);
static void do_virtual(DIR_SOL*, BOARD*, ID_BOARD*);
static void do_actual(DIR_SOL*, BOARD*, ID_BOARD*);
static int get_set_size(DIR_SOL*);
static void get_changed_and_removed_mates(DIR_SOL*);
static void get_added_mates(DIR_SOL*);
static bool mate_equals(BOARD*, BOARD*, BOARD*);
static bool black_equals(BOARD*, BOARD*);
static bool is_flight_giver(BOARD*, unsigned int);
static bool is_provided(BOARD*, BOARDLIST*);
static void classify_threats(BOARD*, BOARDLIST*, ID_BOARD*);

static int ChangedMates = 0;
static int AddedMates = 0;
static int RemovedMates = 0;
static int UpFlights = 0;
static int UpChecks = 0;
static int UpFgivers = 0;
static int UpCaps = 0;
static int TotUp = 0;
char pieces[] = "0PSBRQK";
char* lab_check = "CHECK";
char* lab_kcast = "0-0";
char* lab_qcast = "0-0-0";
char* lab_p_flight = "P-FLIGHT";
char* lab_s_flight = "S_FLIGHT";

int featsort(const void* a, const void* b)
{
    char** aa = (char**) a;
    char** bb = (char**) b;

#ifdef MOVESTAT
    fprintf(stderr, "%s\n%s\n\n", *aa, *bb);
#endif

    return strcmp(*aa, *bb);
}

void class_direct_2(DIR_SOL* insol, BOARD* inBrd)
{
    ID_BOARD* p_init_idbd;

    start_class_2_xml();

    do_statics(insol, inBrd);

    p_init_idbd = getIdBoard();
    setup_id_board(inBrd, p_init_idbd);

#ifdef MOVESTAT
    fprintf(stderr, "WHITE_ID => %s\n", p_init_idbd->white_ids);
    fprintf(stderr, "BLACK_ID => %s\n", p_init_idbd->black_ids);
#endif

    if (inBrd->check == false) {
        do_sets(insol, inBrd, p_init_idbd);
    }

    do_virtual(insol, inBrd, p_init_idbd);
    do_actual(insol, inBrd, p_init_idbd);

    freeIdBoard(p_init_idbd);

    end_class_2_xml();

    return;
}

void do_statics(DIR_SOL* insol, BOARD* inBrd)
{
#ifndef NDEBUG
    fputs("do_statics()\n", stderr);
#endif
    BOARDLIST* bList;
    unsigned int flights;
    int set_count;
    int set_full_count;
    bool set_complete = false;

    start_static_class_xml();

    if (inBrd->check == false) {
        bList = generateBlackBoardlist(inBrd, 1, &flights);

        if (insol->set->vektor != NULL) {
            BOARD* elt;

            if (bList->vektor != NULL) {
                DL_COUNT(insol->set->vektor, elt, set_count);
                DL_COUNT(bList->vektor, elt, set_full_count);

                set_complete = (set_count == set_full_count) ? true : false;
            }
        }
    }

    /*
      * Classify type
      */

    if (inBrd->check == false) {
        get_changed_and_removed_mates(insol);
        get_added_mates(insol);
        int SetSize = get_set_size(insol);

        add_changed(ChangedMates);
        add_added(AddedMates);
        add_removed(RemovedMates);

        if (insol->keys->vektor->check == true) {
            add_static_type("CHECKING_KEY");
        } else if (insol->keys->vektor->threat == NULL) {

            if ((SetSize > 0) && (set_complete == true)) {

                if (ChangedMates > 0) {
                    add_static_type("MUTATE");
                } else {
                    add_static_type("WAITER");
                }
            } else {
                add_static_type("INCOMPLETE_BLOCK");
            }
        } else {
            if ((SetSize > 0) && (set_complete == true)) {
                add_static_type("BLOCK_THREAT");
            } else {
                add_static_type("THREAT");
            }
        }

        /*
        	* Count strong unprovided moves.
        	*/
        {
            BOARD* elt;
            BOARDLIST* sets = insol->set;

            DL_FOREACH(bList->vektor, elt) {

                if (elt->mover == KING) {
                    if (is_provided(elt, sets) == false) {
                        UpFlights++;
                        TotUp++;
                    }
                } else if (elt->check == true) {
                    if (is_provided(elt, sets) == false) {
                        UpChecks++;
                        TotUp++;
                    }
                } else if (elt->captured == true) {
                    if (is_provided(elt, sets) == false) {
                        UpCaps++;
                        TotUp++;
                    }
                } else if (is_flight_giver(elt, flights) == true) {
                    if (is_provided(elt, sets) == false) {
                        UpFgivers++;
                        TotUp++;
                    }
                }
            }
        }

        freeBoardlist(bList);

    } else {
        add_static_type("WHITE_IN_CHECK");
    }

    add_up_flights(UpFlights);
    add_up_checks(UpChecks);
    add_up_fgivers(UpFgivers);
    add_up_caps(UpCaps);
    add_tot_up(TotUp);

    end_static_clas_xml();


    return;
}

bool is_provided(BOARD* inBrd, BOARDLIST* sets)
{
    BOARD* elt;
    bool rc = false;

    DL_FOREACH(sets->vektor, elt) {

        if ((inBrd->from == elt->from) && (inBrd->to == elt->to)) {
            rc = true;
            break;
        }
    }

    return rc;
}

bool is_flight_giver(BOARD* inBrd, unsigned int inFlights)
{
    unsigned int flights;

    BOARDLIST* bList = generateBlackBoardlist(inBrd, 1, &flights);
    freeBoardlist(bList);

    return (flights > inFlights) ? true : false;
}

int get_set_size(DIR_SOL* insol)
{
    BOARD* elt;
    int rc = 0;

    if (insol->set->vektor != NULL) {
        DL_COUNT(insol->set->vektor, elt, rc);
    }

    return rc;
}

void get_changed_and_removed_mates(DIR_SOL* insol)
{
    // Iterate round set variations counting changed and removed.
    // Only inspect variations that are dual-free in set and actual.
    // A mate by the key-piece going to the same square as in the set play is not a change.

    if (insol->set->vektor != NULL) {
        BOARDLIST* defences = insol->keys->vektor->nextply;
        BOARD* setDefence;

        DL_FOREACH(insol->set->vektor, setDefence) {
            int mateCount;
            BOARD* tp;
            BOARDLIST* setMates = setDefence->nextply;
            DL_COUNT(setMates->vektor, tp, mateCount);

            if (mateCount == 1) {
                bool found = false;
                BOARD* setMate = setMates->vektor;
                BOARD* defence;

                DL_FOREACH(defences->vektor, defence) {

                    if (black_equals(setDefence, defence) == true) {
                        int aMateCount;
                        BOARDLIST* actualMates = defence->nextply;
                        BOARD* tp1;
                        DL_COUNT(actualMates->vektor, tp1, aMateCount);

                        if (aMateCount == 1) {
                            found = true;

                            if (mate_equals(insol->keys->vektor, setMate, actualMates->vektor) == false) {
                                ChangedMates++;
                            }

                            break;
                        }
                    }
                }

                if (found == false) {
                    RemovedMates++;
                }
            }
        }
    }

    return;
}

void get_added_mates(DIR_SOL* insol)
{
    // Iterate round actual variations counting added mates.
    // Inspect dual-free variations only.

    BOARDLIST* defences = insol->keys->vektor->nextply;
    BOARDLIST* setVars = insol->set;
    BOARD* defence;

    DL_FOREACH(defences->vektor, defence) {
        int MateCount;
        BOARD* tp1;
        BOARDLIST* mates = defence->nextply;
        DL_COUNT(mates->vektor, tp1, MateCount);

        if (MateCount == 1) {
            bool found = false;
            BOARD* setVar;

            DL_FOREACH(setVars->vektor, setVar) {

                if (black_equals(defence, setVar) == true) {
                    int ccount;
                    BOARD* tp2;
                    BOARDLIST* setMates = setVar->nextply;
                    DL_COUNT(setMates->vektor, tp2, ccount);

                    if (ccount == 1) {
                        found = true;
                        break;
                    }
                }
            }

            if (found == false) {
                AddedMates++;
            }
        }
    }

    return;
}

void do_sets(DIR_SOL* insol, BOARD* inBrd, ID_BOARD* in_Idb)
{
    BOARDLIST* wlist;

#ifndef NDEBUG
    fputs("do_sets()\n", stderr);
#endif
    start_set_class_xml();
    wlist = generateWhiteBoardlist(inBrd, 1);
    classify_vars(wlist, insol->set, inBrd, in_Idb);
    freeBoardlist(wlist);
    end_set_class_xml();

    return;
}

void do_virtual(DIR_SOL* insol, BOARD* inBrd, ID_BOARD* in_Idb)
{
    BOARDLIST* wlist;

#ifndef NDEBUG
    fputs("do_virtual()\n", stderr);
#endif
    BOARDLIST* tries = insol->tries;
    BOARD* elt;

    start_virtual_class_xml();

    DL_FOREACH(tries->vektor, elt) {
        start_try();
        ID_BOARD* newIB = cloneIdBoard(in_Idb);
        update_id_board(WHITE, elt, in_Idb, newIB);
        classify_white_move(inBrd, elt, in_Idb, newIB);

        if (elt->threat != NULL) {
            classify_threats(elt, elt->threat, newIB);
        }

        wlist = generateWhiteBoardlist(elt, 1);
        classify_vars(wlist, elt->nextply, elt, newIB);
        freeBoardlist(wlist);
        freeIdBoard(newIB);
        end_try();
    }

    end_virtual_class_xml();

    return;
}

void do_actual(DIR_SOL* insol, BOARD* inBrd, ID_BOARD* in_Idb)
{
    BOARDLIST* wlist;
#ifndef NDEBUG
    fputs("do_actual()\n", stderr);
#endif
    BOARD* wkey = insol->keys->vektor;
    start_actual_class_xml();
    ID_BOARD* newIB = cloneIdBoard(in_Idb);
    update_id_board(WHITE, wkey, in_Idb, newIB);
    classify_white_move(inBrd, wkey, in_Idb, newIB);

    if (wkey->threat != NULL) {
        classify_threats(wkey, wkey->threat, newIB);
    }

    wlist = generateWhiteBoardlist(wkey, 1);
    classify_vars(wlist, wkey->nextply, wkey, newIB);

    freeBoardlist(wlist);
    freeIdBoard(newIB);
    end_actual_class_xml();

    return;
}

void classify_threats(BOARD* initBrd, BOARDLIST* threats, ID_BOARD* in_Idb)
{
#ifndef NDEBUG
    fputs("classify_threats()\n", stderr);
#endif
    unsigned int count;
    BOARD* elt;

    DL_COUNT(threats->vektor, elt, count);

    if (count == 2) {
        // Treat promotion to Q/R or Q/B as non dual.
        BOARD* m1 = threats->vektor;
        BOARD* m2 = threats->vektor->next;

        if ((m1->mover == PAWN) && (m2->mover == PAWN)) {
            if ((m1->from == m2->from) && (m1->to == m2->to)) {
                if ((m1->promotion == QUEEN) && ((m2->promotion == ROOK) || (m2->promotion == BISHOP))) {
                    // Allowable dual - delete underpromotion.
                    DL_DELETE(m1, m2);
                    count--;
                }
            }
        }
    }

    if (count == 1) {
        UT_string* s = get_mate_class(initBrd, threats->vektor, in_Idb);
        add_threat(utstring_body(s));
        utstring_free(s);
    } else {
        UT_string* d;
        utstring_new(d);
        utstring_printf(d, "DUALS(%u)", count);
        add_threat(utstring_body(d));
        utstring_free(d);
    }

    return;
}

bool mate_equals(BOARD* key, BOARD* setmate, BOARD* actualmate)
{
    if (setmate->mover != actualmate->mover) return false;

    if (setmate->to != actualmate->to) return false;

    if ((setmate->from != actualmate->from) && (key->to != actualmate->from)) return false;

    if (setmate->promotion != actualmate->promotion) return false;

    return true;
}

bool black_equals(BOARD* brda, BOARD* brdb)
{
    if (brda->mover != brdb->mover) return false;

    if (brda->from != brdb->from) return false;

    if (brda->to != brdb->to) return false;

    if (brda->promotion != brdb->promotion) return false;

    return true;
}


