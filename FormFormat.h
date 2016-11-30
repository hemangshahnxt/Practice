

#pragma once



extern  double PRINT_X_SCALE;			//13.87//14.6//15.1
extern  double PRINT_Y_SCALE;			//13.87//14.6//15.1
extern  double PRINT_X_OFFSET;		//-350//-400//-100
extern  double PRINT_Y_OFFSET;		//400//100

#define TWODIGIT	0x00000001	// 1
#define READONLY	0x00000002	// 2
#define PRINTABLE	0x00000004	// 4
#define GROUP		0x00000008	// 8
#define ITALIC		0x00000010	// 16
#define BIRTHDATE	0x00000020	// 32
#define PHONE		0x00000040	// 64
#define IGNORESOURCE  0x00000080	// 128

#define	STATIC		0x00000100	// 256
#define	EDIT		0x00000200	// 512
#define LINE		0x00000400	// 1024
#define CHECK		0x00000800	// 2048
#define DATE		0x00001000	// 4096

#define RIGHTALIGN	0x00002000	// 8192
#define YEARFIRST	0x00004000	// 16384
#define WIDE		0x00008000	// 32768
#define NOSLASH		0x00010000	// 65536

// (j.jones 2013-05-08 16:30) - PLID 54511 - added checkbox type, our existing CHECK is actually a radio button.
// We would need CHECKBOX + CHECK to be a functional checkbox, so the actual format value will be 133120. (131072 + 2048)
#define CHECKBOX	0x00020000	// 131072

//examples:
//year-first date is 20480
//month-first date is 4096
//month-first wide-date (10  30   2002) is 36864

//all items with 512 will be saved

// Small font used in box 31, 32 and 33.
// This overrides the font sizes in FormsT.
//moved to static variable
//#define MINI_FONT_SIZE	80