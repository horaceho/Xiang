/*
QH2UCCI - a Qianhong to UCCI Protocol Adapter
Designed by Morning Yellow, Version: 1.6, Last Modified: Jun. 2006
Copyright (C) 2004-2006 www.elephantbase.net

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

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "../base/base.h"
#include "../base/base2.h"
#include "../base/parse.h"
#include "../base/pipe.h"
#include "../eleeye/ucci.h"

const int MAX_CHAR = 1024; // �����ļ�����󳤶�

// ICCS��ʽת����ǳ�쵽UCCI
inline uint32_t ICCS_QH_UCCI(const char *szIccs) {
  union {
    char c[4];
    uint32_t dw;
  } Ret;
  Ret.c[0] = szIccs[0] - 'A' + 'a';
  Ret.c[1] = szIccs[1];
  Ret.c[2] = szIccs[3] - 'A' + 'a';
  Ret.c[3] = szIccs[4];
  return Ret.dw;
}

// ICCS��ʽת����UCCI��ǳ��
inline void ICCS_UCCI_QH(char *szIccs, uint32_t dwMoveStr) {
  char *lpMoveStr;
  lpMoveStr = (char *) &dwMoveStr;
  szIccs[0] = lpMoveStr[0] - 'a' + 'A';
  szIccs[1] = lpMoveStr[1];
  szIccs[2] = '-';
  szIccs[3] = lpMoveStr[2] - 'a' + 'A';
  szIccs[4] = lpMoveStr[3];
  szIccs[5] = '\0';
}

// ��UCCI�����FEN��ת��Ϊǳ������FEN��
static void FenUcci2QH(char *lp, bool bBlackMoves) {
  while (*lp != ' ' && *lp != '\0') {
    switch (*lp) {
    case 'B':
      *lp = 'E';
      break;
    case 'N':
      *lp = 'H';
      break;
    case 'b':
      *lp = 'e';
      break;
    case 'n':
      *lp = 'h';
      break;
    default:
      break;
    }
    lp ++;
  }
  strcpy(lp, bBlackMoves ? " b - - - 1" : " w - - - 1");
}

PipeStruct pipePlugin;

// ��ǳ��������ָ��
inline void Adapter2QH(const char *szLineStr) {
  pipePlugin.LineOutput(szLineStr);
  printf("info lineoutput [%s]\n", szLineStr);
  fflush(stdout);
}

// ����ǳ�����ķ�����Ϣ
inline bool QH2Adapter(char *szLineStr) {
  if (pipePlugin.LineInput(szLineStr)) {
    printf("info lineinput [%s]\n", szLineStr);
    fflush(stdout);
    return true;
  } else {
    return false;
  }
}

inline void PrintLn(const char *sz) {
  printf("%s\n", sz);
  fflush(stdout);
}

// ������
int main(void) {
  int i, nLevel, nThinkTime;
  bool bQuit, bBlackMoves, bTimeOut;
  uint32_t dwMoveStr;
  FILE *fpIniFile;
  char *lp;
  int64_t llTime;
  UcciCommStruct UcciComm;
  char szIccs[8];
  char szIniFile[MAX_CHAR], szCommand[MAX_CHAR];
  char szLineStr[LINE_INPUT_MAX_CHAR];

  if (BootLine() != UCCI_COMM_UCCI) {
    return 0;
  }
  // �յ�"ucci"ָ���������¼�������

  // 1. ��ȡ�����������ļ�"QH2UCCI.INI"
  LocatePath(szIniFile, "QH2UCCI.INI");
  nLevel = 0;
  fpIniFile = fopen(szIniFile, "rt");
  if (fpIniFile == NULL) {
    PrintLn("uccierror");
    PrintLn("bye");
    return 0;
  }
  while (!feof(fpIniFile)) {
    fgets(szLineStr, MAX_CHAR, fpIniFile);
    lp = strchr(szLineStr, '\n');
    if (lp != NULL) {
      *lp = '\0';
    }
    lp = szLineStr;
    if (false) {
    } else if (StrEqvSkip(lp, "Command=")) {
      LocatePath(szCommand, lp);
    } else if (StrEqvSkip(lp, "Name=")) {
      printf("id name %s\n", lp);
      fflush(stdout);
    } else if (StrEqvSkip(lp, "Copyright=")) {
      printf("id copyright %s\n", lp);
      fflush(stdout);
    } else if (StrEqvSkip(lp, "Author=")) {
      printf("id author %s\n", lp);
      fflush(stdout);
    } else if (StrEqvSkip(lp, "User=")) {
      printf("id user %s\n", lp);
      fflush(stdout);
    } else if (StrEqvSkip(lp, "Level=")) {
      nLevel = Str2Digit(lp, 0, 99);
    }
  }
  fclose(fpIniFile);

  // 2. ��ʹ������������
  bQuit = bBlackMoves = false;
  pipePlugin.Open(szCommand);
  sprintf(szLineStr, "LEVEL %d", nLevel);
  Adapter2QH(szLineStr);
  while (!QH2Adapter(szLineStr)) {
    Idle();
  }
  Adapter2QH("BGTHINK OFF");
  while (!QH2Adapter(szLineStr)) {
    Idle();
  }
  PrintLn("option usemillisec type check default true");
  PrintLn("option dualtime type label");
  PrintLn("ucciok");

  // 3. ����UCCIָ��
  while (!bQuit) {
    switch (IdleLine(UcciComm, false)) {
    case UCCI_COMM_ISREADY:
      PrintLn("readyok");
      break;
    case UCCI_COMM_STOP:
      PrintLn("nobestmove");
      break;
    case UCCI_COMM_POSITION:
      bBlackMoves = (strstr(UcciComm.szFenStr, " b") != NULL);
      FenUcci2QH(UcciComm.szFenStr, bBlackMoves);
      sprintf(szLineStr, "FEN %s", UcciComm.szFenStr);
      Adapter2QH(szLineStr);
      while (!QH2Adapter(szLineStr)) {
        Idle();
      }
      if (UcciComm.nMoveNum > 0) {
        sprintf(szLineStr, "LOAD %d", UcciComm.nMoveNum);
        Adapter2QH(szLineStr);
        for (i = 0; i < UcciComm.nMoveNum; i ++) {
          ICCS_UCCI_QH(szIccs, UcciComm.lpdwMovesCoord[i]);
          Adapter2QH(szIccs);
          bBlackMoves = !bBlackMoves;
        }
        while (!QH2Adapter(szLineStr)) {
          Idle();
        }
      }
      break;
    case UCCI_COMM_BANMOVES:
      if (UcciComm.nBanMoveNum > 0) {
        sprintf(szLineStr, "BAN %d", UcciComm.nMoveNum);
        Adapter2QH(szLineStr);
        for (i = 0; i < UcciComm.nBanMoveNum; i ++) {
          ICCS_UCCI_QH(szIccs, UcciComm.lpdwBanMovesCoord[i]);
          Adapter2QH(szIccs);
        }
        while (!QH2Adapter(szLineStr)) {
          Idle();
        }
      }
      break;
    case UCCI_COMM_GO:
      switch (UcciComm.Go) {
      case UCCI_GO_DEPTH:
      case UCCI_GO_NODES:
        // ��������֧��"depth"��"nodes"˼��ģʽ
        PrintLn("nobestmove");
        break;
      case UCCI_GO_TIME_MOVESTOGO:
      case UCCI_GO_TIME_INCREMENT:
        // ��"time"˼��ģʽ�£������ʵ���ʱ����˼��
        if (UcciComm.Go == UCCI_GO_TIME_MOVESTOGO) {
          nThinkTime = UcciComm.nTime / UcciComm.nMovesToGo;
        } else {
          nThinkTime = UcciComm.nTime / 20 + UcciComm.nIncrement;
        }
        Adapter2QH("AI");
        llTime = GetTime();
        bTimeOut = false;
        while (!bQuit) {
          // �ȴ�˼���������Ҫ������¼��������ݣ�
          // (1) ����ʱ��
          if (!bTimeOut && (int) (GetTime() - llTime) > nThinkTime) {
            Adapter2QH("TIMEOUT");
            bTimeOut = true;
          }
          // (2) ����UCCIָ��
          switch (BusyLine(UcciComm, false)) {
          case UCCI_COMM_STOP:
            if (!bTimeOut) {
              Adapter2QH("TIMEOUT");
              bTimeOut = true;
            }
            break;
          case UCCI_COMM_QUIT:
            Adapter2QH("ABORT");
            bQuit = true;
            break;
          default:
            break;
          }
          // (3) ����ǳ�����ķ�����Ϣ
          if (QH2Adapter(szLineStr)) {
            if (StrEqv(szLineStr, "ERROR") || StrEqv(szLineStr, "ABORTED")) {
              PrintLn("nobestmove");
            } else {
              dwMoveStr = ICCS_QH_UCCI(szLineStr);
              printf("bestmove %.4s\n", (const char *) &dwMoveStr);
              fflush(stdout);
            }
            Adapter2QH("UNDO");
            while (!QH2Adapter(szLineStr)) {
              Idle();
            }
            break;
          } else {
            Idle();
          }
        }
        break;
      default:
        break;
      }
      break;
    case UCCI_COMM_QUIT:
      bQuit = true;
      break;
    default:
      break;
    }
  }

  // 4. �ر�ǳ�����͹ܵ�
  Adapter2QH("QUIT");
  llTime = GetTime();
  while ((int) (GetTime() - llTime) < 1000) {
    if (QH2Adapter(szLineStr)) {
      if (StrEqv(szLineStr, "BYE")) {
        break;
      }
    }
  }
  pipePlugin.Close();
  PrintLn("bye");
  return 0;
}
