/*
 *	wmove.c
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
 *	This is the module for classifying white non-mating moves.
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
int featsort(const void* a, const void* b);
char get_piece_type(enum COLOUR, BOARD*, unsigned char);
void add_key(char*);

void classify_white_move(BOARD* initBrd, BOARD* wm, ID_BOARD* inIdBrd, ID_BOARD* afterIdBrd)
{
#ifndef NDEBUG
    fputs("classify_white_move()\n", stderr);
#endif
    UT_string* var;
    UT_array* wfeats;
    UT_string* wfeat;
    char** p;
    char captured_piece = ' ';
    char captured_id = ' ';

    utstring_new(var);
    utstring_new(wfeat);
    utarray_new(wfeats, &ut_str_icd);

    char piece = pieces[wm->mover];
    char id = inIdBrd->white_ids[wm->from];
    utstring_printf(var, "%c(%c);", piece, id);

    if (wm->check == true) {
        //CHECK
        utarray_push_back(wfeats, &lab_check);
    }

    if (wm->captured == true) {
        //X[QRBSP](id)
        UT_string* capstr;
        utstring_new(capstr);
        captured_id = inIdBrd->black_ids[wm->to];
        captured_piece = get_piece_type(BLACK, initBrd, wm->to);
        utstring_printf(capstr, "X%c(%c)", captured_piece, captured_id);
        utarray_push_back(wfeats, &(utstring_body(capstr)));
        utstring_free(capstr);
    }


    //F-GIVER(n)
    //F_TAKER(n)


    PIN_STATUS* ps = get_pin_status();
    populate_pin_status(ps, initBrd, wm, inIdBrd, afterIdBrd);

#ifndef NDEBUG
    fprintf(stderr, "w_before = %s\n", utstring_body(ps->w_before));
    fprintf(stderr, "w_after = %s\n", utstring_body(ps->w_after));
    fprintf(stderr, "b_before = %s\n", utstring_body(ps->b_before));
    fprintf(stderr, "b_after = %s\n", utstring_body(ps->b_after));
#endif

    if (strcmp(utstring_body(ps->w_before), utstring_body(ps->w_after)) != 0) {
        char* haystack;
        char* needle;
        char temp;

        //P_SPIN([KQRBSP])
        haystack = utstring_body(ps->w_before);
        needle = utstring_body(ps->w_after);

        while (*needle != '\0') {
            temp = *(needle + 2);
            *(needle + 2) = '\0';

            if (strstr(haystack, needle) == NULL) {
                UT_string* p;
                utstring_new(p);
                utstring_printf(p, "P_SPIN%c(%c)", *needle, *(needle + 1));
                utarray_push_back(wfeats, &(utstring_body(p)));
                utstring_free(p);
            }

            *(needle + 2) = temp;

            needle += 2;
        }

        //N_SPIN([KQRBSP])
        haystack = utstring_body(ps->w_after);
        needle = utstring_body(ps->w_before);

        while (*needle != '\0') {
            temp = *(needle + 2);
            *(needle + 2) = '\0';

            if (strstr(haystack, needle) == NULL) {
                UT_string* p;
                utstring_new(p);
                utstring_printf(p, "N_SPIN%c(%c)", *needle, *(needle + 1));
                utarray_push_back(wfeats, &(utstring_body(p)));
                utstring_free(p);
            }

            *(needle + 2) = temp;

            needle += 2;
        }
    }

    if (strcmp(utstring_body(ps->b_before), utstring_body(ps->b_after)) != 0) {
        char* haystack;
        char* needle;
        char temp;

        //P-PIN([KQRBSP])
        haystack = utstring_body(ps->b_before);
        needle = utstring_body(ps->b_after);

        while (*needle != '\0') {
            temp = *(needle + 2);
            *(needle + 2) = '\0';

            if (strstr(haystack, needle) == NULL) {
                UT_string* p;
                utstring_new(p);
                utstring_printf(p, "P_PIN%c(%c)", *needle, *(needle + 1));
                utarray_push_back(wfeats, &(utstring_body(p)));
                utstring_free(p);
            }

            *(needle + 2) = temp;

            needle += 2;
        }

        //N-PIN([KQRBSP])
        haystack = utstring_body(ps->b_after);
        needle = utstring_body(ps->b_before);

        while (*needle != '\0') {
            temp = *(needle + 2);
            *(needle + 2) = '\0';

            if (strstr(haystack, needle) == NULL) {
                if ((wm->captured == false) || (captured_piece != *needle) || (captured_id != *(needle + 1))) {
                    UT_string* p;
                    utstring_new(p);
                    utstring_printf(p, "N_PIN%c(%c)", *needle, *(needle + 1));
                    utarray_push_back(wfeats, &(utstring_body(p)));
                    utstring_free(p);
                }
            }

            *(needle + 2) = temp;

            needle += 2;
        }
    }

    //P_CUT([KQRBSP])
    //N_CUT([KQRBSP])
    //P_SCUT([KQRBSP])
    //N_SCUT([KQRBSP])

    free_pin_status(ps);

    //EP

    if (wm->ep == true) {
        char* ep = "EP";
        utarray_push_back(wfeats, &ep);
    }

    if (wm->mover == KING) {
        int diff;

        diff = abs(wm->from - wm->to);

        if ((diff == 8) || (diff == 1)) {
            utarray_push_back(wfeats, &lab_p_flight);
        } else if ((diff == 7) || (diff == 9)) {
            utarray_push_back(wfeats, &lab_s_flight);
        } else if ((wm->from == 4) && (wm->to == 6)) {
            utarray_push_back(wfeats, &lab_kcast);
        } else if ((wm->from == 4) && (wm->to == 2)) {
            utarray_push_back(wfeats, &lab_qcast);
        }
    }

    utarray_sort(wfeats, featsort);
    // Add white features to var
    unsigned int bf = utarray_len(wfeats);
    unsigned int bfc = 0;
    p = NULL;

    while ((p = (char**)utarray_next(wfeats, p))) {
        bfc++;

        if (bfc == bf) {
            utstring_printf(var, "%s", *p);
        } else {
            utstring_printf(var, "%s,", *p);
        }
    }


    add_key(utstring_body(var));

    utstring_free(var);
    utstring_free(wfeat);
    utarray_free(wfeats);

    return;
}

