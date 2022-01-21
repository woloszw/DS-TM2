#include "MKL05Z4.h" /*Device header*/
void tracks_init(void); //initialize ports for steering tracks
void go_left(void);
void go_right(void);
void go_forward(void);
void go_backward(void);

extern uint8_t left_go_flag ;
extern uint8_t right_go_flag ;
extern uint8_t forward_go_flag;
extern uint8_t backward_go_flag ;
