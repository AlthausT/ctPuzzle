#include <stdio.h>
#include <windows.h>

#include "Point.h"
#include "Piece.h"
#include "Puzzle.h"


extern "C" int Solutions;


void main()
{
	LARGE_INTEGER pc1, pc2, MSFrequency;

	QueryPerformanceCounter(&pc1);

	CPuzzle* pPuzzle = new CPuzzle();
	pPuzzle->Solve();
	delete pPuzzle;

	QueryPerformanceCounter(&pc2);

	QueryPerformanceFrequency(&MSFrequency);
	double t = (double)(pc2.QuadPart-pc1.QuadPart) / (double)MSFrequency.QuadPart;

	printf("\nSolutions: %i (%f seconds)\n",Solutions,t);
}