#include <stddef.h>

int strlen(char* str){

	int len = 0;;

	while(true) {

		if(str[len] == '\0') return len;

		len++;
	}
}


int stringToNumber(char* str) {

	int dec = 1;
	int acc = 0;

	for(int i = strlen(str) - 1; i >= 0; i--){

		if(str[i] == '0'){

			acc += dec * 0;
		}
		else if(str[i] == '1'){

			acc += dec * 1;
		}
		else if(str[i] == '2'){

			acc += dec * 2;
		}
		else if(str[i] == '3'){

			acc += dec * 3;
		}
		else if(str[i] == '4'){

			acc += dec * 4;
		}
		else if(str[i] == '5'){

			acc += dec * 5;
		}
		else if(str[i] == '6'){

			acc += dec * 6;
		}
		else if(str[i] == '7'){

			acc += dec * 7;
		}
		else if(str[i] == '8'){

			acc += dec * 8;
			
		}
		else if(str[i] == '9'){

			acc += dec * 9;
		}
		else if(str[i] == '-' && i == 0){

			return (acc * -1);
		}
		else {

			return; //invalid number 
		}

		dec *= 10;	
	}

	return acc;
}

char* strok(char* str, char delimiter) {
   
   	static char* last = NULL;

   	if(str != NULL){

   		last = str;
   	}

   	if (last == NULL || *last == '\0') {
        
        return NULL;
    }

   	char* start = last;

   	while(*last != '\0'){

   		if(*last == delimiter){

   			*last = '\0';

   			last++;

   			return start;
   		}

   		last++;
   	}

   	if(*start == '\0'){

   		return NULL;
   	}

   	return start;

}