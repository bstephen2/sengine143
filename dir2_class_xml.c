/*
 *	dir2_class_xml.c
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
 *	This is the module for preparing the xml output of classifications of directmates in two.
 *
 * MesonClass
 * 	Static
 * 		Item
 * 	Set
 * 		Var
 * 	Virtual
 * 		Try
 * 			Key
 * 			Threat
 * 			Var
 * 			Refut
 * 	Actual
 * 		Key
 * 		Threat
 * 		Var
 */

#include "sengine.h"
#include "genx.h"
#include "utstring.h"

static genxWriter w;
static const unsigned char mc[] = "MesonClass";
static const unsigned char stat[] = "Static";
static const unsigned char set[] = "Set";
static const unsigned char var[] = "Var";
static const unsigned char virt[] = "Virtual";

static const unsigned char try[] = "Try";

static const unsigned char thr[] = "Threat";
static const unsigned char refut[] = "Refut";
static const unsigned char act[] = "Actual";
static const unsigned char key[] = "Key";
static const unsigned char type[] = "Type";
static const unsigned char up_flights[] = "Up_Flights";
static const unsigned char up_checks[] = "Up_Checks";
static const unsigned char up_fgivers[] = "Up_Fgivers";
static const unsigned char up_caps[] = "Up_Caps";
static const unsigned char tot_up[] = "Tot_Up";
static const unsigned char added[] = "Added";
static const unsigned char changed[] = "Changed";
static const unsigned char removed[] = "Removed";

void start_class_2_xml()
{
    w = genxNew(NULL, NULL, NULL);
    (void) genxStartDocFile(w, stdout);
    (void) genxStartElementLiteral(w, NULL, mc);
    return;
}

void end_class_2_xml()
{
    (void) genxEndElement(w);
    (void) genxEndDocument(w);
    genxDispose(w);
    printf("\n");

    return;
}

void start_static_class_xml()
{
    (void) genxStartElementLiteral(w, NULL, stat);

    return;
}

void add_added(int inInt)
{
    UT_string* s;

    utstring_new(s);

    (void) genxStartElementLiteral(w, NULL, added);
    utstring_printf(s, "%d", inInt);
    (void) genxAddText(w, (unsigned char*) utstring_body(s));
    (void) genxEndElement(w);

    utstring_free(s);

    return;
}

void add_removed(int inInt)
{
    UT_string* s;

    utstring_new(s);

    (void) genxStartElementLiteral(w, NULL, removed);
    utstring_printf(s, "%d", inInt);
    (void) genxAddText(w, (unsigned char*) utstring_body(s));
    (void) genxEndElement(w);

    utstring_free(s);

    return;
}

void add_changed(int inInt)
{
    UT_string* s;

    utstring_new(s);

    (void) genxStartElementLiteral(w, NULL, changed);
    utstring_printf(s, "%d", inInt);
    (void) genxAddText(w, (unsigned char*) utstring_body(s));
    (void) genxEndElement(w);

    utstring_free(s);

    return;
}

void add_up_flights(int inInt)
{
    UT_string* s;

    utstring_new(s);

    (void) genxStartElementLiteral(w, NULL, up_flights);
    utstring_printf(s, "%d", inInt);
    (void) genxAddText(w, (unsigned char*) utstring_body(s));
    (void) genxEndElement(w);

    utstring_free(s);

    return;
}

void add_up_checks(int inInt)
{
    UT_string* s;

    utstring_new(s);

    (void) genxStartElementLiteral(w, NULL, up_checks);
    utstring_printf(s, "%d", inInt);
    (void) genxAddText(w, (unsigned char*) utstring_body(s));
    (void) genxEndElement(w);

    utstring_free(s);

    return;
}

void add_up_fgivers(int inInt)
{
    UT_string* s;

    utstring_new(s);

    (void) genxStartElementLiteral(w, NULL, up_fgivers);
    utstring_printf(s, "%d", inInt);
    (void) genxAddText(w, (unsigned char*) utstring_body(s));
    (void) genxEndElement(w);

    utstring_free(s);

    return;
}

void add_up_caps(int inInt)
{
    UT_string* s;

    utstring_new(s);

    (void) genxStartElementLiteral(w, NULL, up_caps);
    utstring_printf(s, "%d", inInt);
    (void) genxAddText(w, (unsigned char*) utstring_body(s));
    (void) genxEndElement(w);

    utstring_free(s);

    return;
}

void add_tot_up(int inInt)
{
    UT_string* s;

    utstring_new(s);

    (void) genxStartElementLiteral(w, NULL, tot_up);
    utstring_printf(s, "%d", inInt);
    (void) genxAddText(w, (unsigned char*) utstring_body(s));
    (void) genxEndElement(w);

    utstring_free(s);

    return;
}

void add_static_type(char* in_type)
{
    (void) genxStartElementLiteral(w, NULL, type);
    (void) genxAddText(w, (unsigned char*) in_type);
    (void) genxEndElement(w);

    return;
}

void end_static_clas_xml()
{
    (void) genxEndElement(w);

    return;
}

void start_set_class_xml()
{
    (void) genxStartElementLiteral(w, NULL, set);

    return;
}

void end_set_class_xml()
{
    (void) genxEndElement(w);

    return;
}

void start_virtual_class_xml()
{
    (void) genxStartElementLiteral(w, NULL, virt);

    return;
}

void end_virtual_class_xml()
{
    (void) genxEndElement(w);

    return;
}

void start_actual_class_xml()
{
    (void) genxStartElementLiteral(w, NULL, act);

    return;
}

void end_actual_class_xml(void)
{
    (void) genxEndElement(w);

    return;
}

void add_var(char* in_var)
{
    (void) genxStartElementLiteral(w, NULL, var);
    (void) genxAddText(w, (unsigned char*) in_var);
    (void) genxEndElement(w);

    return;
}

void start_try()
{
    (void) genxStartElementLiteral(w, NULL, try);

    return;
}

void end_try()
{
    (void) genxEndElement(w);

    return;
}

void add_key(char* in_key)
{
    (void) genxStartElementLiteral(w, NULL, key);
    (void) genxAddText(w, (unsigned char*) in_key);
    (void) genxEndElement(w);

    return;
}
void add_threat(char* in_thr)
{
    (void) genxStartElementLiteral(w, NULL, thr);
    (void) genxAddText(w, (unsigned char*) in_thr);
    (void) genxEndElement(w);

    return;
}

void add_refut(char* in_refut)
{
    (void) genxStartElementLiteral(w, NULL, refut);
    (void) genxAddText(w, (unsigned char*) in_refut);
    (void) genxEndElement(w);

    return;
}
