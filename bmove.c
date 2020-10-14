
/*
 *	bmove.c
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
 *	This is the module for classifying black moves.
 */

#include "sengine.h"
#include "utarray.h"
#include "utstring.h"

extern char pieces[];
extern char* lab_check;
extern char* lab_kcast;
extern char* lab_qcast;
extern char* lab_p_flight;
extern char* lab_s_flight;
extern BITBOARD setMask[64];

char* sblock = "SBLOCK";
char* ogate = "OGATE";
char* ogateb = "OGATEB";
char* nguard = "N_GUARD";
char* scut = "S_CUT";

int featsort(const void* a, const void* b);
char get_piece_type(enum COLOUR, BOARD*, unsigned char);
void update_id_board(enum COLOUR, BOARD*, ID_BOARD*, ID_BOARD*);
UT_string* get_mate_class(BOARD*, BOARD*, ID_BOARD*);
void add_var(char*);
void add_refut(char*);
bool attacks(POSITION*, unsigned char, enum COLOUR);
int count_checks(enum COLOUR, BOARD*);

void classify_vars(BOARDLIST* wlist, BOARDLIST* blist, BOARD* inBrd, ID_BOARD* inIdBrd)
{
#ifndef NDEBUG
    fputs("classify_vars()\n", stderr);
#endif

    assert(bList != NULL);
    assert(inBrd != NULL);
    assert(inIdBrd != NULL);
    assert(wlist != NULL);

    BOARD* elt;
    HASH_VAR* vars = NULL;
    HASH_VAR* s;
    HASH_VAR* tmp;
    char captured_piece = ' ';
    char captured_id = ' ';
    UT_string* refut;
    utstring_new(refut);

    DL_FOREACH(blist->vektor, elt) {
        unsigned int mates;
        BOARD* m;

        if (elt->nextply != NULL) {
            DL_COUNT(elt->nextply->vektor, m, mates);

            if (mates == 2) {
                // Treat promotion to Q/R or Q/B as non dual.
                BOARDLIST* bl = elt->nextply;
                BOARD* m1 = bl->vektor;
                BOARD* m2 = bl->vektor->next;

                if ((m1->mover == PAWN) && (m2->mover == PAWN)) {
                    if ((m1->from == m2->from) && (m1->to == m2->to)) {
                        if ((m1->promotion == QUEEN) && ((m2->promotion == ROOK) || (m2->promotion == BISHOP))) {
                            // Allowable dual - delete underpromotion.
                            DL_DELETE(m1, m2);
                            mates--;
                        }
                    }
                }
            }
        } else {
            mates = 0;
        }

        // Don't bother with dualled variations
        if (mates <= 1) {
#ifndef NDEBUG
            fprintf(stderr, "VAR: mates = %d\n", mates);
#endif
            UT_array* bfeats;
            UT_string* var;
            UT_string* bfeat;
            char** p;
            ID_BOARD* bmIdBoard = cloneIdBoard(inIdBrd);
            update_id_board(BLACK, elt, inIdBrd, bmIdBoard);
            utstring_new(var);
            utarray_new(bfeats, &ut_str_icd);

            // Identify black mover and add to var.

            char piece = pieces[elt->mover];
            char id = inIdBrd->black_ids[elt->from];
            utstring_printf(var, "%c(%c);", piece, id);

            // Identify black features
            utstring_new(bfeat);

            if (elt->check == true) {
                //CHECK
                utarray_push_back(bfeats, &lab_check);
            }

            if (elt->captured == true) {
                //XTURE[QRBSP](id)
                UT_string* capstr;
                utstring_new(capstr);
                captured_id = inIdBrd->white_ids[elt->to];
                captured_piece = get_piece_type(WHITE, inBrd, elt->to);
                utstring_printf(capstr, "X%c(%c)", captured_piece, captured_id);
                utarray_push_back(bfeats, &(utstring_body(capstr)));
                utstring_free(capstr);
            }

            PIN_STATUS* ps = get_pin_status();
            populate_pin_status(ps, inBrd, elt, inIdBrd, bmIdBoard);

            if (strcmp(utstring_body(ps->b_before), utstring_body(ps->b_after)) != 0) {
                char* haystack;
                char* needle;
                char temp;

                //P_SPIN([KQRBSP])
                haystack = utstring_body(ps->b_before);
                needle = utstring_body(ps->b_after);

                while (*needle != '\0') {
                    temp = *(needle + 2);
                    *(needle + 2) = '\0';

                    if (strstr(haystack, needle) == NULL) {
                        UT_string* p;
                        utstring_new(p);
                        utstring_printf(p, "P_SPIN%c(%c)", *needle, *(needle + 1));
                        utarray_push_back(bfeats, &(utstring_body(p)));
                        utstring_free(p);
                    }

                    *(needle + 2) = temp;

                    needle += 2;
                }

                //N_SPIN([KQRBSP])
                haystack = utstring_body(ps->b_after);
                needle = utstring_body(ps->b_before);

                while (*needle != '\0') {
                    temp = *(needle + 2);
                    *(needle + 2) = '\0';

                    if (strstr(haystack, needle) == NULL) {
                        UT_string* p;
                        utstring_new(p);
                        utstring_printf(p, "N_SPIN%c(%c)", *needle, *(needle + 1));
                        utarray_push_back(bfeats, &(utstring_body(p)));
                        utstring_free(p);
                    }

                    *(needle + 2) = temp;

                    needle += 2;
                }
            }

            if (strcmp(utstring_body(ps->w_before), utstring_body(ps->w_after)) != 0) {
                char* haystack;
                char* needle;
                char temp;

                //P-PIN([KQRBSP])
                haystack = utstring_body(ps->w_before);
                needle = utstring_body(ps->w_after);

                while (*needle != '\0') {
                    temp = *(needle + 2);
                    *(needle + 2) = '\0';

                    if (strstr(haystack, needle) == NULL) {
                        UT_string* p;
                        utstring_new(p);
                        utstring_printf(p, "P_PIN%c(%c)", *needle, *(needle + 1));
                        utarray_push_back(bfeats, &(utstring_body(p)));
                        utstring_free(p);
                    }

                    *(needle + 2) = temp;

                    needle += 2;
                }

                //N-PIN([KQRBSP])
                haystack = utstring_body(ps->w_after);
                needle = utstring_body(ps->w_before);

                while (*needle != '\0') {
                    temp = *(needle + 2);
                    *(needle + 2) = '\0';

                    if (strstr(haystack, needle) == NULL) {
                        if ((elt->captured == false) || (captured_piece != *needle) || (captured_id != *(needle + 1))) {
                            UT_string* p;
                            utstring_new(p);
                            utstring_printf(p, "N_PIN%c(%c)", *needle, *(needle + 1));
                            utarray_push_back(bfeats, &(utstring_body(p)));
                            utstring_free(p);
                        }
                    }

                    *(needle + 2) = temp;

                    needle += 2;
                }
            }

            //OGATE
            //If mating piece (QRBP) not pinned and mating move not possible before black move.
            //OGATEB
            //If mating (QRB) is not pinnded and mating move possible before black move but isn't check.

            if (mates == 1) {

                BOARD* mb = elt->nextply->vektor;

                if ((mb->mover != KING) && (mb->mover != KNIGHT)) {

                    char cid = bmIdBoard->white_ids[mb->from];

                    if (strchr(utstring_body(ps->w_before), cid) == NULL) {
                        unsigned char mfrom = mb->from;
                        unsigned char mto = mb->to;
                        bool mfound = false;
                        BOARD* mptr;

                        DL_FOREACH(wlist->vektor, mptr) {

                            if ((mfrom == mptr->from) && (mto == mptr->to)) {
                                mfound = true;
                                break;
                            }
                        }

                        if (mfound == false) {
                            utarray_push_back(bfeats, &ogate);
                        }
                    }
                }

                if ((mb->mover == QUEEN) || (mb->mover == ROOK) || (mb->mover == BISHOP)) {

                    char cid = bmIdBoard->white_ids[mb->from];

                    if (strchr(utstring_body(ps->w_before), cid) == NULL) {
                        unsigned char mfrom = mb->from;
                        unsigned char mto = mb->to;
                        bool mfound = false;
                        BOARD* mptr;

                        DL_FOREACH(wlist->vektor, mptr) {

                            if ((mfrom == mptr->from) && (mto == mptr->to) && (mptr->check == false)) {
                                mfound = true;
                                break;
                            }
                        }

                        if (mfound == true) {
                            utarray_push_back(bfeats, &ogateb);
                        }
                    }
                }
            }

            //S_BLOCK

            if ((mates == 1) && (elt->mover != KING)) {
                int bkpos = (int) elt->pos->kingsq[BLACK];
                int to = (int) elt->to;
                int dist = abs(bkpos - to);

                if ((dist == 1) || (dist == 8) || (dist == 7) || (dist == 9)) {
                    // Move is next to bK
                    BOARD* mb;
                    BITBOARD temp;

                    mb = elt->nextply->vektor;
                    temp = mb->pos->bitBoard[BLACK][OCCUPIED];

                    if ((temp & setMask[to]) != 0) {
                        if (attacks(mb->pos, to, WHITE) == false) {
                            utarray_push_back(bfeats, &sblock);
                        }
                    }
                }
            }

            //P_SCUT([KQRBSP])
            // If the actual mate would have been prevented by another black piece (that isn't pinned)
            // if the moving piece hadn't moved.
            //N-GUARD of mating square(s)
            // If the actual mate would have been prevented by the moving piece if it hadn't moved.

            if ((mates == 1) && (elt->mover != KING)) {
                BOARD* mb = elt->nextply->vektor;
                int mchecks = count_checks(WHITE, mb);
                unsigned char mfrom = mb->from;
                unsigned char mto = mb->to;
                bool mfound = false;
                BOARD* mptr;

                DL_FOREACH(wlist->vektor, mptr) {

                    if ((mfrom == mptr->from) && (mto == mptr->to) && (mptr->check == true)) {
                        mfound = true;
                        break;
                    }
                }

                if (mfound == true) {

                    assert(mptr->tag != '#');
                    unsigned int mflights;
                    BOARD* refptr;
                    bool from_piece_found = false;
                    BOARDLIST* mblist = generateBlackBoardlist(mptr, 1, &mflights);

                    DL_FOREACH(mblist->vektor, refptr) {

                        if ((refptr->from == elt->from) && (from_piece_found == false)) {
                            // unguard
                            utarray_push_back(bfeats, &nguard);
                            from_piece_found = true;
                        } else if ((refptr->from != elt->from) && (refptr->mover != KING) && (mchecks == 1)) {
                            // Interference
                            UT_string* cut;
                            char bpid[3];
                            bpid[0] = pieces[refptr->mover];
                            bpid[1] = inIdBrd->black_ids[refptr->from];
                            bpid[2] = '\0';

                            if (strstr(utstring_body(ps->b_after), bpid) == NULL) {
                                utstring_new(cut);
                                utstring_printf(cut, "S_CUT%c(%c)", bpid[0], bpid[1]);
                                utarray_push_back(bfeats, &(utstring_body(cut)));
                                utstring_free(cut);
                            }
                        }
                    }

                    freeBoardlist(mblist);
                }
            }

            //EP
            if (elt->ep == true) {
                char* ep = "EP";
                utarray_push_back(bfeats, &ep);
            }

            if (elt->mover == KING) {
                int diff;

                diff = abs(elt->from - elt->to);

                if ((diff == 8) || (diff == 1)) {
                    utarray_push_back(bfeats, &lab_p_flight);
                } else if ((diff == 7) || (diff == 9)) {
                    utarray_push_back(bfeats, &lab_s_flight);
                } else if ((elt->from == 60) && (elt->to == 62)) {
                    utarray_push_back(bfeats, &lab_kcast);
                } else if ((elt->from == 60) && (elt->to == 58)) {
                    utarray_push_back(bfeats, &lab_qcast);
                }
            }

            free_pin_status(ps);

            // Sort black features
            utarray_sort(bfeats, featsort);
            // Add black features to var
            unsigned int bf = utarray_len(bfeats);
            unsigned int bfc = 0;
            p = NULL;

            while ((p = (char**)utarray_next(bfeats, p))) {
                bfc++;

                if (bfc == bf) {
                    utstring_printf(var, "%s", *p);
                } else {
                    utstring_printf(var, "%s,", *p);
                }
            }

            // Identify white mover and mate details and add to var

            if (mates == 1) {
                UT_string* wm = get_mate_class(elt, elt->nextply->vektor, bmIdBoard);
                utstring_printf(var, "::");
                utstring_concat(var, wm);
                utstring_free(wm);

                // Add var classification if unique so far.
                HASH_FIND_STR(vars, utstring_body(var), s);

                if (s == NULL) {
                    s = (HASH_VAR*) malloc(sizeof(HASH_VAR));
                    SENGINE_MEM_ASSERT(s);
                    s->class = strdup(utstring_body(var));
                    HASH_ADD_KEYPTR(hh, vars, s->class, strlen(s->class), s);
                }
            } else {
                utstring_concat(refut, var);
            }

            utstring_free(var);
            utstring_free(bfeat);
            utarray_free(bfeats);
            freeIdBoard(bmIdBoard);
        }
    }

    // Add var classifications to xml and free from uthash.
    HASH_ITER(hh, vars, s, tmp) {
        add_var(s->class);
        HASH_DEL(vars, s);
        free(s->class);
        free(s);
    }

    if (utstring_len(refut) > 0) {
        add_refut(utstring_body(refut));
    }

    utstring_free(refut);

    return;
}

