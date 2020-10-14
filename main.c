/*
 *	sengine.c
 *	(c) 2017-2018, Brian Stephenson
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
 *	This is the main control module that contains the 'main' function.
 */

#include "sengine.h"

extern enum STIP opt_stip;
extern bool opt_meson;
extern bool opt_classify;
extern enum AIM opt_aim;
extern enum STIP opt_stip;
extern unsigned int opt_moves;

static clock_t prog_start, prog_end;
static double run_time;
enum SOUNDNESS sound;

static void do_direct(BOARD*);
static void do_self(BOARD*);
static void do_help(BOARD*);
static void do_reflex(BOARD*);

void end_clock(void)
{
    prog_end = clock();
    run_time = (double)(prog_end - prog_start) / CLOCKS_PER_SEC;
    return;
}

int main(int argc, char* argv[])
{
    int rc;
    prog_start = clock();
    rc = do_options(argc, argv);

    if (rc == 0) {
        BOARD* init_pos;
        init();
        init_mem();

        if (opt_stip == HELP) {
            init_pos = setup_diagram(WHITE);
        } else {
            init_pos = setup_diagram(BLACK);
        }

        rc = validate_board(init_pos);

        if (rc == 0) {
            switch (opt_stip) {
            case DIRECT: {
                do_direct(init_pos);
                break;
            }

            case SELF:
                do_self(init_pos);
                break;

            case REFLEX:
                do_reflex(init_pos);
                break;

            case HELP:
                do_help(init_pos);
                break;

            default:
                (void) fputs("sengine ERROR: impossible invalid stipulation!!",
                             stderr);
                exit(1);
                break;
            }

            close_mem();
        } else {
            close_mem();
            prog_end = clock();
            run_time = (double)(prog_end - prog_start) / CLOCKS_PER_SEC;
            (void) fprintf(stderr, "Running Time = %f\n", run_time);
        }
    }

    return rc;
}

void do_direct(BOARD* init_pos)
{
    DIR_SOL* dir_sol;
    dir_sol = (DIR_SOL*) calloc(1, sizeof(DIR_SOL));
    SENGINE_MEM_ASSERT(dir_sol);
    solve_direct(dir_sol, init_pos);
    start_dir();

    if (dir_sol->set != NULL) {
        add_dir_set(dir_sol->set);
    }

    if (dir_sol->tries != NULL) {
        add_dir_tries(dir_sol->tries);
    }

    if (dir_sol->keys != NULL) {
        add_dir_keys(dir_sol->keys);
    }

    if (opt_meson == false) {
        add_dir_options();
        add_dir_stats(dir_sol);
    }

    end_clock();

    if (opt_meson == false) {
        time_dir(run_time);
    }

    end_dir();

    if ((opt_classify == true) && (opt_aim == MATE) && (opt_stip == DIRECT) && (opt_moves == 2) && (sound == SOUND)) {
        class_direct_2(dir_sol, init_pos);
    }

    if (dir_sol->set != NULL) {
        freeBoardlist(dir_sol->set);
    }

    if (dir_sol->tries != NULL) {
        freeBoardlist(dir_sol->tries);
    }

    if (dir_sol->keys != NULL) {
        freeBoardlist(dir_sol->keys);
    }

    free(dir_sol);
    freeBoard(init_pos);
    return;
}

static void do_self(BOARD* init_pos)
{
    (void) fputs("sengine ERROR: Can't solve selfmates yet!\n",
                 stderr);
    exit(1);
    return;
}

static void do_help(BOARD* init_pos)
{
    (void) fputs("sengine ERROR: Can't solve helpmates yet!\n",
                 stderr);
    exit(1);
    return;
}

static void do_reflex(BOARD* init_pos)
{
    (void) fputs("sengine ERROR: Can't solve reflexmates yet!\n",
                 stderr);
    exit(1);
    return;
}

