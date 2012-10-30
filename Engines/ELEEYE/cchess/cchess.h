/*
cchess.h/cchess.cpp - Source Code for ElephantEye, Additional Part

ElephantEye - a Chinese Chess Program (UCCI Engine)
Designed by Morning Yellow, Version: 2.2, Last Modified: Apr. 2007
Copyright (C) 2004-2007 www.elephantbase.net

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "../base/base.h"
#include "../eleeye/position.h"

#ifndef CCHESS_H
#define CCHESS_H

void ChineseInit(bool bTraditional = false);
bool TryMove(PositionStruct &pos, int &nStatus, int mv);
void ExchangeSide(PositionStruct &pos);
void FlipBoard(PositionStruct &pos);
void BoardText(char *szBoard, const PositionStruct &pos, bool bAnsi = false);
void FenMirror(char *szFenDst, const char *szFenSrc);
uint32_t FileMirror(uint32_t dwFileStr);
uint32_t Chin2File(uint64_t qwChinStr);
uint64_t File2Chin(uint32_t dwFileStr, int sdPlayer);
int File2Move(uint32_t dwFileStr, const PositionStruct &pos);
uint32_t Move2File(int mv, const PositionStruct &pos);

// ���³����涨��"TryMove()"�ķ���״̬
const int MOVE_ILLEGAL = 256;       // ���Ϸ����ŷ�
const int MOVE_INCHECK = 128;       // �򽫾������Ϸ����ŷ�
const int MOVE_DRAW = 64;           // �����ŷ�(�Ա����Ϊ�Ϸ��ģ���ͬ)
const int MOVE_PERPETUAL_LOSS = 32; // ������ظ��ŷ�
const int MOVE_PERPETUAL_WIN = 16;  // �Է�������ظ��ŷ�
const int MOVE_PERPETUAL = 8;       // �ظ����ε��ŷ�
const int MOVE_MATE = 4;            // ����(��������)
const int MOVE_CHECK = 2;           // ����
const int MOVE_CAPTURE = 1;         // ����

#endif
