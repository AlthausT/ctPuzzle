#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include "Point.h"
#include "Piece.h"



CPiece::CPiece(int NumberOfCubes, int *Coordinates)
{
	int i;

	TotalNumRotations = 0;
	NumCubes = NumberOfCubes;
	pRotation = new CPoint[NumberOfCubes];

	for (i = 0; i < NumberOfCubes; i++)
	{
		pRotation[i].x = Coordinates[i*3];
		pRotation[i].y = Coordinates[i*3+1];
		pRotation[i].z = Coordinates[i*3+2];
	}
	SetNumRotations(1);
	SortCubes(&(pRotation[0]));

	for (i = 0; i < 3*4*5; i++) 
	{
		NumRotationsXYZ[i] = 0;
		pRotationListXYZ[i] = new int[24];
	}
}



CPiece::~CPiece()
{
	for (int i = 0; i < 3*4*5; i++) delete[] pRotationListXYZ[i];
	delete []pRotation;
}



static int compare( const void *arg1, const void *arg2 )
{
	CPoint *p1 = (CPoint*)arg1;
	CPoint *p2 = (CPoint*)arg2;
	int adr1 = (p1->x) + (p1->y)*256 + (p1->z)*65536;
	int adr2 = (p2->x) + (p2->y)*256 + (p2->z)*65536;
	return adr1 - adr2;
}



void CPiece::SortCubes(CPoint *pCubes)
{
	// we sort the cubes so that the first one has the lowest z,y,x
	qsort(pCubes,GetNumCubes(),sizeof(CPoint),compare);
}



bool CPiece::Compare(CPoint *p1, CPoint *p2)
{
	for (int i = 0; i < GetNumCubes(); i++)
	{
		if ( (p1[i].x != p2[i].x) || (p1[i].y != p2[i].y) || (p1[i].z != p2[i].z) )
			return false;
	}
	return true;
}



void CPiece::AddRotation(CPoint *pNewRotation)
{
	int i;
	for (i = 0; i < GetNumRotations(); i++)
	{
		if (Compare(pNewRotation,&(pRotation[i*GetNumCubes()]))) return;
	}
	CPoint *pNewMem = new CPoint[(GetNumRotations()+1)*GetNumCubes()];
	memcpy(pNewMem,pRotation,sizeof(CPoint)*GetNumRotations()*GetNumCubes());
	delete[] pRotation;
	pRotation = pNewMem;
	for (i = 0; i < GetNumCubes(); i++)
	{
		pRotation[GetNumRotations()*GetNumCubes()+i] = pNewRotation[i];
	}
	SetNumRotations(GetNumRotations()+1);
}



void CPiece::CreateRotations()
{
	int i;
	CPoint *pNewRotation = new CPoint[GetNumCubes()];

	for (int wz = 0; wz <= 270; wz += 90)
	{
		for (int wy = 0; wy <= 270; wy += 90)
		{
			for (int wx = 0; wx <= 270; wx += 90)
			{
				for (i = 0; i < GetNumCubes(); i++)
				{
					pNewRotation[i] = pRotation[i];
					pNewRotation[i].Rotate(wx,wy,wz,0.0f,0.0f,0.0f);
				}
				SortCubes(pNewRotation);
				CPoint FirstPoint = pNewRotation[0];
				for (i = 0; i < GetNumCubes(); i++)
				{
					pNewRotation[i].x -= FirstPoint.x;
					pNewRotation[i].y -= FirstPoint.y;
					pNewRotation[i].z -= FirstPoint.z;
				}
				AddRotation(pNewRotation);
			}
		}
	}
	delete[]pNewRotation;
}



inline bool CPiece::BoxFit(int x, int y, int z, int rot)
{
	CPoint* point;

	for (int cube = 0; cube < GetNumCubes(); cube++)
	{
		point = GetRotation(rot,cube);
		if ( (point->x+x < 0) || (point->x+x > 2) ||
			 (point->y+y < 0) || (point->y+y > 3) ||
			 (point->z+z < 0) || (point->z+z > 4) ) return false;
	}
	return true;
}



