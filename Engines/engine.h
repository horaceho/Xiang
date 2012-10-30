//
//  engine.h
//  Xiang
//
//  Created by Horace Ho on 2012/10/29.
//  Copyright (c) 2012 Horace Ho. All rights reserved.
//

#ifndef ENGINE_H
#define ENGINE_H

#ifdef __cplusplus
extern "C" {
#endif
    
const int FileLeft    = 3;
const int FileRight   = 11;
const int RankTop     = 3;
const int RankBottom  = 12;

void engineInit(const char *bookPath);
void engineQuit();

void engineSetFEN(const char *fen);
const char engineGetPieceAt(int file, int rank);
unsigned long engineThink(const int seconds);
    
#ifdef __cplusplus
}
#endif

#endif
