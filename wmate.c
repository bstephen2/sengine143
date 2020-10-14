/*
 *	wmate.c
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
 *	This is the module for classifying the mating moves.
 */

#include "sengine.h"
#include "utarray.h"
#include "utstring.h"

void get_check_square_list(enum COLOUR, BOARD*, CHECK_SQUARE_LIST*);
void do_queen(UT_string*, BOARD*, ID_BOARD*);
void do_king(UT_string*, BOARD*, ID_BOARD*);
void do_rook(UT_string*, BOARD*, ID_BOARD*);
void do_bishop(UT_string*, BOARD* inBrd, ID_BOARD*);
void do_knight(UT_string*, BOARD*, ID_BOARD*);
void do_pawn(UT_string*, BOARD*, ID_BOARD*);
char get_piece_type(enum COLOUR, BOARD*, unsigned char);

static CHECK_SQUARE_LIST* csl;
static char id;
static char cid;
static char pid;
static int bkpos;

UT_string* get_mate_class(BOARD* initBrd, BOARD* inBrd, ID_BOARD* idBrd)
{
#ifndef NDEBUG
    fputs("get_mate_class()\n", stderr);
#endif
    UT_string* s;

    csl = getCSL();
    utstring_new(s);
    id = idBrd->white_ids[inBrd->from];
    cid = idBrd->black_ids[inBrd->to];
    pid = get_piece_type(BLACK, initBrd, inBrd->to);
    bkpos = (int) inBrd->pos->kingsq[BLACK];
    get_check_square_list(WHITE, inBrd, csl);

    switch (inBrd->mover) {
    case QUEEN:
        do_queen(s, inBrd, idBrd);
        break;

    case KING:
        do_king(s, inBrd, idBrd);
        break;

    case ROOK:
        do_rook(s, inBrd, idBrd);
        break;

    case BISHOP:
        do_bishop(s, inBrd, idBrd);
        break;

    case KNIGHT:
        do_knight(s, inBrd, idBrd);
        break;

    case PAWN:
        do_pawn(s, inBrd, idBrd);
        break;

    default:
        break;
    }

    //([QRB]-[KRBSP])BAT(EP)*
    //([QRB]-[KRBSP])IND_BAT(EP)*
    //([QRBSP]-[QRBSP])DCHECK
    //[BS]
    //P(EP)*
    //R
    //R1

    freeCSL(csl);

    return s;
}

void do_queen(UT_string* s, BOARD* inBrd, ID_BOARD* idBrd)
{
    int qpos = (int) inBrd->to;
    int dist = (int) abs(bkpos - qpos);

    if ((dist == 1) || (dist == 8)) {
        utstring_printf(s, "QAR1(%c)", id);
    } else if ((dist == 2) || (dist == 16)) {
        utstring_printf(s, "QAR2(%c)", id);
    } else if ((dist == 7) || (dist == 9)) {
        utstring_printf(s, "QAB1(%c)", id);
    } else if (((dist % 7) == 0) || ((dist % 9) == 0)) {
        utstring_printf(s, "QAB(%c)", id);
    } else {
        utstring_printf(s, "QAR(%c)", id);
    }

    if (inBrd->captured == true) {
        utstring_printf(s, ",X%c(%c)", pid, cid);
    }

    return;
}

void do_king(UT_string* s, BOARD* inBrd, ID_BOARD* idBrd)
{
    assert(csl->count == 1);
    char bpid = idBrd->white_ids[csl->square[0]];

    if ((inBrd->from == 4) && (inBrd->to == 6)) {
        utstring_printf(s, "0-0");
    } else if ((inBrd->from == 4) && (inBrd->to == 2)) {
        utstring_printf(s, "0-0-0");
    } else if (csl->real_piece[0] == ROOK) {
        utstring_printf(s, "R(%c)+K(%c)", bpid, id);
    } else if (csl->real_piece[0] == BISHOP) {
        utstring_printf(s, "B(%c)+K(%c)", bpid, id);
    } else {
        // must be QUEEN
        if (csl->as_piece[0] == ROOK) {
            utstring_printf(s, "QAR(%c)+K(%c)", bpid, id);
        } else {
            // must be BISHOP
            utstring_printf(s, "QAB(%c)+K(%c)", bpid, id);
        }
    }

    if (inBrd->captured == true) {
        utstring_printf(s, ",X%c(%c)", pid, cid);
    }

    return;
}

void do_rook(UT_string* s, BOARD* inBrd, ID_BOARD* idBrd)
{
    int qpos = (int) inBrd->to;
    int dist = (int) abs(bkpos - qpos);

    if ((csl->count == 1) && (csl->real_piece[0] == ROOK)) {
        if ((dist == 1) || (dist == 8)) {
            utstring_printf(s, "R1(%c)", id);
        } else {
            utstring_printf(s, "R(%c)", id);
        }
    } else {
        // Battery
        char bpid = idBrd->white_ids[csl->square[0]];
        char* bp = (csl->real_piece[0] == QUEEN) ? "QAB" : "B";

        if (csl->count == 1) {
            utstring_printf(s, "%s(%c)+R(%c)", bp, bpid, id);
        } else {
            assert(csl->as_piece[0] == BISHOP);
            utstring_printf(s, "%s(%c)+R(%c),DCHECK", bp, bpid, id);
        }
    }

    if (inBrd->captured == true) {
        utstring_printf(s, ",X%c(%c)", pid, cid);
    }

    return;
}

