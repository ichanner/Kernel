

/*

Store an array of indexed of all the keys that are being currently pressed down (initialized to 0)

	- use default look up (keydown) table to determine keydown code - set element to 1
	- use default look up (keyup) table to unset element back to 0 

Store 5 look up tables:
	
	default lookup up (keyup)
	default look up (keydown) table

	LATER: CUSTOM tables: lookup tables for when SHFT, CTRL, CMD are being held down

*/

#define LFT_SHFT 27
#define NUM_KEYS 72

int keylog[72];

char keyMapping[87] = {

	[0x1e] = 'a',
    [0x30] = 'b',
    [0x2e] = 'c',
    [0x20] = 'd',
    [0x12] = 'e',
    [0x21] = 'f',
    [0x22] = 'g',
    [0x23] = 'h',
    [0x17] = 'i',
    [0x24] = 'j',
    [0x25] = 'k',
    [0x26] = 'l',
    [0x32] = 'm',
    [0x31] = 'n',
    [0x18] = 'o',
    [0x19] = 'p',
    [0x10] = 'q',
    [0x13] = 'r',
    [0x1f] = 's',
    [0x14] = 't',
    [0x16] = 'u',
    [0x2f] = 'v',
    [0x11] = 'w',
    [0x2d] = 'x',
    [0x15] = 'y',
    [0x2c] = 'z',
    [0x39] = 0x20, //space
    [0x02] = '1',
    [0x03] = '2',
    [0x04] = '3',
    [0x05] = '4',
    [0x06] = '5',
    [0x07] = '6',
    [0x08] = '7',
    [0x09] = '8',
    [0x0a] = '9',
    [0x0b] = '0',   
    [0x0c] = '-', 
    [0x0d] = '=',
    [0x1a] = '[',
    [0x1b] = ']',
    [0x27] = ';',
    [0x28] = 0x27,   // ' 
    [0x2b] = 0x5c,   // \ 
    [0x33] = ',', 
    [0x34] = '.',  
    [0x35] = '/',  
    [0x0f] = 0x09 // tab
};

//TODO
int keyShiftMapping[] = {};


int keyDownMapping[185] = {

	[0x1e] = 71,    // a
    [0x30] = 1,    // b
    [0x2e] = 2,    // c
    [0x20] = 3,   // d
    [0x12] = 4,   // e
    [0x21] = 5,   // f
    [0x22] = 6,   // g
    [0x23] = 7,   // h
    [0x17] = 8,   // i
    [0x24] = 9,   // j
    [0x25] = 10,  // k
    [0x26] = 11,  // l
    [0x32] = 12,  // m
    [0x31] = 13,  // n
    [0x18] = 14,  // o
    [0x19] = 15,  // p
    [0x10] = 16,  // q
    [0x13] = 17,  // r
    [0x1f] = 18,  // s
    [0x14] = 19,   // t
    [0x16] = 20,   // u
    [0x2f] = 21,   // v
    [0x11] = 22,   // w
    [0x2d] = 23,   // x
    [0x15] = 24,   // y
    [0x2c] = 25,   // z
    [0x39] = 26,   // space
    [0x2a] = 27,   // left shift
    [0x36] = 28,   // right shift
    [0x38] = 29,   // left alt
    [0xb8] = 30,   // right alt
    [0x1d] = 31,   // control
    [0x1c] = 32,   // enter
    [0x01] = 33,   // escape
    [0x3b] = 34,   // F1
    [0x3c] = 35,   // F2
    [0x3d] = 36,   // F3
    [0x3e] = 37,   // F4
    [0x3f] = 38,   // F5
    [0x40] = 39,   // F6
    [0x41] = 40,   // F7
    [0x42] = 41,   // F8
    [0x43] = 42,   // F9
    [0x44] = 43,   // F10
    [0x57] = 44,   // F11
    [0x58] = 45,   // F12
    [0x48] = 46,   // up arrow
    [0x50] = 47,   // down arrow
    [0x4b] = 48,   // left arrow
    [0x4d] = 49,   // right arrow
    [0x02] = 50,   // 1 (!)
    [0x03] = 51,   // 2 (@)
    [0x04] = 52,   // 3 (#)
    [0x05] = 53,   // 4 ($)
    [0x06] = 54,   // 5 (%)
    [0x07] = 55,   // 6 (^)
    [0x08] = 56,   // 7 (&)
    [0x09] = 57,   // 8 (*)
    [0x0a] = 58,   // 9 (()
    [0x0b] = 59,   // 0 ())
    [0x0c] = 60,   // - (_)
    [0x0d] = 61,   // = (+)
    [0x1a] = 62,   // [ ({)
    [0x1b] = 63,   // ] (})
    [0x27] = 64,   // ; (:)
    [0x28] = 65,   // ' (")
    [0x2b] = 66,   // \ (|)
    [0x33] = 67,   // , (<)
    [0x34] = 68,   // . (>)
    [0x35] = 69,   // / (?)
    [0x0f] = 70   // tab
};

