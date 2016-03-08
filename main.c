#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

//I guess this is what word uses to split tables
#define SPLIT 0x7

#ifdef DEBUG
	FILE *debug;
#	define dPrint(...) fprintf(debug, __VA_ARGS__);
#else
#	define dPrint(...) ;
#endif

#ifdef _WIN32
#	include <conio.h>
#else
int _getch(){
	char next;
	char ret = next = fgetc(stdin);
	while(next != '\n' && next != EOF){next = fgetc(stdin);}
	(void) next;
	return ret;
}
#endif


//replacement for strstr when neither string is null terminated properly
//returns offset where it is found or NULL
int findStr(const char* buffer, int blen, const char* findstr, int findstrlen){
	findstrlen--;
	int findlength, i;

	for(i = 0; i < blen; i++){
		for(findlength = 0; i < blen; i++, findlength++){
			if(findstr[findlength] != buffer[i]) break;
			/*printf("cmp %i + %i: %c + %c %.*s\n",*/
				/*i, startfind,*/
				/*buffer[i], findstr[i - startfind],*/
				/*i - startfind, buffer + startfind*/
			/*);*/
			if(findlength == findstrlen) return i;
		}
	}
	return -1;
}

#define GLOSSARYSIZE (75 + 1) //+1 for free space
#define GLOSSARYSTRSIZE 60
#define GLOSSARYXSTRSIZE 150 //Glossaryx > glossary

char glossary[GLOSSARYSIZE][GLOSSARYSTRSIZE];
char glossaryx[GLOSSARYSIZE][GLOSSARYXSTRSIZE];
FILE *glossaryxFile;
void readGlossary(){
	for(int i = 0; i < GLOSSARYSIZE; i++){
		glossary[i][0] = '\0';
		glossaryx[i][0] = '\0';
	}
	FILE *in = fopen("glossary.txt", "r");
	char inbuffer[GLOSSARYXSTRSIZE + 3]; //3 characters for the number identifier and space
	char outbuffer[GLOSSARYXSTRSIZE];
	int index;
	while(fgets(inbuffer, GLOSSARYSTRSIZE + 3, in)){
		sscanf(inbuffer, "%d %[^\t\n]", &index, outbuffer);
		strcpy(glossary[index], outbuffer);
	}

	glossaryxFile = fopen("glossaryx.txt", "a+");
	while(fgets(inbuffer, GLOSSARYXSTRSIZE + 3, glossaryxFile)){
		sscanf(inbuffer, "%d %[^\t\n]", &index, outbuffer);
		strcpy(glossaryx[index], outbuffer);
	}

	dPrint("====GLOSSARY====\n");
	for(int i = 0; i < GLOSSARYSIZE; i++){
		dPrint("%d %s", i, glossary[i]);
		if(glossaryx[i]){
			dPrint(" ANS: %s\n", glossaryx[i]);
		}
	}
	fclose(in);
}

enum clueState{
	CLUE_FALSE = 0,
	CLUE_TRUE,
	CLUE_MABYE
};

struct board{
	int id[5][5];
	enum clueState data[5][5];
} board;

void readBoard(int boardnum){
	char buffer[40];
	sprintf(buffer, "cards/%d.txt", boardnum); //First used as file name
	printf("Loading board %d, %s\n", boardnum, buffer);
	FILE *in = fopen(buffer, "r");
	if(!in){
		printf("board not defined");
		exit(0);
	}
	int digits[5];
	for(int i = 0; i < 5; i++){
		fgets(buffer, 40, in);
		sscanf(buffer, "%d %d %d %d %d",
			&digits[0], &digits[1],
			&digits[2], &digits[3],
			&digits[4]
		);
		for(int j = 0; j < 5; j++){
			board.id[i][j] = digits[j];
			if(glossary[digits[j]][0] == '\0'){
				printf("Unknown: %d\n", digits[j]);
			}
		}
	}
	printf("done");
	fclose(in);

	//Set every slot to CLUE_FALSE
	memset(board.data, 0, sizeof(board.data));
}

//Implemented as linked list
struct clue{
	int col;
	char first;
	char *text;
	struct clue *next;
};

struct clue *cclue, *cluestart;

void resetClue(void){
	cclue = cluestart;
}
void addClue(void){
	struct clue *newclue = calloc(1, sizeof *newclue);
	if(!cluestart){
		cluestart = newclue;
		resetClue();
	}else{
		cclue->next = newclue;
		cclue = cclue->next;
	}
}

void doFile(char filename[]){
	FILE *in = fopen(filename, "r");

	fseek(in, 0, SEEK_END);
	int filesize = ftell(in);
	fseek(in, 0, SEEK_SET);

	char *buffer = malloc(filesize);
	fread(buffer, 1, filesize, in); //read whole file can be optimized
	fclose(in);

	dPrint("filesize is %i\n", filesize);

	int start = findStr(buffer, filesize, "answer", 6) + 1; //+1 for space after answer
	if(start < 0) return; //not found

	dPrint("Starting buffer\n");

	/*printf("%.*s\n\n", endpos - startpos, buffer + startpos);*/


	int i, cStep = 0, starti = 0, strSize = 0;
	for(i = start; buffer[i] != '\0'; i++){ //can start at one because it works backwards
		if(buffer[i] == SPLIT){
		switch(cStep++){
		case 0:
			addClue();
			switch(buffer[i - 1]){
			case 'B':
				cclue->col = 0;
				break;
			case 'I':
				cclue->col = 1;
				break;
			case 'N':
				cclue->col = 2;
				break;
			case 'G':
				cclue->col = 3;
				break;
			case 'O':
				cclue->col = 4;
				break;
			default:
				cclue->col = 8;
			};
			starti = i + 1;
			break;
		case 1:
			//Includes the SPLIT character which will be replaced with a null terminator
			strSize = i - starti;
			cclue->text = malloc(strSize + 1);
			memcpy(cclue->text, buffer + starti, strSize);
			cclue->text[strSize] = '\0';
			break;
		case 2:
			cclue->first = buffer[i - 1];
			break;
		case 3:
			//Buffer should always have the same pattern of col|clue|first||col|clue|first...
			if(buffer[i + 2] != SPLIT) goto CLUESCOMPLETE;
			cStep = 0;
			dPrint("Made it to next clue\n\n");
			break;
		}
		}
	}
	CLUESCOMPLETE:;
	free(buffer);
}


