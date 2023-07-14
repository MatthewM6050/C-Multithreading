/*@author Matthew Melendez
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include <semaphore.h>
#include <ctype.h>
#include <stdbool.h>

#define BUFFER_SIZE 10

// Global Variables
sem_t empty, full, replace, upper;
char buffer[BUFFER_SIZE];
bool endOfFile = false;


// Struct for passsing arguments into threads
struct arguments {
	char *fileName;
	char *replacementChar;
};


// Function for reading file thread
void *readFile(void *arg){
	// Local Variables
	struct arguments *argument;
	argument = (struct arguments*)arg;
	
	char *fileName = argument->fileName;
	FILE *file = fopen(fileName, "r+");// Opening file
	
	// Reading through file
	if(file != NULL){
		char line[BUFFER_SIZE];
		
		// Gets each 10 characters of the file
		while(fgets(line, BUFFER_SIZE, file) != 0){
			sem_wait(&empty); // Semaphore to check if buffer is empty
			strcpy(buffer, line);
			sem_post(&replace);// Semaphore to signal that buffer is full
		}
		if(feof(file)){
			fclose(file);
			endOfFile = true;
		}
	}
	else
		perror(fileName);
		
		pthread_exit(0);
}

//Function for char replacement
void *charReplace(void *arg){
	struct arguments *argument;
	argument = (struct arguments*)arg;
	char *repChar = argument->replacementChar;
	
	while(!endOfFile){
		sem_wait(&replace);
		for(int i = 0; i < strlen(buffer); i++){
			if(buffer[i] == ' ')
				buffer[i] = repChar;
		}
		sem_post(&upper);
	}
	pthread_exit(0);
}


void *upperCaseString(){
	char ch;
	
	while(!endOfFile){
		sem_wait(&upper);
		for(int i = 0; i < strlen(buffer); i++){
			ch = buffer[i];
			buffer[i] = toupper(ch);
		}
		sem_post(&full);
	}
	pthread_exit(0);
}


// Function for Writing File thread
void *writeContent(void *arg){
	struct arguments *argument;
	argument = (struct arguments*)arg;
	
	char *outputName = argument->fileName;
        FILE *file = fopen(outputName, "w+");
        char temp[BUFFER_SIZE] = "";
        
        while(!endOfFile){
        	sem_wait(&full);
        	strcpy(temp, buffer);
        	fprintf(file, "%s", buffer);// Prints string into new file
        	sem_post(&empty);
        	
        }
       	fclose(file);
	pthread_exit(0);
}	

void *producer(char *inputFile, char *outFile, char ch)
{
	//Variables
	char *buffer[BUFFER_SIZE];
	struct arguments *readArg, *writeArg, *charArg;
	pthread_t readThread, writeThread, charThread, toUpperThread;
	char fileName[20];
	char outputFile[20];
	
	strcpy(fileName, inputFile);
	strcpy(outputFile, outFile);
	
	readArg = (struct arguments *)calloc(1,sizeof(struct arguments));
	readArg->fileName = fileName;
	charArg = (struct arguments *)calloc(1,sizeof(struct arguments));
	charArg->replacementChar = ch;
	writeArg = (struct arguments *)calloc(1,sizeof(struct arguments));
	writeArg->fileName = outputFile;
	
	sem_init(&empty, 0, 1);
	sem_init(&replace, 0, 0);
	sem_init(&upper, 0, 0);
	sem_init(&full, 0, 0);
	
	pthread_create(&readThread, NULL, readFile, (void *)readArg);
	pthread_create(&charThread, NULL, charReplace, (void *)charArg);
	pthread_create(&toUpperThread, NULL, upperCaseString, NULL);
	pthread_create(&writeThread, NULL, writeContent, (void *)writeArg);
	pthread_join(readThread, NULL);
	pthread_join(charThread, NULL);
	pthread_join(toUpperThread, NULL);
	pthread_join(writeThread, NULL);
	sem_destroy(&full);
	sem_destroy(&empty);
	sem_destroy(&replace);
	sem_destroy(&upper);
	
	return 0;
}

void *consumer(){
	char ch;
	char inputFile[20];
	char outputFile[20];
	
	printf("\nPlease insert file to be copied, file for output and character to replace empty spaces:");
	scanf("%s %s %c", inputFile, outputFile, &ch);
	
	producer(inputFile, outputFile, ch);
}

int main(){
	consumer();
	
	return 0;
}
