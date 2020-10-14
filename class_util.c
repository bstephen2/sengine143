
/*
 *	class_util.c
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
 *	This is the module for utility routines for classifying directmates in two.
 */

#include "sengine.h"

extern BITBOARD clearMask[64];
extern BITBOARD setMask[64];
extern BITBOARD knight_attacks[64];
extern BITBOARD bishop_attacks[64];
extern BITBOARD rook_attacks[64];
extern BITBOARD pawn_attacks[2][64];
extern BITBOARD pawn_moves[2][64];
extern BBOARD rook_commonAttacks[64][64];
extern BBOARD bishop_commonAttacks[64][64];

static const unsigned char w_ids[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
static const unsigned char b_ids[] = "abcdefghijklmnopqrstuvwxyz";
static char pieces[] = "0PSBRQK";
int count_checks(enum COLOUR, BOARD* inBrd);

int tzcount(BITBOARD inBrd);

void setup_id_board(BOARD* inBrd, ID_BOARD* idBrd)
{
    int w = 0;
    int b = 0;
    BITBOARD temp;
    int i;

    temp = inBrd->pos->bitBoard[WHITE][OCCUPIED];

    i = tzcount(temp);

    while (i < 64) {
        idBrd->white_ids[i] = w_ids[w];
        w++;
        temp &= clearMask[i];
        i = tzcount(temp);
    }

    temp = inBrd->pos->bitBoard[BLACK][OCCUPIED];

    i = tzcount(temp);

    while (i < 64) {
        idBrd->black_ids[i] = b_ids[b];
        b++;
        temp &= clearMask[i];
        i = tzcount(temp);
    }

    return;
}

void update_id_board(enum COLOUR colour, BOARD* inBrd, ID_BOARD* inId, ID_BOARD* outId)
{
    if (colour == WHITE) {
        if (inBrd->ep == true) {
            outId->white_ids[inBrd->to + 8] = outId->white_ids[inBrd->from];
            outId->white_ids[inBrd->from] = 'Z';
            outId->black_ids[inBrd->to] = 'Z';

#ifndef NDEBUG
            {
                char before_line[9];
                char after_line[9];
                int j;

                before_line[8] = '\0';
                after_line[8] = '\0';

                fputs("WHITE EP\nWHITE BOARDS\n\n", stderr);

                for (j = 56; j >= 0; j -= 8) {
                    strncpy(before_line, &(inId->white_ids[j]), 8);
                    strncpy(after_line, &(outId->white_ids[j]), 8);
                    fprintf(stderr, "%s\t%s\n", before_line, after_line);
                }

                fputs("\nWHITE EP\nBLACK BOARDS\n\n", stderr);

                for (j = 56; j >= 0; j -= 8) {
                    strncpy(before_line, &(inId->black_ids[j]), 8);
                    strncpy(after_line, &(outId->black_ids[j]), 8);
                    fprintf(stderr, "%s\t%s\n", before_line, after_line);
                }
            }
#endif

        } else if ((inBrd->mover == KING) && (inBrd->from == 4) && (inBrd->to == 6)) {
            // White king-side castling
            outId->white_ids[6] = outId->white_ids[4];
            outId->white_ids[4] = 'Z';
            outId->white_ids[5] = outId->white_ids[7];
            outId->white_ids[7] = 'Z';
        } else if ((inBrd->mover == KING) && (inBrd->from == 4) && (inBrd->to == 2)) {
            // White queen-side castling
            outId->white_ids[2] = outId->white_ids[4];
            outId->white_ids[4] = 'Z';
            outId->white_ids[3] = outId->white_ids[0];
            outId->white_ids[0] = 'Z';
        } else {
            outId->white_ids[inBrd->to] = outId->white_ids[inBrd->from];
            outId->white_ids[inBrd->from] = 'Z';
        }
    } else if (colour == BLACK) {
        if (inBrd->ep == true) {
            outId->black_ids[inBrd->to - 8] = outId->black_ids[inBrd->from];
            outId->black_ids[inBrd->from] = 'Z';
            outId->white_ids[inBrd->to] = 'Z';

#ifndef NDEBUG
            {
                char before_line[9];
                char after_line[9];
                int j;

                before_line[8] = '\0';
                after_line[8] = '\0';

                fputs("BLACK EP\nWHITE BOARDS\n\n", stderr);

                for (j = 56; j >= 0; j -= 8) {
                    strncpy(before_line, &(inId->white_ids[j]), 8);
                    strncpy(after_line, &(outId->white_ids[j]), 8);
                    fprintf(stderr, "%s\t%s\n", before_line, after_line);
                }

                fputs("\nBLACK EP\nBLACK BOARDS\n\n", stderr);

                for (j = 56; j >= 0; j -= 8) {
                    strncpy(before_line, &(inId->black_ids[j]), 8);
                    strncpy(after_line, &(outId->black_ids[j]), 8);
                    fprintf(stderr, "%s\t%s\n", before_line, after_line);
                }
            }
#endif

        } else if ((inBrd->mover == KING) && (inBrd->from == 60) && (inBrd->to == 62)) {
            // Black king-side castling
            outId->black_ids[62] = outId->black_ids[60];
            outId->black_ids[60] = 'Z';
            outId->black_ids[61] = outId->black_ids[63];
            outId->black_ids[63] = 'Z';
        } else if ((inBrd->mover == KING) && (inBrd->from == 60) && (inBrd->to == 58)) {
            // Black queen-side castling
            outId->black_ids[58] = outId->black_ids[60];
            outId->black_ids[60] = 'Z';
            outId->black_ids[59] = outId->black_ids[56];
            outId->black_ids[56] = 'Z';
        } else {
            outId->black_ids[inBrd->to] = outId->black_ids[inBrd->from];
            outId->black_ids[inBrd->from] = 'Z';
        }
    }

    return;
}

char get_piece_type(enum COLOUR col, BOARD* inBrd, unsigned char to)
{
    BITBOARD temp;
    enum PIECE ep;
    int i;
    int ito = (int) to;
    POSITION* pos = inBrd->pos;

    for (ep = PAWN; ep <= QUEEN; ep++) {
        temp = pos->bitBoard[col][ep];
        i = tzcount(temp);

        while (i < 64) {

            if (i == ito) {
                return pieces[ep];
            }

            temp &= clearMask[i];
            i = tzcount(temp);
        }
    }

    return 'X';
}

void get_check_square_list(enum COLOUR colour, BOARD* inBrd, CHECK_SQUARE_LIST* csl)
{
    BITBOARD qb;
    BITBOARD qr;
    BITBOARD occupied;
    BBOARD temp;
    int i;
    POSITION* pos = inBrd->pos;
    int square = (int) inBrd->pos->kingsq[colour ^ 1];
    int count = 0;

    // (2) Atack by a Pawn?

    if ((pawn_attacks[colour ^ 1][square] & pos->bitBoard[colour][PAWN]) !=
            0) {
        csl->real_piece[count] = PAWN;
        csl->as_piece[count] = PAWN;
        count++;
        csl->count = count;
    }

    // (3) Attack by a Knight?

    if ((knight_attacks[square] & pos->bitBoard[colour][KNIGHT]) != 0) {
        csl->real_piece[count] = KNIGHT;
        csl->as_piece[count] = KNIGHT;
        count++;
        csl->count = count;

        if (count == 2) {
            return;
        }
    }

    // (4) Attack by bishop/queen?
    qb = pos->bitBoard[colour][BISHOP];
    occupied = pos->bitBoard[WHITE][OCCUPIED] | pos->bitBoard[BLACK][OCCUPIED];

    if ((bishop_attacks[square] & qb) != 0) {
        i = tzcount(qb);

        while (i < 64) {
            temp = bishop_commonAttacks[i][square];

            if (temp.used == true) {
                if ((occupied & temp.bb) == 0) {
                    csl->real_piece[count] = BISHOP;
                    csl->as_piece[count] = BISHOP;
                    csl->square[count] = i;
                    count++;
                    csl->count = count;

                    if (count == 2) {
                        return;
                    }
                }
            }

            qb &= clearMask[i];
            i = tzcount(qb);
        }
    }

    qb = pos->bitBoard[colour][QUEEN];
    occupied = pos->bitBoard[WHITE][OCCUPIED] | pos->bitBoard[BLACK][OCCUPIED];

    if ((bishop_attacks[square] & qb) != 0) {
        i = tzcount(qb);

        while (i < 64) {
            temp = bishop_commonAttacks[i][square];

            if (temp.used == true) {
                if ((occupied & temp.bb) == 0) {
                    csl->real_piece[count] = QUEEN;
                    csl->as_piece[count] = BISHOP;
                    csl->square[count] = i;
                    count++;
                    csl->count = count;

                    if (count == 2) {
                        return;
                    }
                }
            }

            qb &= clearMask[i];
            i = tzcount(qb);
        }
    }

    // (5) Attack by rook/queen?
    qr = pos->bitBoard[colour][ROOK];

    if ((rook_attacks[square] & qr) != 0) {
        i = tzcount(qr);

        while (i < 64) {
            temp = rook_commonAttacks[i][square];

            if (temp.used == true) {
                if ((occupied & temp.bb) == 0) {
                    csl->real_piece[count] = ROOK;
                    csl->as_piece[count] = ROOK;
                    csl->square[count] = i;
                    count++;
                    csl->count = count;

                    if (count == 2) {
                        return;
                    }
                }
            }

            qr &= clearMask[i];
            i = tzcount(qr);
        }
    }

    qr = pos->bitBoard[colour][QUEEN];

    if ((rook_attacks[square] & qr) != 0) {
        i = tzcount(qr);

        while (i < 64) {
            temp = rook_commonAttacks[i][square];

            if (temp.used == true) {
                if ((occupied & temp.bb) == 0) {
                    csl->real_piece[count] = QUEEN;
                    csl->as_piece[count] = ROOK;
                    csl->square[count] = i;
                    count++;
                    csl->count = count;

                    if (count == 2) {
                        return;
                    }
                }
            }

            qr &= clearMask[i];
            i = tzcount(qr);
        }
    }

    return;
}

int count_checks(enum COLOUR colour, BOARD* inBrd)
{
    int rc = 0;
    BITBOARD qb;
    BITBOARD qr;
    BITBOARD occupied;
    BBOARD temp;
    int i;
    POSITION* pos = inBrd->pos;
    int square = (int) inBrd->pos->kingsq[colour ^ 1];

    if ((pawn_attacks[colour ^ 1][square] & pos->bitBoard[colour][PAWN]) !=
            0) {
        rc++;
    }

    // (3) Attack by a Knight?

    if ((knight_attacks[square] & pos->bitBoard[colour][KNIGHT]) != 0) {
        rc++;
    }

    // (4) Attack by bishop/queen?
    qb = pos->bitBoard[colour][BISHOP] | pos->bitBoard[colour][QUEEN];
    occupied = pos->bitBoard[WHITE][OCCUPIED] | pos->bitBoard[BLACK][OCCUPIED];

    if ((bishop_attacks[square] & qb) != 0) {
        i = tzcount(qb);

        while (i < 64) {
            temp = bishop_commonAttacks[i][square];

            if (temp.used == true) {
                if ((occupied & temp.bb) == 0) {
                    rc++;
                }
            }

            qb &= clearMask[i];
            i = tzcount(qb);
        }
    }

    // (5) Attack by rook/queen?
    qr = pos->bitBoard[colour][ROOK] | pos->bitBoard[colour][QUEEN];

    if ((rook_attacks[square] & qr) != 0) {
        i = tzcount(qr);

        while (i < 64) {
            temp = rook_commonAttacks[i][square];

            if (temp.used == true) {
                if ((occupied & temp.bb) == 0) {
                    rc++;
                }
            }

            qr &= clearMask[i];
            i = tzcount(qr);
        }
    }

    return rc;
}

void populate_pin_status(PIN_STATUS* ps, BOARD* beforeBrd, BOARD* afterBrd, ID_BOARD* beforeID, ID_BOARD* afterID)
{
    BITBOARD qr;
    POSITION* pos;
    enum PIECE p;
    int i;
    int j;
    int before_bchecks = count_checks(BLACK, beforeBrd);
    int before_wchecks = count_checks(WHITE, beforeBrd);
    int after_bchecks = count_checks(BLACK, afterBrd);
    int after_wchecks = count_checks(WHITE, afterBrd);

    pos = beforeBrd->pos;

    // WHITE_BEFORE

    for (p = PAWN; p <= QUEEN; p++) {
        qr = pos->bitBoard[WHITE][p];
        i = tzcount(qr);

        while (i < 64) {
            pos->bitBoard[WHITE][p] &= clearMask[i];
            pos->bitBoard[WHITE][OCCUPIED] &= clearMask[i];
            j = count_checks(BLACK, beforeBrd);
            pos->bitBoard[WHITE][p] |= setMask[i];
            pos->bitBoard[WHITE][OCCUPIED] |= setMask[i];

            if (j > before_bchecks) {
                char pic = pieces[p];
                char id = beforeID->white_ids[i];
                utstring_printf(ps->w_before, "%c%c", pic, id);
                break;
            }

            qr &= clearMask[i];
            i = tzcount(qr);
        }
    }

    // BLACK BEFORE

    for (p = PAWN; p <= QUEEN; p++) {
        qr = pos->bitBoard[BLACK][p];
        i = tzcount(qr);

        while (i < 64) {
            pos->bitBoard[BLACK][p] &= clearMask[i];
            pos->bitBoard[BLACK][OCCUPIED] &= clearMask[i];
            j = count_checks(WHITE, beforeBrd);
            pos->bitBoard[BLACK][p] |= setMask[i];
            pos->bitBoard[BLACK][OCCUPIED] |= setMask[i];

            if (j > before_wchecks) {
                char pic = pieces[p];
                char id = beforeID->black_ids[i];
                utstring_printf(ps->b_before, "%c%c", pic, id);
                break;
            }

            qr &= clearMask[i];
            i = tzcount(qr);
        }
    }

    pos = afterBrd->pos;
    // WHITE AFTER

    for (p = PAWN; p <= QUEEN; p++) {
        qr = pos->bitBoard[WHITE][p];
        i = tzcount(qr);

        while (i < 64) {
            pos->bitBoard[WHITE][p] &= clearMask[i];
            pos->bitBoard[WHITE][OCCUPIED] &= clearMask[i];
            j = count_checks(BLACK, afterBrd);
            pos->bitBoard[WHITE][p] |= setMask[i];
            pos->bitBoard[WHITE][OCCUPIED] |= setMask[i];

            if (j > after_bchecks) {
                char pic = pieces[p];
                char id = afterID->white_ids[i];
                utstring_printf(ps->w_after, "%c%c", pic, id);
                break;
            }

            qr &= clearMask[i];
            i = tzcount(qr);
        }
    }

    // BLACK AFTER

    for (p = PAWN; p <= QUEEN; p++) {
        qr = pos->bitBoard[BLACK][p];
        i = tzcount(qr);

        while (i < 64) {
            pos->bitBoard[BLACK][p] &= clearMask[i];
            pos->bitBoard[BLACK][OCCUPIED] &= clearMask[i];
            j = count_checks(WHITE, afterBrd);
            pos->bitBoard[BLACK][p] |= setMask[i];
            pos->bitBoard[BLACK][OCCUPIED] |= setMask[i];

            if (j > after_wchecks) {
                char pic = pieces[p];
                char id = afterID->black_ids[i];
                utstring_printf(ps->b_after, "%c%c", pic, id);
                break;
            }

            qr &= clearMask[i];
            i = tzcount(qr);
        }
    }

    return;
}
