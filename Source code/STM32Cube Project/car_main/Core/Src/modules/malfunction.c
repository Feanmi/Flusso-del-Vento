#include "modules/malfunction.h"

static uint8_t in_malfunction = 0;
extern uint8_t CONNECTED;

void init_malfunction_detection(){

}

void detect_malfunction(){
	if (!CONNECTED && !in_malfunction){
		in_malfunction = 1;
		enter_malfunction();
	}
	if (CONNECTED && in_malfunction){
			in_malfunction = 0;
			leave_malfunction();
	}
}

void leave_malfunction(){

}

void enter_malfunction(){

}
