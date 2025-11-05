#ifndef __CHARDEV_H_
#define __CHARDEV_H_



typedef struct beep_desc{
	int beep;    //2 3 4 5
	int beep_state;  //0 or 1
	int tcnt;  //占空比
	int tcmp;  //调节占空比
}beep_desc_t;

#define beeptype 'f'

#define BEEP_ON 	_IOW(beeptype,0,beep_desc_t)
#define BEEP_OFF 	_IOW(beeptype,1,beep_desc_t)
#define BEEP_FREQ 	_IOW(beeptype,2,beep_desc_t)






#endif