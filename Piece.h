#if !defined(AFX_PIECE_H__204B6BD7_A5A9_4AEE_9C6C_C7E36E16985B__INCLUDED_)
#define AFX_PIECE_H__204B6BD7_A5A9_4AEE_9C6C_C7E36E16985B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CPiece  
{
	int NumCubes;
	int NumRotations;				  // num rotations (up to 24)	
	int TotalNumRotations;            // sum over all locations
	CPoint *pRotation;
	int *pRotationListXYZ[3*4*5];
	int	NumRotationsXYZ[3*4*5];

public:
	CPiece(int NumberOfCubes, int *Coordinates);
	~CPiece();

	int GetNumCubes() { return NumCubes; }
	int GetNumRotations() { return NumRotations; }
	int GetTotalNumRotations() { return TotalNumRotations; }
	void SetNumRotations(int x) { NumRotations = x; } 
	void CreateRotations();
	void CreateTranslations();
	bool BoxFit(int x, int y, int z, int rot);
	void AddRotation(CPoint *pNewRotation);
	bool Compare(CPoint *p1, CPoint *p2);
	void SortCubes(CPoint *pCubes);
	void SymmetryElimination();
	bool CheckRotation(int rot, int x, int y, int z);
	void DeleteRotation(int index, int adr);
	bool GetDegeneration(int i, int adr);
	CPoint *GetRotation(int rot, int cube) { return &(pRotation[GetNumCubes()*rot + cube]); }
	unsigned __int64 GetRotation64(int index, int adr);

	inline int GetNumRotationsXYZ(int adr)
	{
		return NumRotationsXYZ[adr];
	}

	inline int GetRotationNumber(int index, int adr)
	{
		return (pRotationListXYZ[adr])[index] & 255;
	}
};

#endif // !defined(AFX_PIECE_H__204B6BD7_A5A9_4AEE_9C6C_C7E36E16985B__INCLUDED_)