void do_bishop(UT_string* s, BOARD* inBrd, ID_BOARD* idBrd)
{
    if ((csl->count == 1) && (csl->real_piece[0] == BISHOP)) {
        utstring_printf(s, "B(%c)", id);
    } else {
        // Battery

        if (csl->count == 1) {
            char bpid = idBrd->white_ids[csl->square[0]];
            char* bp = (csl->real_piece[0] == QUEEN) ? "QAR" : "R";
            utstring_printf(s, "%s(%c)+B(%c)", bp, bpid, id);
        } else {
            assert(csl - as_piece[1] == ROOK);
            char bpid = idBrd->white_ids[csl->square[1]];
            char* bp = (csl->real_piece[1] == QUEEN) ? "QAR" : "R";
            utstring_printf(s, "%s(%c)+B(%c),DCHECK", bp, bpid, id);
        }
    }

    if (inBrd->captured == true) {
        utstring_printf(s, ",X%c(%c)", pid, cid);
    }

    return;
}

void do_knight(UT_string* s, BOARD* inBrd, ID_BOARD* idBrd)
{
    if ((csl->count == 1) && (csl->real_piece[0] == KNIGHT)) {
        utstring_printf(s, "S(%c)", id);
    } else {
        // Battery.
        if (csl->count == 1) {
            // Check must be from rear-piece.
            char bpid = idBrd->white_ids[csl->square[0]];

            if (csl->as_piece[0] == ROOK) {

                if (csl->real_piece[0] == ROOK) {
                    utstring_printf(s, "R(%c)+S(%c)", bpid, id);
                } else {
                    assert(cst->read_piece[0] == QUEEN);
                    utstring_printf(s, "QAR(%c)+S(%c)", bpid, id);
                }
            } else {
                assert(csl->as_piece[0] == BISHOP);

                if (csl->real_piece[0] == BISHOP) {
                    utstring_printf(s, "B(%c)+S(%c)", bpid, id);
                } else {
                    assert(cst->read_piece[0] == QUEEN);
                    utstring_printf(s, "QAB(%c)+S(%c)", bpid, id);
                }
            }
        } else {
            // Must be double-check.
            assert(csl->count == 2);
            assert(csl->real_piece[0] == KNIGHT);
            char bpid = idBrd->white_ids[csl->square[1]];

            if (csl->as_piece[1] == ROOK) {

                if (csl->real_piece[1] == ROOK) {
                    utstring_printf(s, "R(%c)+S(%c),DCHECK", bpid, id);
                } else {
                    assert(cst->read_piece[1] == QUEEN);
                    utstring_printf(s, "QAR(%c)+S(%c),DCHECK", bpid, id);
                }
            } else {
                assert(csl->as_piece[1] == BISHOP);

                if (csl->real_piece[1] == BISHOP) {
                    utstring_printf(s, "B(%c)+S(%c),DCHECK", bpid, id);
                } else {
                    assert(cst->real_piece[1] == QUEEN);
                    utstring_printf(s, "QAB(%c)+S(%c),DCHECK", bpid, id);
                }
            }
        }
    }

    if (inBrd->captured == true) {
        utstring_printf(s, ",X%c(%c)", pid, cid);
    }

    return;
}

void do_pawn(UT_string* s, BOARD* inBrd, ID_BOARD* idBrd)
{
    if (inBrd->promotion == NOPIECE) {
        if ((csl->count == 1) && (csl->real_piece[0] == PAWN)) {
            // No battery
            utstring_printf(s, "P(%c)", id);
        } else {
            // Battery
            char bpid;
            char* bp;

            if (csl->count == 1) {
                bpid = idBrd->white_ids[csl->square[0]];

                if (csl->as_piece[0] == BISHOP) {
                    bp = (csl->real_piece[0] == QUEEN) ? "QAB" : "B";
                    utstring_printf(s, "%s(%c)+P(%c)", bp, bpid, id);
                } else {
                    assert(csl->as_piece[0] == ROOK);
                    bp = (csl->real_piece[0] == QUEEN) ? "QAR" : "R";
                    utstring_printf(s, "%s(%c)+P(%c)", bp, bpid, id);
                }

            } else {
                bpid = idBrd->white_ids[csl->square[1]];

                if (csl->as_piece[1] == BISHOP) {
                    bp = (csl->real_piece[1] == QUEEN) ? "QAB" : "B";
                    utstring_printf(s, "%s(%c)+P(%c)DCHECK", bp, bpid, id);
                } else {
                    assert(csl->as_piece[1] == ROOK);
                    bp = (csl->real_piece[1] == QUEEN) ? "QAR" : "R";
                    utstring_printf(s, "%s(%c)+P(%c)DCHECK", bp, bpid, id);
                }
            }
        }

        if (inBrd->captured == true) {
            utstring_printf(s, ",X%c(%c)", pid, cid);

            if (inBrd->ep == true) {
                utstring_printf(s, ",EP");
            }
        }
    } else {

        char prom;

        utstring_printf(s, "P");

        if (inBrd->captured == true) {
            utstring_printf(s, "X%c(%c)", pid, cid);
        }

        assert((inBrd->promotion == QUEEN) || (inBrd->promotion == KNIGHT));
        prom = (inBrd->promotion == QUEEN) ? 'Q' : 'S';
        utstring_printf(s, "=%c", prom);
    }

    return;
}




