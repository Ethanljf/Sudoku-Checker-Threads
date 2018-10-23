//Program created by Ethan Foss. This program reads from an input file containing a completed sudoku puzzle
//and checks the validity of the solution using threads to check rows, columns and sub-grids.

//Qsort function implementation details were retrieved from:
//https://www.tutorialspoint.com/c_standard_library/c_function_qsort.htm
//This functionality was implemented in this program.

//Information detailing the returning of pointers from threads was retrieved from:
//https://cboard.cprogramming.com/c-programming/123884-return-value-thread.html
//This functionality was implemented in this program.

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>

void *checkSub(void *param);
void *checkRow(void *param);
void *checkCol(void *param);
int compareInt(const void *one, const void *two);

//Global variables

//2D array of integers to store the completed sudoku grid from the input file
static int grid[9][9];
//Array of values from 1 to 9 to check for valid solutions of rows, columns and sub-grids
static int solution[9] = {1,2,3,4,5,6,7,8,9};

//Integer pointers representing the result of solution checks to be returned by threads
static int *rowTest;
static int *colTest;
static int *subTest;


//Structure containing attributes of row and column numbers to be passed into threads
typedef struct{
	int row;
	int column; 
} parameters;


int main(void){

    //Allocate memory to global integer pointers
	rowTest = malloc(sizeof(int));
	colTest = malloc(sizeof(int));
	subTest = malloc(sizeof(int));

    //Initialize thread IDs for all threads to be created
    pthread_t tidRow;
    pthread_t tidCol;
    pthread_t tidSub[9];

    //Parse the input file "Sud_Sol.txt" and store values in their 
    //corresponding rows and columns until the EOF has been reached.
    int *check;
    int row = 0;
    int column = 0;
    char parse;
    FILE *sudoku;
    sudoku = fopen("Sud_Sol.txt", "r");
    if(sudoku){
    	while((parse = getc(sudoku)) != EOF){
    		if(parse == '\n'){
    			column = 0;
    			row++;
    		} else if(parse != ' '){
    			grid[row][column] = parse - '0';
    			column++;
    		}
    	}
    	fclose(sudoku);
    //Print out an error message if the input file could not be located
    } else {
    	printf("Error: Failed to read from specified file.\n");
    	exit(EXIT_FAILURE);
    }

    //Increment through all rows, checking for valid solutions.
    for(int i=0; i<9; i++){
    	parameters *data = (parameters *) malloc(sizeof(parameters));
		data->row = i;
		data->column = 0;
        //Create a thread with the specified data struct 
    	pthread_create(&tidRow, NULL, checkRow, data);
        //Join the thread with the parent and have it return the test result
    	pthread_join(tidRow, (void**)&check);
        //Free memory, print out an appropriate statement if the solution is erroneous 
        //and exit the program
    	if(*check == 0){
    		free(check);
    		free(data);
    		printf("This sudoku puzzle has NOT been solved correctly.\n");
    		exit(EXIT_FAILURE);
    	}
    	free(data);
	}

	free(check);

    //Increment through all columns, checking for valid solutions.
    for(int i=0; i<9; i++){
    	parameters *data = (parameters *) malloc(sizeof(parameters));
		data->column = i;
		data->row = 0;
        //Create a thread with the specified data struct 
    	pthread_create(&tidCol, NULL, checkCol, data);
        //Join the thread with the parent and have it return the test result
    	pthread_join(tidCol, (void**)&check);
        //Free memory, print out an appropriate statement if the solution is erroneous 
        //and exit the program
    	if(*check == 0){
    		free(check);
    		free(data);
    		printf("This sudoku puzzle has NOT been solved correctly.\n");
    		exit(EXIT_FAILURE);
    	}
    	free(data);
	}

	free(check);

	parameters *data = (parameters *) malloc(sizeof(parameters));

    //Increment through all sub-grids, checking for valid solutions.
	for(int i=0; i<9; i++){

        //If i is zero, check the top-left sub-grid
    	if(i == 0){
    		data->column = 0;
    		data->row = 0;
        //If the current column attribute is less than or equal to 3, 
        //move to the next sub-grid to the right
    	} else if(data->column <= 3){
			data->column += 3;
        //If the current sub-grid is the last in its row,
        //move to the first sub-grid on the next row
    	} else {
			data->column = 0;
			data->row += 3;
    	}
        //Create a thread with the specified data struct and the current i value as the 
        //element in the tidSub array
    	pthread_create(&tidSub[i], NULL, checkSub, data);
        //Join the thread with the parent and have it return the test result
    	pthread_join(tidSub[i], (void**)&check);
        //Free memory, print out an appropriate statement if the solution is erroneous 
        //and exit the program
    	if(*check == 0){
    		free(check);
    		free(data);
    		printf("This sudoku puzzle has NOT been solved correctly.\n");
    		exit(EXIT_FAILURE);
    	}
	}

    //If the program has not encountered any errors in the solution input file,
    //free the memory allocated to the data struct and print out a congratulatory message.
	free(data);
	printf("Congratulations! This sudoku puzzle has been solved!\n");
    return 0;
}

