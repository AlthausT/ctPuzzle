#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>

#include "Point.h"
#include "Piece.h"
#include "Puzzle.h"


typedef struct _THREADDATA
{
	int	 ThreadNum;
	unsigned __int64* pBox64;
	int	 adr;
	int  rot;
}	THREADDATA, *LPTHREADDATA;


static DWORD ThreadId[2]; 
static THREADDATA ThreadData[2];
static HANDLE ThreadHandle[2];
static HANDLE ReadyEvent[2];
static HANDLE ContinueEvent[2];


CPiece *pPiece[NUMPIECES];		// pointers to the 12 "monkeys"


extern "C" 
{
	int tab[3*4*5*NUMPIECES];	// in this table we store pointers were to find our piece data
	int Solutions;				// this is incremented by the SolveIt function

	void SolveIt(unsigned __int64 Box, int FreeForUse, int dummy); // dummy for a nice stackpointer
}


// this is the puzzle definition: the number of cubes and then the cube-coordinates (x,y,z)
// we put the "large" parts at the beginning with the hope to cut the tree early
static const int Cubes[NUMPIECES] = {6,5,5,5,5,5,5,5,5,4,5,5};
static const int Coordinate[MAXNUMCUBES*NUMPIECES*3] =
{
	0,0,0  ,  1, 0,0  ,  2, 0,0  ,  3, 0,0  ,  0, 1,0  , 2,1,0,
	0,0,0  ,  1, 0,0  ,  2, 0,0  ,  3, 0,0  ,  0, 1,0  , 0,0,0,
	0,0,0  ,  1, 0,0  ,  2, 0,0  ,  3, 0,0  ,  1, 1,0  , 0,0,0,
	0,0,0  ,  1, 0,0  ,  0, 1,0  ,  0, 2,0  ,  1, 2,0  , 0,0,0,
	0,0,0  ,  1, 0,0  ,  1, 1,0  ,  1, 2,0  ,  2, 2,0  , 0,0,0,
	0,0,0  ,  1, 0,0  ,  0, 1,0  ,  1, 1,0  ,  0, 1,1  , 0,0,0,
	0,0,0  ,  1, 0,0  ,  2, 0,0  ,  1, 1,0  ,  1, 2,0  , 0,0,0,
	0,0,0  ,  1, 0,0  ,  0, 1,0  ,  1, 1,0  , -1, 1,0  , 0,0,0,
	0,0,0  ,  1, 0,0  ,  0, 1,0  , -1, 1,0  , -1, 2,0  , 0,0,0,
	0,0,0  ,  1, 0,0  ,  0, 1,0  ,  0, 1,1  ,  0, 0,0  , 0,0,0,
	0,0,0  ,  0, 1,0  ,  1, 1,0  , -1, 1,0  , -1, 2,0  , 0,0,0,
	0,0,0  ,  0, 1,0  ,  0, 2,0  , -1, 1,0  ,  1, 1,0  , 0,0,0
};