int keyUpMapping[217] = {

    // Alphanumeric keys
    [0x9e] = 71,    // a
    [0xb0] = 1,    // b
    [0xae] = 2,    // c
    [0xa0] = 3,    // d
    [0x92] = 4,    // e
    [0xa1] = 5,    // f
    [0xa2] = 6,    // g
    [0xa3] = 7,    // h
    [0x97] = 8,    // i
    [0xa4] = 9,    // j
    [0xa5] = 10,   // k
    [0xa6] = 11,   // l
    [0xb2] = 12,   // m
    [0xb1] = 13,   // n
    [0x98] = 14,   // o
    [0x99] = 15,   // p
    [0x90] = 16,   // q
    [0x93] = 17,   // r
    [0x9f] = 18,   // s
    [0x94] = 19,   // t
    [0x96] = 20,   // u
    [0xaf] = 21,   // v
    [0x91] = 22,   // w
    [0xad] = 23,   // x
    [0x95] = 24,   // y
    [0xac] = 25,   // z
    [0xb9] = 26,   // space
    [0xaa] = 27,   // left shift
    [0xb6] = 28,   // right shift
    [0xb8] = 29,   // left alt
    [0x9d] = 30,   // right alt
    [0x9c] = 31,   // control
    [0x81] = 32,   // enter
    [0x01] = 33,   // escape
    [0xbb] = 34,   // F1
    [0xbc] = 35,   // F2
    [0xbd] = 36,   // F3
    [0xbe] = 37,   // F4
    [0xbf] = 38,   // F5
    [0xc0] = 39,   // F6
    [0xc1] = 40,   // F7
    [0xc2] = 41,   // F8
    [0xc3] = 42,   // F9
    [0xc4] = 43,   // F10
    [0xd7] = 44,   // F11
    [0xd8] = 45,   // F12
    [0xc8] = 46,   // up arrow
    [0xd0] = 47,   // down arrow
    [0xcb] = 48,   // left arrow
    [0xcd] = 49,   // right arrow
    [0x82] = 50,   // 1 (!)
    [0x83] = 51,   // 2 (@)
    [0x84] = 52,   // 3 (#)
    [0x85] = 53,   // 4 ($)
    [0x86] = 54,   // 5 (%)
    [0x87] = 55,   // 6 (^)
    [0x88] = 56,   // 7 (&)
    [0x89] = 57,   // 8 (*)
    [0x8a] = 58,   // 9 (()
    [0x8b] = 59,   // 0 ())
    [0x8c] = 60,   // - (_)
    [0x8d] = 61,   // = (+)
    [0x9a] = 62,   // [ ({)
    [0x9b] = 63,   // ] (})
    [0xa7] = 64,   // ; (:)
    [0xa8] = 65,   // ' (")
    [0xab] = 66,   // \ (|)
    [0xb3] = 67,   // , (<)
    [0xb4] = 68,   // . (>)
    [0xb5] = 69,   // / (?)
    [0x8f] = 70   // tab
};


bool isKeyPressed(unsigned int index){

	if(index > NUM_KEYS){

		//print()
	}

	return (keylog[index] == 1);
}

int codeToAscii(unsigned short code) {

	//default key code

	if(isKeyPressed(LFT_SHFT)){

		//return upper case mapping
	}

	if(keyDownMapping[code] != 0){

		return keyMapping[code];

	}

}

void onKeyPressed(unsigned short code){

	int index = keyDownMapping[code];

	keylog[index] = 1;
}


void onKeyReleased(unsigned char code){

	int index = keyUpMapping[code];

	keylog[index] = 0;
}
