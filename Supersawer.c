/**
	@file
	Supersawer.c - one of the simplest max objects you can make -rdd 2001
	(Supersawer is/was the name of a Hungarian vitamin C tablet-drink from the early 90s)

	this example is provided for musicians who want to learn to write their own Max externals but who only
	have rudimentary computer programming skills and feel somewhat overwhelmed by the other examples in the Max SDK

	this object has 2 inlets and one outlet
	it responds to ints in its inlets and the 'bang' message in the left inlet
	it responds to the 'assistance' message sent by Max when the mouse is positioned over an inlet or outlet
		(including an assistance method is optional, but strongly sugggested)
	it adds its input values together and outputs their sum

	@ingroup	examples
*/

#include "ext.h"			// you must include this - it contains the external object's link to available Max functions
#include "ext_obex.h"		// this is required for all objects using the newer style for writing objects.

#define DEBUG 0

typedef struct _supersawer {	// defines our object's internal variables for each instance in a patch
	t_object p_ob;			// object header - ALL objects MUST begin with this...
	double frequency;
	long max_voices;
	long num_voices;
	double detune;
	long num_outlets;
	//void* test_outlet;
	void **outlets;
} t_Supersawer;


// these are prototypes for the methods that are defined below
void Supersawer_bang(t_Supersawer *x);
void Supersawer_ft(t_Supersawer *x, float n);	//freq (float)
void Supersawer_int(t_Supersawer* x, float n);	//freq (int)
void Supersawer_in1(t_Supersawer *x, long n);	//# of voices (density)
void Supersawer_ft2(t_Supersawer* x, float n);	//detune amount
void Supersawer_assist(t_Supersawer *x, void *b, long m, long a, char *s);
void *Supersawer_new(long n);


t_class *Supersawer_class;		// global pointer to the object class - so max can reference the object


//--------------------------------------------------------------------------

void ext_main(void *r)
{
	t_class *c;

	c = class_new("Supersawer", (method)Supersawer_new, (method)NULL, sizeof(t_Supersawer), 0L, A_DEFLONG, 0);
	// class_new() loads our external's class into Max's memory so it can be used in a patch
	// Supersawer_new = object creation method defined below

	class_addmethod(c, (method)Supersawer_bang,		"bang",		0);			// the method it uses when it gets a bang in the left inlet
	class_addmethod(c, (method)Supersawer_ft,		"float",		A_FLOAT, 0);	// frequency (float)
	class_addmethod(c, (method)Supersawer_int, "int", A_LONG, 0);	// Frequency (int)
	class_addmethod(c, (method)Supersawer_in1,		"in1",		A_LONG, 0);	// # of voices
	class_addmethod(c, (method)Supersawer_ft2, "ft2", A_FLOAT, 0);	// Detune
	// "ft1" is the special message for floats
	class_addmethod(c, (method)Supersawer_assist,	"assist",	A_CANT, 0);	// (optional) assistance method needs to be declared like this

	class_register(CLASS_BOX, c);
	Supersawer_class = c;

	if (DEBUG) post("Supersawer object loaded...",0);	// post any important info to the max window when our class is loaded
}


//--------------------------------------------------------------------------

void *Supersawer_new(long n)		// n = int argument typed into object box (A_DEFLONG) -- defaults to 0 if no args are typed
{
	t_Supersawer *x;				// local variable (pointer to a t_Supersawer data structure)
	
	x = (t_Supersawer *)object_alloc(Supersawer_class); // create a new instance of this object

	floatin(x, 2);				// Detune inlet
	intin(x,1);					// # of voices inlet

	x->max_voices = n ? n : 1;			// Set max no. of voices to be the input argument (or 6 if no argument)
	x->frequency = 400;
	x->detune = 0.5;
	x->num_voices = 1;
	x->num_outlets = (x->max_voices * 2) + 1;
	//These value initializations are confirmed to work

	//Create outlets dynamically (pwease?)
	x->outlets = (void**)malloc(x->num_outlets * sizeof(void*));
	for (int i = 0; i < x->num_outlets; i++) {		//one outlet for each positive voice, one for each negative voice, and one for the base frequency
		x->outlets[i] = outlet_new(x, "float");
	}	//this does generate the correct number of outlets

	//x->test_outlet = outlet_new(x, "float");

	if (DEBUG) post(" new Supersawer object instance added to patch...",0); // post important info to the max window when new instance is created

	return(x);					// return a reference to the object instance
}