CPuzzle::CPuzzle()
{
	int adr, PieceNr, HighSymmetryPiece, Min_NumRotations = INT_MAX;

	for (PieceNr = 0; PieceNr < NUMPIECES; PieceNr++)
	{
		// create the piece
		pPiece[PieceNr] = new CPiece(Cubes[PieceNr],(int*)&(Coordinate[6*3*PieceNr]));

		// create all unique rotations of the piece
		pPiece[PieceNr]->CreateRotations();

		// create the possible translations for the piece in the box
		pPiece[PieceNr]->CreateTranslations();

		if (PieceNr == ASYMMETRIC_PIECE) 
			AsymmetricPieceTotalRotations = pPiece[PieceNr]->GetTotalNumRotations();

		int NumRotations = pPiece[PieceNr]->GetNumRotations();
		if (NumRotations < Min_NumRotations)
		{
			Min_NumRotations = NumRotations;
			HighSymmetryPiece = PieceNr;		// the Highsymmetrypiece for this puzzle is "+"
		}
	}

	// we exchange the piece with the highest symmetry so that it is the last one
	CPiece* pTmp = pPiece[LASTPIECE];
	pPiece[LASTPIECE] = pPiece[HighSymmetryPiece];
	pPiece[HighSymmetryPiece] = pTmp;

	pPiece[LASTPIECE]->SymmetryElimination();	// avoid a factor of 4 for symmetric solutions

	// calculate the amount of memory needed for the 64bit representations of the pieces 
	int Ints_needed = 0;	
	for (adr = 0; adr < 60; adr++)
	{
		for (PieceNr = 0; PieceNr < NUMPIECES-1; PieceNr++)
		{
			int NumRotations = pPiece[PieceNr]->GetNumRotationsXYZ(adr);
			if (NumRotations > 0)
			{
				if(adr < 60 - 32) 
					Ints_needed += 2*NumRotations + 1;	// plus one for the end marker
				else 
					// if 32 bits are enough we only use 32 bits of course
					Ints_needed += NumRotations + 1;
			}
		}
	}

	pMem = new int[Ints_needed];
	int index = 0;

	// now store the 64Bit piece representations depending on the location in the box 
	for (adr = 0; adr < 60; adr++)
	{
		for (PieceNr = 0; PieceNr < NUMPIECES-1; PieceNr++)
		{
			int NumRotations = pPiece[PieceNr]->GetNumRotationsXYZ(adr);
			if (NumRotations == 0) 
				tab[adr*(NUMPIECES-1) + PieceNr] = NULL;
			else
			{
				tab[adr*(NUMPIECES-1) + PieceNr] = (int)&(pMem[index]);
				for (int i = 0; i < NumRotations; i++)
				{
					unsigned __int64 rot64 = pPiece[PieceNr]->GetRotation64(i,adr) << 4;
					if (adr < 60 - 32) pMem[index++] = (int)(0xffffffff & rot64);
					pMem[index++] = (int)(rot64 >> 32);
				}
				pMem[index++] = -1;				// this marks the end of the lists
			}
		}
	}
}


CPuzzle::~CPuzzle()
{
	for (int i = 0; i < NUMPIECES; i++)
	{
		delete pPiece[i];
	}
	delete[] pMem;
}



static unsigned __int64 Rotate64(unsigned __int64 arg, int wx, int wz)
{
	// rotate a 64bit Box by angles wx and wz
	CPoint point;
	unsigned __int64 result = 0, eins = 1;
	int x,y,z,adr = 0;	

	for (z = 0; z <= 4; z++)
		for(y = 0; y <= 3; y++)
			for (x = 0; x <= 2; x++)
			{
				if ((arg & (eins << adr)) != 0)
				{
					point.x = x;
					point.y = y;
					point.z = z;
					point.Rotate(wx,0,wz,1.0f, 1.5f, 2.0f);
					result |= (eins << (point.x + point.y*3 + point.z*12));
				}	
				adr++;
			}

	return result;
}


static void SymmetryElimination64(int *NumElements, unsigned __int64 *pBox64)
{
	// for a list of 64bit boxes eliminate symmetric ones
	unsigned __int64 last64, Box64_r, Box64;
	int i,index, found_index, NumValues = *NumElements;

	index = 0;
	do
	{
		Box64 = pBox64[index];
		for (int omega = 1; omega <= 3; omega++)
		{
			int wx = (omega & 1) * 180;
			int wz = (omega >> 1) * 180;
			Box64_r = Rotate64(Box64,wx,wz);
			found_index = -1;
			for (i = 0; (i < NumValues) && (found_index < 0); i++)
			{
				if (pBox64[i] == Box64_r) found_index = i;
			}
			if (found_index >= 0)
			{
				last64 = pBox64[NumValues-1];
				pBox64[found_index] = last64;
                NumValues--;
			}
		}
		index++;
	} while (index < NumValues);

	*NumElements = NumValues;
}




