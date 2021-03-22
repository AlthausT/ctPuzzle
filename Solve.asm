.686
.MODEL		FLAT

DATA		SEGMENT PUBLIC 'DATA' USE32
			EXTRN   _Solutions	: DWORD
			EXTRN	_tab		: DWORD
DATA		ENDS



CODE		SEGMENT	PUBLIC 'CODE' USE32
PUBLIC		_SolveIt

MatchTest	MACRO
			mov		eax,[esi]
			add		esi,8
			test	ebx,eax
			jz		Match
			cmp		dword ptr[esi],-1
			je		NewPiece  
			ENDM
			

_SolveIt	PROC	NEAR
			pushad
			mov		ebx,[esp+4+32]		; edi:ebx contain the box which we want to solve
			mov		edi,[esp+8+32]		;
			mov     edx,[esp+12+32]		; edx-bits tell us which pieces are available
			mov		esi,ebx
			not		esi
			bsf		ebp,esi				; search location were to start to add a piece
			call	FindNext
			popad
			ret
_SolveIt	ENDP
			
			align	16
FindNext	PROC	NEAR				; this is the recursive function which adds a piece
			mov		esi,ebp				; ebp contains the location where to add the piece
			add		esi,esi
			lea		ebp,[ebp*8+ebp]
			add		ebp,esi
			lea		ebp,[ebp*4 -4*4*11 + offset _tab]	; ebp*4*11 + _Tab, 4 bits unused
			xor		ecx,ecx				; ecx is the counter for the piece which we use
			mov		esi,edx				; edx contains the list of available pieces in binary form
			align	4
bitscan:	bsf		esi,esi				; find the next piece which is free
			jz		return
			add		ecx,esi
			mov		esi,[ecx*4 + ebp]	; load pointer where to find the piece data
			or		esi,esi				; null pointer means no rotation possible at this location
			jz		NewPiece
Next:		mov		eax,[esi]			; load lower 32 Bits of 64 Bit representation of the
			add		esi,8				; rotated piece in the box
			test	ebx,eax				; this is "AND" without destroying ebx
			jz		Match
			align	4
NoMatch:	cmp		dword ptr [esi],-1	; the end of the list for the piece is marked by "-1"
			jz		NewPiece
			MatchTest					; a maximum of 24 rotations of a total asymmetric
			MatchTest					; piece is possible
			MatchTest					; a good "compiler" unrolls the loop !	
			MatchTest
			MatchTest
			MatchTest
			MatchTest
			MatchTest
			MatchTest
			MatchTest
			MatchTest
			MatchTest
			MatchTest
			MatchTest
			MatchTest
			MatchTest
			MatchTest
			MatchTest
			MatchTest
			MatchTest
			MatchTest
			MatchTest
			MatchTest
			align	4
NewPiece:	add		ecx,1				; goto the next piece
			mov		esi,edx
			shr		esi,cl				; the already handled pieces are of no interest
			jnz		bitscan
return:		ret

			align	8
Match:		test	edi,[esi-4]			; low 32 bits matched, so try high 32 bits
			jnz		NoMatch
			pushad						; bingo, part matches
			or		ebx,eax				; set part (low 32 bits)
			or		edi,[esi-4]			; set part (high 32 bits)
			btr		edx,ecx				; mark the part as used in edx (binary part avail. list)
			mov		ebp,ebx				; search the next free location in the box
			not		ebp					; we can only search for "1"'s not for zeros		
			bsf		ebp,ebp				; seems to be faster than exspected ?
			jnz		LocFound			; zero found in the low order 32 bits
			mov     ebp,edi				; zero is in the higher 32 bits
			not		ebp
			bsf		ebp,ebp				; 
			jz		Solution			; if no zero was found the box is completely filled
			call	FindNext32			; Now we are in the 32 bit case
			popad						; reset part, reset availibility list (edx) and so on
			cmp		dword ptr [esi],-1	; see above
			jnz		Next
			add		ecx,1
			mov		esi,edx
			shr		esi,cl
			jnz		bitscan
			ret

			align	16
LocFound:	call	FindNext			; here we are still in the 64 bit case
Continue:	popad
			cmp		dword ptr [esi],-1
			jnz		Next
			add		ecx,1
			mov		esi,edx
			shr		esi,cl
			jnz		bitscan
			ret

			align	8
Solution:	lock	inc		dword ptr [ebp + offset _Solutions] ; at this point we have a 
			jmp		continue									; solution (on the stack)

FindNext	ENDP



MatchTest32	MACRO						; the following is the same as above but a
			mov		eax,[esi]			; special version for the 32 bit case
			add		esi,4
			test	edi,eax
			jz		Match32
			cmp		dword ptr[esi],-1
			je		NewPiece32  
			ENDM

			align	16
FindNext32	PROC	NEAR				; if 32 bits are enough we have a special function for this
			mov		esi,ebp
			add		esi,esi
			lea		ebp,[ebp*8+ebp]
			add		ebp,esi
			lea		ebp,[ebp*4 + -4*4*11 + 32*4*11 + offset _tab]
			xor		ecx,ecx
			mov		esi,edx
			align	4
bitscan32:	bsf		esi,esi
			jz		return	
			add		ecx,esi
			mov		esi,[ecx*4 + ebp]
			or		esi,esi
			jz		NewPiece32
			align	4
Next32:		mov		eax,[esi]
			add		esi,4
			test	edi,eax
			jz		Match32
			cmp		dword ptr [esi],-1
			jz		NewPiece32
			MatchTest32
			MatchTest32
			MatchTest32
			MatchTest32
			MatchTest32
			MatchTest32
			MatchTest32
			MatchTest32
			MatchTest32
			MatchTest32
			MatchTest32
			MatchTest32
			MatchTest32
			MatchTest32
			MatchTest32
			MatchTest32
			MatchTest32
			MatchTest32
			MatchTest32
			MatchTest32
			MatchTest32
			MatchTest32
			MatchTest32
			align	4
NewPiece32:	add		ecx,1
			mov		esi,edx
			shr		esi,cl
			jnz		bitscan32
return:		ret

			align	16
Match32:	pushad
			or		edi,eax
			btr		edx,ecx
			mov     ebp,edi
			not		ebp
			bsf		ebp,ebp
			jz		Solution32
			call	FindNext32	
Continue32:	popad
			cmp		dword ptr [esi],-1
			jnz		Next32
			add		ecx,1
			mov		esi,edx
			shr		esi,cl
			jnz		bitscan32
			ret

			align	8
Solution32:	lock	inc		dword ptr [ebp + offset _Solutions]	; at this point we have a 
			jmp		continue32									; Solution (on the stack)

FindNext32	ENDP

CODE		ENDS
			END