//--------------------------------------------------------------------------

void Supersawer_assist(t_Supersawer *x, void *b, long m, long a, char *s) // 4 final arguments are always the same for the assistance method
{
	if (m == ASSIST_OUTLET)
		sprintf(s,"Sum of Left and Right Inlets");
	else {
		switch (a) {
		case 0:
			sprintf(s,"Inlet %ld: Left Operand (Causes Output)", a);
			break;
		case 1:
			sprintf(s,"Inlet %ld: Right Operand (Added to Left)", a);
			break;
		}
	}
}


void Supersawer_bang(t_Supersawer *x)			// x = reference to this instance of the object
{
	if (DEBUG) post("Supersawer_bang", 0);
	if (DEBUG) post("Max voices: %i", x->max_voices);
	if (DEBUG) post("Frequency: %f", x->frequency);
	if (DEBUG) post("Num voices: %i", x->num_voices);
	if (DEBUG) post("Detune: %f", x->detune);
	double detune_amount = x->frequency * x->detune;
	if (DEBUG) post("Detune amount: %f", detune_amount);
	outlet_float((t_outlet*) x->outlets[x->num_outlets - 1], x->frequency);	//output base frequency to middle outlet
	int j = 1;
	for (int i = 1; i < x->max_voices + 1; i++) {
		if (i < x->num_voices + 1) {
			outlet_float((t_outlet*)x->outlets[x->num_outlets - 1 - j], x->frequency + ((double)i) * detune_amount);
			outlet_float((t_outlet*)x->outlets[x->num_outlets - 1 - ++j], x->frequency - ((double)i) * detune_amount);
			if (DEBUG) post("Out +: %f", x->frequency + ((double)i) * detune_amount);
			if (DEBUG) post("Out -: %f", x->frequency - ((double)i) * detune_amount);
		}
		else {
			outlet_float((t_outlet*)x->outlets[x->num_outlets - 1 - j], 0.0);
			outlet_float((t_outlet*)x->outlets[x->num_outlets - 1 - ++j], 0.0);
			if (DEBUG) post("Out NULL", 0);
			if (DEBUG) post("Out NULL", 0);
		}
		j++;
	}

	//Works with default values!!!
}

//int in left inlet
void Supersawer_int(t_Supersawer *x, long n)	// x = the instance of the object; n = the int received in the left inlet
{
	if (DEBUG) post("Supersawer_int", 0);
	x->frequency = (float) n;				// store int frequency at leftmost inlet as float frequency in object
	//Supersawer_bang(x);						// ... call the bang method to update the supersaw and send to outlets
}

//float in left inlet
void Supersawer_ft(t_Supersawer *x, double n)	// x = the instance of the object, n = the int received in the right inlet
{
	if (DEBUG) post("Supersawer_int", 0);
	x->frequency = n;
	if (DEBUG) post("frequency changed to %f", x->frequency);
	//Supersawer_bang(x);
}

//int in second to left inlet
void Supersawer_in1(t_Supersawer* x, long n) {
	if (DEBUG) post("Supersawer_in1", 0);
	x->num_voices = n;
	if (DEBUG) post("num_voices changed to %i", x->num_voices);
	//Supersawer_bang(x);
}

//float in third to left inlet
void Supersawer_ft2(t_Supersawer* x, double n) {
	if (DEBUG) post("Supersawer_ft2", 0);
	x->detune = n;
	if (DEBUG) post("Detune changed to %f", x->detune);
}