void CPiece::CreateTranslations()
{
	int adr = 0;
	for (int z = 0; z <= 4; z++)
	{
		for (int y = 0; y <= 3; y++)
		{
			for (int x = 0; x <= 2; x++)
			{
				for (int rot = 0; rot < GetNumRotations(); rot++)
				{
					if (BoxFit(x,y,z,rot))
					{
						int *p = pRotationListXYZ[adr];
						p[NumRotationsXYZ[adr]] = rot;
						NumRotationsXYZ[adr]++;
					}
				}
				TotalNumRotations += NumRotationsXYZ[adr];
				adr++;
			}
		}
	}
}


void CPiece::DeleteRotation(int index, int adr)
{
	if(NumRotationsXYZ[adr] == 0) return;
	int *p = pRotationListXYZ[adr];
	int Last = p[NumRotationsXYZ[adr] -1];
	p[index] = Last;
	assert(NumRotationsXYZ[adr] >= 1);
	NumRotationsXYZ[adr]--;
}



bool CPiece::GetDegeneration(int i, int adr)
{
	int *p = pRotationListXYZ[adr];
	return ((p[i] & 256) != 0);
}



void CPiece::SymmetryElimination()
{
	for (int z = 0; z <= 4; z++)
	{
		for (int y = 0; y <= 3; y++)
		{
			for (int x = 0; x <= 2; x++)
			{
				for (int i = 0; i < GetNumRotationsXYZ(x+y*3+z*12); i++)
				{
					int rot = GetRotationNumber(i,x+y*3+z*12);
					if (CheckRotation(rot,x,y,z))
					{
						int adr = x + y*3 + z*12;
						int *p = pRotationListXYZ[adr];
						p[i] |= 256;
					}
				}
			}
		}
	}
}


bool CPiece::CheckRotation(int rot, int x, int y, int z)
{
	int i, j;
	CPoint *pNewRotation = new CPoint[GetNumCubes()];
	CPoint *pLocalRotation = new CPoint[GetNumCubes()];
	bool degeneration = false;

	for (int omega = 1; omega <= 3; omega++)
	{
		int wx = (omega & 1) * 180;
		int wz = (omega >> 1) * 180;
		for (i = 0; i < GetNumCubes(); i++)
		{
			pNewRotation[i] = pRotation[rot*GetNumCubes() + i];
			pNewRotation[i].x += x;
			pNewRotation[i].y += y;
			pNewRotation[i].z += z;
			pNewRotation[i].Rotate(wx, 0, wz, 1.0f, 1.5f, 2.0f);
			pLocalRotation[i] = pRotation[rot*GetNumCubes() + i];
			pLocalRotation[i].x += x;
			pLocalRotation[i].y += y;
			pLocalRotation[i].z += z;
		}
		SortCubes(pNewRotation);
		if (Compare(pNewRotation,pLocalRotation)) degeneration = true;
		else
		{
			CPoint firstpoint = pNewRotation[0];
			for (i = 0; i < GetNumRotationsXYZ(firstpoint.x+firstpoint.y*3+firstpoint.z*12); i++)
			{
				int r2 = GetRotationNumber(i, firstpoint.x+firstpoint.y*3+firstpoint.z*12);
				for (j = 0; j < GetNumCubes(); j++)
				{
					pLocalRotation[j] = pRotation[GetNumCubes()*r2 + j];
					pLocalRotation[j].x += firstpoint.x;
					pLocalRotation[j].y += firstpoint.y;
					pLocalRotation[j].z += firstpoint.z;
				}
				if ( Compare(pNewRotation, pLocalRotation) )
					DeleteRotation(i, firstpoint.x+firstpoint.y*3+firstpoint.z*12);
			}
		}
	}

	delete[]pLocalRotation;
	delete[]pNewRotation;

	return degeneration;
}


unsigned __int64 CPiece::GetRotation64(int index, int adr)
{
	unsigned __int64 result = 0, eins = 1;
	CPoint *point;
	int lin, cube, r;

	r = GetRotationNumber(index,adr);

	for (cube = 0; cube < GetNumCubes(); cube++)
	{
		point = GetRotation(r,cube);
		lin = adr + point->x + point->y*3 + point->z*12;
		result |= (eins << lin);
	}
	return result;
}