DWORD ThreadFunc(LPTHREADDATA pData)
{
	// this is the worker thread function
	// it creates all solutions for one position (adr) and rotation (rot) of the
	// highsymmetry-piece in the box

	int FreeForUse, adr, adr_2, rot, j, index, ThreadNum;
	unsigned __int64 Box64_2, Box64;

	FreeForUse = 0xfff ^ (1L << LASTPIECE);  // a bit "1" means the piece can be used
	ThreadNum = pData->ThreadNum;

	do
	{
		adr = pData->adr;
		rot = pData->rot;

		// if symmetric degeneration could happen we add another Piece (ASYMMETRIC_PIECE)
		if (pPiece[LASTPIECE]->GetDegeneration(rot,adr))
		{
			Box64 = pPiece[LASTPIECE]->GetRotation64(rot,adr);
			index = 0;
			for (adr_2 = 0; adr_2 < 60; adr_2++)
			{
				for (j = 0; j < pPiece[ASYMMETRIC_PIECE]->GetNumRotationsXYZ(adr_2); j++)
				{	
					Box64_2 = pPiece[ASYMMETRIC_PIECE]->GetRotation64(j,adr_2);
					if ((Box64 & Box64_2) == 0) pData->pBox64[index++] = (Box64 | Box64_2);
				}
			}
			SymmetryElimination64(&index, pData->pBox64);
			for (j = 0; j < index; j++)
				SolveIt( (pData->pBox64[j] << 4) | 15,FreeForUse ^ (1L << ASYMMETRIC_PIECE),0);
		}
		else
			// if no symmetric degeneration is possible we just start with the piece in the box	
			SolveIt((pPiece[LASTPIECE]->GetRotation64(rot,adr) << 4) | 15,FreeForUse,0);

		SetEvent(ReadyEvent[ThreadNum]);			// tell the mother thread that we are ready
		WaitForSingleObject(ContinueEvent[ThreadNum],INFINITE);  
		ResetEvent(ContinueEvent[ThreadNum]);

	} while (true);

	return(0);
}




void CPuzzle::Solve()
{
	int RunningThreads = 0;

	Solutions = 0;

	// we employ two threads to create all solutions (plus the mother thread)
	ThreadData[0].pBox64 = new unsigned __int64[AsymmetricPieceTotalRotations];
	ThreadData[1].pBox64 = new unsigned __int64[AsymmetricPieceTotalRotations];
	ThreadData[0].ThreadNum = 0;
	ThreadData[1].ThreadNum = 1;
	
	ReadyEvent[0] = CreateEvent(NULL,TRUE,FALSE,(LPCWSTR)"Thread0");
	ReadyEvent[1] = CreateEvent(NULL,TRUE,FALSE,(LPCWSTR)"Thread1");
	ContinueEvent[0]  = CreateEvent(NULL,TRUE,FALSE,(LPCWSTR)"Continue0");
	ContinueEvent[1]  = CreateEvent(NULL,TRUE,FALSE,(LPCWSTR)"Continue1");

	for (int adr = 0; adr < 60; adr++)
	{
		int NumRotations = pPiece[LASTPIECE]->GetNumRotationsXYZ(adr);
		for (int rot = 0; rot < NumRotations; rot++)
		{
			if (RunningThreads < 2)
			{
				ThreadData[RunningThreads].adr = adr;
				ThreadData[RunningThreads].rot = rot;
				ThreadHandle[RunningThreads] = CreateThread(NULL,0,
											(LPTHREAD_START_ROUTINE)ThreadFunc,
											&(ThreadData[RunningThreads]),
											0,&(ThreadId[RunningThreads]));
				RunningThreads++;
			}
			else
			{
				// the motherthread waits until one of the worker threads gets ready
				DWORD result = WaitForMultipleObjects(2,&(ReadyEvent[0]),FALSE,INFINITE);
				if( ( result >= WAIT_OBJECT_0) && (result <= WAIT_OBJECT_0 + 1))
				{
					int NumReady = result - WAIT_OBJECT_0;
					// give the thread the next piece address and rotation to work at
					ThreadData[NumReady].adr = adr;
					ThreadData[NumReady].rot = rot;
					ResetEvent(ReadyEvent[NumReady]);
					printf("%i ",Solutions);
					SetEvent(ContinueEvent[NumReady]);
				}
			}
		}
	}	

	// wait until both treads are ready	
	DWORD result = WaitForMultipleObjects(2,&(ReadyEvent[0]),TRUE,INFINITE);

	TerminateThread(ThreadHandle[0],0);
	TerminateThread(ThreadHandle[1],0);
	CloseHandle(ThreadHandle[0]);
	CloseHandle(ThreadHandle[1]);
	delete[]ThreadData[1].pBox64;
	delete[]ThreadData[0].pBox64;
}