void *checkSub(void *param){
  parameters *dataStruct = param;
  int *subGrid = malloc(9 * sizeof(int));
  int z = 0;
  
  //Put the sub-grid values into subGrid array 
  for(int i=dataStruct->row; i<dataStruct->row+3; i++){
  	for(int x=dataStruct->column; x<dataStruct->column+3; x++){
  		subGrid[z] = grid[i][x];
  		z++;
  	}
  }

  //Sort values in subgrid array using the qsort function
  //with the compareInt method as the determining comparator
  qsort(subGrid, 9, sizeof(int), compareInt);
 
  //Compare values in sorted subgrid array to the correct values
	for(int i=0; i<9; i++){
        //If the values at the ith index do not match, return a 0
        //to represent a failed test.
		if(subGrid[i] != solution[i]){
			*subTest = 0;
			free(subGrid);
			pthread_exit(subTest);
		}
	}

    //Return a 1 to represent a passed test
	free(subGrid);
	*subTest = 1;
	pthread_exit(subTest);
}

void *checkRow(void *param){
	parameters *dataStruct = param;
	int *rowCopy = malloc(9 * sizeof(int));

    //Put the row values into the rowCopy array
	for(int i=0; i<9; i++){
		rowCopy[i] = grid[dataStruct->row][i];
	}
  
    //Sort values in rowCopy array using the qsort function
    //with the compareInt method as the determining comparator
  	qsort(rowCopy, 9, sizeof(int), compareInt);

	for(int i=0; i<9; i++){
        //If the values at the ith index do not match, return a 0
        //to represent a failed test.
		if(rowCopy[i] != solution[i]){
			*rowTest = 0;
			free(rowCopy);
			pthread_exit(rowTest);
		}
	}

    //Return a 1 to represent a passed test
	free(rowCopy);
	*rowTest = 1;
	pthread_exit(rowTest);
}

void *checkCol(void *param){
	parameters *dataStruct = param;
	int *colCopy = malloc(9 * sizeof(int));

    //Put the column values into the colCopy array
	for(int i=0; i<9; i++){
		colCopy[i] = grid[i][dataStruct->column];
	}

    //Sort values in colCopy array using the qsort function
    //with the compareInt method as the determining comparator
	qsort(colCopy, 9, sizeof(int), compareInt);

	for(int i=0; i<9; i++){
        //If the values at the ith index do not match, return a 0
        //to represent a failed test.
		if(colCopy[i] != solution[i]){
			*colTest = 0;
			free(colCopy);
			pthread_exit(colTest);
		}
	}

    //Return a 1 to represent a passed test
	free(colCopy);
	*colTest = 1;
	pthread_exit(colTest);
}

//Comparator function passed into qsort function call
int compareInt(const void *one, const void *two){
	return *(int*)one - *(int*)two;
}