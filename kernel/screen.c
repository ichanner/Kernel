int textPos = 0;
int currLine = 0;

const int SCREEN_WIDTH = 80;
const int SCREEN_HEIGHT = 80;

// (row * screen_width + col) * 2

void print(char* message){

	char* video_memory = (char*)0xb8000;

	while (*message != '\0'){

		if(*message == '\n'){

			println();
		}
		else { 

			int pos = (currLine * SCREEN_WIDTH + textPos) * 2;

			video_memory[pos] = *message;
			video_memory[pos + 1] = 0x07;

			if (textPos >= SCREEN_WIDTH){

				textPos = 0;
				currLine++;
			}
			else{

				textPos++;
			}

		}

		message++;

	}

}

int getNumberOfDigits(int number){

	int counter = 0;

	while(number > 0){

		counter++;

		number = number / 10;
	}

	return counter;
}

void printi(int number){

	if(number == 0){

		print("0");

		return;
	}

	if(number < 0){

		print("-");

		number = (~number) + 1; // 2's complement to find abs
	}

	int num_digits = getNumberOfDigits(number);

	char digits[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
	char buffer[num_digits + 1]; 

	buffer[num_digits] = '\0';

	while(number > 0){

		buffer[num_digits - 1] = digits[number % 10];

		number = number / 10;

		num_digits--;
	}


	print(buffer);

}

void println(){

	textPos = 0;
	currLine++;

}