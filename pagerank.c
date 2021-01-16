#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "engine.h"
#define BUFSIZE 100
#define convert 48

char** read_txt(FILE* maze_file, int dimension);

int main(void) {

	// Variables

	FILE* ptr;
	int dimension = 0;
	char buffer_line[BUFSIZE];
	int row, column;
	double temp1[BUFSIZE][BUFSIZE], temp2[BUFSIZE][BUFSIZE];
	char** matrix = NULL;
	double temp3[BUFSIZE * BUFSIZE];
	

	// MATLAB Variables

	Engine* ep = NULL; // A pointer to a MATLAB engine object

	mxArray* testArray = NULL, * result = NULL; // mxArray is the fundamental type underlying MATLAB data


	// Open file and check for error
	ptr = fopen("web.txt", "r");
	if (ptr == NULL)
	{
		printf("Error!");

	}


	//finding the dimensions
	fgets(buffer_line, BUFSIZE, ptr);
	dimension = (int)strlen(buffer_line) / 2;


	printf("dimension: %d", dimension);

	printf("\n");
	printf("\n");

	if (dimension > 0) // read into matrix from file, cast it into double and store in temp1
	{
		matrix = read_txt(ptr, dimension);
		for (row = 0; row < dimension; row++)
		{
			for (column = 0; column < dimension; column++)
			{
				temp1[row][column] = (double)matrix[row][column]-convert;
			}
		}
	}

	printf("The input Matrix is:"); // print the input matrix
	printf("\n");

	for (row = 0; row < dimension; row++)
	{
		for (column = 0; column < dimension; column++)
		{
			temp2[column][row] = temp1[row][column]; // reverse order and save it because MATLAB reads matrices in opposite order as that in C
		}
	}

	for (row = 0; row < dimension; row++)		//printing the input matrix
	{
		for (column = 0; column < dimension; column++)
		{
			printf("%lf ", temp1[row][column]);
		}
		printf("\n");
	}
	int i=0;
	for (row = 0; row < dimension; row++)		//finding the transpose of the matrix
	{
		for (column = 0; column < dimension; column++) {
			temp3[i] = temp2[row][column];
			i++;
		}
	}
	
	// Check if MATLAB Engine opens
	if (!(ep = engOpen(NULL))) {

		fprintf(stderr, "\nCan't start MATLAB engine\n");

		system("pause");

		return 1;

	}

	testArray = mxCreateDoubleMatrix(dimension, dimension, mxREAL); // Create MATLAB matrix variable testArray
	memcpy((void*)mxGetPr(testArray), (void*)temp3, dimension*dimension * sizeof(double));

	// error checking in writing test array to MATLAB
	if (engPutVariable(ep, "testArray", testArray)) { 

		fprintf(stderr, "\nCannot write test array to MATLAB \n");

		system("pause");

		exit(1); // Same as return 1;

	}

	// Check for error in calculating rows and columns
	if (engEvalString(ep, "[rows, columns] = size(testArray)")) {
		fprintf(stderr, "\nError calculating rows and columns  \n");
		system("pause");
		exit(1);
	}

	// Check for error calculating dimensions
	if (engEvalString(ep, "dimension = size(testArray , 1)")) {
		fprintf(stderr, "\nError calculating dimension  \n");
		system("pause");
		exit(1);
	}

	// Check for error calculating columnsums
	if (engEvalString(ep, "columnsums = sum(testArray , 1)")) {
		fprintf(stderr, "\nError calculating coulumnsums  \n");
		system("pause");
		exit(1);
	}

	// Check for error in entering probability factor
	if (engEvalString(ep, "p = 0.85")) {
		fprintf(stderr, "\nError entering probability factor \n");
		system("pause");
		exit(1);
	}

	// Check for error in finding non zero columnsums
	if (engEvalString(ep, "zerocolumns = find(columnsums~=0)")) {
		fprintf(stderr, "\nError finding columnsums non zero sum  \n");
		system("pause");
		exit(1);
	}

	// Check for error calculating sparse
	if (engEvalString(ep, "D = sparse( zerocolumns, zerocolumns, 1./columnsums(zerocolumns), dimension, dimension )")) {
		fprintf(stderr, "\nError calculating sparse   \n");
		system("pause");
		exit(1);
	}


	// Check for error calculating stochastic matrix
	if (engEvalString(ep, "StochasticMatrix = testArray * D")) {
		fprintf(stderr, "\nError calculating Stochastic matrix  \n");
		system("pause");
		exit(1);
	}

	// Check for error finding columnsums zero sum
	if (engEvalString(ep, "[row, column] = find(columnsums==0)")) {
		fprintf(stderr, "\nError finding columnsums zero sum  \n");
		system("pause");
		exit(1);
	}

	// Check for error generating stochastic matrix
	if (engEvalString(ep, "StochasticMatrix(:, column) = 1./dimension")) {
		fprintf(stderr, "\nError generating Stochastic matrix \n");
		system("pause");
		exit(1);
	}

	// Check for error generating Q
	if (engEvalString(ep, "Q = ones(dimension, dimension)")) {
		fprintf(stderr, "\nError calculating  Q \n");
		system("pause");
		exit(1);
	}


	// Check for error generating transition matrix
	if (engEvalString(ep, "TransitionMatrix = p * StochasticMatrix + (1 - p) * (Q/dimension)")) {
		fprintf(stderr, "\nError calculating Transition matrix \n");
		system("pause");
		exit(1);
	}

	// Check for error generating pagerank
	if (engEvalString(ep, "PageRank = ones(dimension, 1)")) {
		fprintf(stderr, "\nError calculating Page rank \n");
		system("pause");
		exit(1);
	}

	// Check for error in MATLAB for loop
	if (engEvalString(ep, "for i = 1:100 PageRank = TransitionMatrix * PageRank; end")) {
		fprintf(stderr, "\nError calculating for Page rank in loop");
		system("pause");
		exit(1);
	}

	// Check for error generating normalized Page rank vector
	if (engEvalString(ep, "PageRank = PageRank / sum(PageRank)")) {
		fprintf(stderr, "\nError calculating normalized Page rank vector \n");
		system("pause");
		exit(1);
	}

	// Check for error in generating result
	if ((result = engGetVariable(ep, "PageRank")) == NULL) {
		fprintf(stderr, "\nFailed to retrieve PageRank\n");
		system("pause");
		exit(1);
	}

	// Print the result
	else {
		size_t size = mxGetNumberOfElements(result);
		size_t i = 0;
		printf("The Page Ranks are:\n\n");
		for (i = 0; i < size; i++) {
			printf(" Page %d: %f \n", i + 1, *(mxGetPr(result) + i));
		}
	}

	char buffer[BUFSIZE + 1];



	if (engOutputBuffer(ep, buffer, BUFSIZE)) {

		fprintf(stderr, "\nCan't create buffer for MATLAB output\n");

		system("pause");

		return 1;

	}

	buffer[BUFSIZE] = '\0';



	// Closing MATLAB engine and NULLING out pointers
	engEvalString(ep, "whos");

	printf("%s\n", buffer);

	mxDestroyArray(testArray);
	mxDestroyArray(result);

	testArray = NULL;
	result = NULL;

	if (engClose(ep)) {

		fprintf(stderr, "\nFailed to close MATLAB engine\n");

	}





	system("PAUSE");

	return 0;

}




char** read_txt(FILE* test_file, int dimension)
{

	char line_buffer[BUFSIZE];
	int row = 0;
	int column = 0;
	char** matrix = NULL;
	int t = 0;

	matrix = (char**)calloc(dimension, sizeof(char) * dimension);

	rewind(test_file);

	for (row = 0; row < dimension; ++row) {
		matrix[row] = (char*)calloc(dimension, sizeof(char) * dimension);
	}


	row = 0;
	while (fgets(line_buffer, BUFSIZE, test_file)) {
		for (column = 0; column < dimension*2; ++column) {
			if (line_buffer[column] != 32)
			{
				matrix[row][t] = line_buffer[column];
				t++;
			}
		}
		row++;
		t = 0;
	}

	

	return matrix;
}