int main(int argc, char **argv){
#ifdef DEBUG
	debug = fopen("debug.txt", "w");
#endif
	static const char *BINGO = "BINGO";
	{
		int board;
		if(argc == 1){
			printf("Enter your board number(Under free space): ");
			scanf("%d", &board);
		}else{
			board = atoi(argv[1]);
		}
		printf("Using board %d.\n", board);
		readGlossary();
		readBoard(board);
	}


	DIR *directory;
	struct dirent *dir;
	directory = opendir("./clues/");


	while((dir = readdir(directory)) != NULL){ //!includes . and ..
		if(dir->d_name[0] == '.') continue;

		dPrint(
			"============================================================\n"
			"=%56s  =\n"
			"============================================================\n",
		dir->d_name);

		char filename[256];
		strcpy(filename, "./clues/");
		strcat(filename, dir->d_name);
		doFile(filename);
	}
	closedir(directory);

	resetClue();
	printf("=-=-=-=-=-=-=-=-=-=-=-=-=-=\n Clue #: Category | First\n Clue\n=-=-=-=-=-=-=-=-=-=-=-=-=-=\n");
	int numClues = 0;
	while(cclue){
		
		int answer = 0;
		//Search glossaryx for current clue. answer will be 0 if not found.
		for(int j = 0; j < GLOSSARYSIZE; j++){
			if(!strncmp(glossaryx[j], cclue->text, GLOSSARYXSTRSIZE - 1)){
				answer = j;
				break;
			}
		}
		numClues++;
		if(answer){
			printf("'%s' clue found.\n", glossary[answer]);
			for(int j = 0; j < 5; j++){
				if(board.id[j][cclue->col] == answer){
					//Display human readable format and push data
					printf("Added at board pos %c%d, (%d, %d)\n", BINGO[cclue->col], answer, cclue->col + 1, j + 1);
					board.data[j][cclue->col] = CLUE_TRUE;
				}
			}
		}else{
			//Ask the user about the possible clues
			//Only need to ask about stuff in the correct row with the same first letter
			int hasprinted = 0;
			for(int j = 0; j < 5; j++){
				int cid = board.id[j][cclue->col];
				char *word = glossary[cid];
				if(word[0] == cclue->first && glossaryx[cid] != '\0'){
					if(!hasprinted){
						hasprinted = 1;
						printf("\n===========================\n %.2i: Column %c | Starts with %c\n  %s\n===========================\n",
							numClues,
							BINGO[cclue->col],
							cclue->first,
							cclue->text
						);
					}
					printf("Is this word %d, '%s'? (y/n/m) ", cid, word);
					char inp;
			FAILIN: inp = _getch();
					//Only allow yes or no answers
					if(inp == 'y'){
						board.data[j][cclue->col] = CLUE_TRUE;
						//Add to glossary
						fprintf(glossaryxFile, "%.2d %s\n", cid, cclue->text);
						//No need to test the other things in the column
						break;
					}else if(inp == 'm'){
						board.data[j][cclue->col] = CLUE_MABYE;
					}else if(inp == 'n'){
						//do nothing
					}else{
						//Get new input
						goto FAILIN;
					}
					printf("%c\n", inp);
				}
			}
		}
		cclue = cclue->next;
	}
	
	printf("\n============================\nFinal Answers: %d clues\n============================\n", numClues);
	
	//Display clues as list
	for(int i = 0; i < 5; i++){
		for(int j = 0; j < 5; j++){
			int id = board.id[i][j];
			switch(board.data[i][j]){
			case CLUE_FALSE:
				//Do nothing if the clue is not there
			break;
			case CLUE_TRUE:
				printf("%s\n", glossary[id]);
			break;
			case CLUE_MABYE:
				printf("%s??\n", glossary[id]);
				printf("Associated clue: '%s...'\n", glossaryx[id]);
			break;
			};
		}
	}
	
	//Free space enabled only after printing clues or else it prints "FREE SPACE"
	board.data[2][2] = CLUE_TRUE;
	
	//Display board in ascii
	for(int i = 0; i < 5; i++){
		for(int j = 0; j < 5; j++){
			switch(board.data[i][j]){
			case CLUE_FALSE:
				putchar('-');
			break;
			case CLUE_TRUE:
				putchar('X');
			break;
			case CLUE_MABYE:
				putchar('?');
			break;
			};
		}
		putchar('\n');
	}

	resetClue();
	//Clear list
	while(cclue){
		struct clue* next = cclue->next;
		free(cclue->text);
		free(cclue);
		cclue = next;
	}
#ifdef DEBUG
	fclose(debug);
#endif
	fclose(glossaryxFile);
	return 0;
}
