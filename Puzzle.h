#if !defined(AFX_PUZZLE_H__DC06699E_BD8E_438E_8DAC_C67F2A179503__INCLUDED_)
#define AFX_PUZZLE_H__DC06699E_BD8E_438E_8DAC_C67F2A179503__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define NUMPIECES 12
#define LASTPIECE 11			// one piece is preset into the box
#define MAXNUMCUBES 6			// The biggest part has six cubes
#define ASYMMETRIC_PIECE 8		// we choose one asymmetric piece to
								// avoid symmetric degeneration 
								// however it does not need to be completely
								// asymmetric (like part number 9)

class CPuzzle  
{
	int *pMem;							// this will point to the pieces bits
	int AsymmetricPieceTotalRotations;

public:

	CPuzzle();
	~CPuzzle();

	void Solve();
};

#endif // !defined(AFX_PUZZLE_H__DC06699E_BD8E_438E_8DAC_C67F2A179503__INCLUDED_)
