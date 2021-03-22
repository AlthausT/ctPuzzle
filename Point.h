#if !defined(AFX_POINT_H__40573398_0320_4EC6_9B76_6A2962031913__INCLUDED_)
#define AFX_POINT_H__40573398_0320_4EC6_9B76_6A2962031913__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CPoint  
{
public:
	int x;
	int	y;
	int z;
	
	void Rotate(int wx, int wy, int wz, double dx, double dy, double dz);
};

#endif // !defined(AFX_POINT_H__40573398_0320_4EC6_9B76_6A2962031913__INCLUDED_)