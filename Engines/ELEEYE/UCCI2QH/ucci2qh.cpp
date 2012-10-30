/*
UCCI2QH - a UCCI to Qianhong Protocol Adapter
Designed by Morning Yellow, Version: 2.0, Last Modified: Apr. 2007
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

#include <stdio.h>
#include <string.h>
#include "../base/base2.h"
#include "../base/parse.h"
#include "../base/pipe.h"
#include "../eleeye/position.h"

const int MAX_CHAR = 1024;      // �����ļ�����󳤶�
const int MAX_IRREV_POS = 33;   // ������������������������ֳ��������ᳬ��32��
const int MAX_IRREV_MOVE = 200; // ��������������ŷ������������ŷ�����������100�غ�����
const int MAX_BAN_MOVE = 128;   // ���Ľ�ֹ�ŷ���
const int MAX_INFO = 16;        // �汾��Ϣ���������
const int MAX_OPTION = 16;      // ѡ�����õ��������
const int MAX_LEVEL = 16;       // �Ѷȵ���߼�����


/* ���³���������������˼��״̬����̨˼���Ĵ�����һ���ѵ㣺
 * (1) �����ú�̨˼��ʱ������״̬��"IDLE_NONE"������˼��״̬��"BUSY_THINK"����ʾ˼��״̬��"BUSY_HINTS"��
 * (2) ���ú�̨˼��ʱ������˼�������󣬾ͽ����̨˼��״̬(BUSY_PONDER)����"mvPonder"���ǲ²��ŷ���
 * (3) "BUSY_PONDER"״̬�£�������ָ������ŷ�û���ú�̨˼�����У����̨˼���жϣ�
 * (4) "BUSY_PONDER"״̬�£�������ָ������ŷ��ú�̨˼������(��"mvPonder"һ��)���ͽ����̨˼������״̬(BUSY_PONDERHIT)��
 * (5) "BUSY_PONDER"״̬�£������̨˼������(�ڶ��ָ����ŷ�֮ǰ)��������̨˼�����״̬(IDLE_PONDER_FINISHED)����"mvPonderFinished"�򱣴��̨˼���Ľ����
 * (6) "BUSY_PONDERHIT"״̬�£�����յ�˼��ָ���ת������˼��״̬(BUSY_THINK)��
 * (7) "BUSY_PONDERHIT"״̬�£������̨˼������(�ڶ��ָ����ŷ�֮ǰ)����ת���̨˼����ɲ�������״̬(IDLE_PONDERHIT_FINISHED)����"mvPonderFinished"�򱣴��̨˼���Ľ����
 * (8) "IDLE_PONDER_FINISHED"״̬�£�������ָ������ŷ�û���ú�̨˼�����У���������¿�ʼ˼����
 * (9) "IDLE_PONDER_FINISHED"״̬�£�������ָ������ŷ��ú�̨˼�����У���ת���̨˼����ɲ�������״̬(IDLE_PONDERHIT_FINISHED)��
 * (10) "IDLE_PONDERHIT_FINISHED"״̬�£�����յ�˼��ָ�����������"mvPonderFinished"�ŷ���
 */
const int IDLE_NONE = 0;
const int IDLE_PONDER_FINISHED = 1;
const int IDLE_PONDERHIT_FINISHED = 2;
const int BUSY_WAIT = 3;
const int BUSY_THINK = 4;
const int BUSY_HINTS = 5;
const int BUSY_PONDER = 6;
const int BUSY_PONDERHIT = 7;

static struct {
  // ������״̬ѡ��
  bool bDebug, bUcciOkay, bBgThink;       // �Ƿ����ģʽ��UCCI�����Ƿ���������̨˼���Ƿ�����
  int nLevel, nStatus;                    // �����״̬
  int mvPonder, mvPonderFinished;         // ��̨˼���Ĳ²��ŷ��ͺ�̨˼����ɵ��ŷ�
  int mvPonderFinishedPonder;             // ��̨˼��������˼������ĺ�̨˼���²��ŷ�
  // ������������Ϣ
  int nIrrevPosNum;                       // ��ǰ���������ĸ���
  PositionStruct posIrrev[MAX_IRREV_POS]; // ����������б���������������ڲ�����
  char szIrrevFen[MAX_IRREV_POS][128];    // ÿ�鲻����������ʼ�����FEN��
  int nBanMoveNum;                        // ��ֹ�ŷ�����������Ķ��������
  int wmvBanList[MAX_BAN_MOVE];           // ��ֹ�ŷ��б�
  // �������������ͨ��
  PipeStruct pipeStdin, pipeEngine;       // ��׼����(ǳ������ָ��)��UCCI����ܵ�������"pipe.cpp"
  // UCCI����������Ϣ
  char szIniFile[MAX_CHAR];                            // �����������ļ�"UCCI2QH.INI"��ȫ·��
  int nInfoNum, nOptionNum, nLevelNum;                 // �汾��Ϣ������ѡ�������������Ѷȼ�����
  char szEngineName[MAX_CHAR], szEngineFile[MAX_CHAR]; // UCCI�������ƺ�UCCI��������ļ���ȫ·��
  char szInfoStrings[MAX_INFO][MAX_CHAR], szOptionStrings[MAX_OPTION][MAX_CHAR]; // �汾��Ϣ��ѡ������
  char szLevelStrings[MAX_LEVEL][MAX_CHAR], szThinkModes[MAX_LEVEL][MAX_CHAR];   // �Ѷȼ���͸����Ѷȼ����µ�˼��ģʽ
} Ucci2QH;

// ICCS��ʽת��Ϊ�ŷ��ṹ
inline int ICCS_MOVE(const char *szIccs) {
  int sqSrc, sqDst;
  sqSrc = COORD_XY(szIccs[0] - 'A' + FILE_LEFT, '9' + RANK_TOP - szIccs[1]);
  sqDst = COORD_XY(szIccs[3] - 'A' + FILE_LEFT, '9' + RANK_TOP - szIccs[4]);
  return MOVE(sqSrc, sqDst);
}

// �ŷ��ṹת��ΪICCS��ʽ
inline void MOVE_ICCS(char *szIccs, int mv) {
  szIccs[0] = (FILE_X(SRC(mv))) + 'A' - FILE_LEFT;
  szIccs[1] = '9' + RANK_TOP - (RANK_Y(SRC(mv)));
  szIccs[2] = '-';
  szIccs[3] = (FILE_X(DST(mv))) + 'A' - FILE_LEFT;
  szIccs[4] = '9' + RANK_TOP - (RANK_Y(DST(mv)));
  szIccs[5] = '\0';
}

// ����������״̬(�ڵ���ģʽ�£���ʾ��״̬)
static void SetStatus(int nArg) {
  Ucci2QH.nStatus = nArg;
  if (Ucci2QH.bDebug) {
    fprintf(stderr, "Adapter Info: Status = ");
    switch (nArg) {
    case IDLE_NONE:
      fprintf(stderr, "IDLE_NONE");
      break;
    case IDLE_PONDER_FINISHED:
      fprintf(stderr, "IDLE_PONDER_FINISHED");
      break;
    case IDLE_PONDERHIT_FINISHED:
      fprintf(stderr, "IDLE_PONDERHIT_FINISHED");
      break;
    case BUSY_WAIT:
      fprintf(stderr, "BUSY_WAIT");
      break;
    case BUSY_THINK:
      fprintf(stderr, "BUSY_THINK");
      break;
    case BUSY_HINTS:
      fprintf(stderr, "BUSY_HINTS");
      break;
    case BUSY_PONDER:
      fprintf(stderr, "BUSY_PONDER");
      break;    
    case BUSY_PONDERHIT:
      fprintf(stderr, "BUSY_PONDERHIT");
      break;    
    }
    fprintf(stderr, "\n");
    fflush(stderr);
  }
}

// ��ǳ�����塱���ͷ�����Ϣ(�ڵ���ģʽ����ʾ����Ϣ)
inline void Adapter2QH(const char *szLineStr) {
  printf("%s\n", szLineStr);
  fflush(stdout);
  if (Ucci2QH.bDebug) {
    fprintf(stderr, "Adapter->Qianhong: %s\n", szLineStr);
    fflush(stderr);
  }
}

// ��UCCI���淢��ָ��(�ڵ���ģʽ����ʾ����Ϣ)
inline void Adapter2UCCI(const char *szLineStr) {
  Ucci2QH.pipeEngine.LineOutput(szLineStr);
  if (Ucci2QH.bDebug) {
    fprintf(stderr, "Adapter->UCCI-Engine: %s\n", szLineStr);
    fflush(stderr);
  }
}

// ���ա�ǳ�����塱��ָ��
inline bool QH2Adapter(char *szLineStr) {
  if (Ucci2QH.pipeStdin.LineInput(szLineStr)) {
    if (Ucci2QH.bDebug) {
      fprintf(stderr, "Qianhong->Adapter: %s\n", szLineStr);
      fflush(stderr);
    }
    return true;
  } else {
    return false;
  }
}

// ����UCCI����ķ�����Ϣ
inline bool UCCI2Adapter(char *szLineStr) {
  if (Ucci2QH.pipeEngine.LineInput(szLineStr)) {
    if (Ucci2QH.bDebug) {
      fprintf(stderr, "UCCI-Engine->Adapter: %s\n", szLineStr);
      fflush(stderr);
    }
    return true;
  } else {
    return false;
  }
}

// ǳ��ģʽ�¸����ڲ�����Ĺ���
static bool MakeMove(int mv) {
  if (mv == 0 || Ucci2QH.posIrrev[Ucci2QH.nIrrevPosNum].ucpcSquares[SRC(mv)] == 0) {
    return false;
  }
  if (Ucci2QH.posIrrev[Ucci2QH.nIrrevPosNum].ucpcSquares[DST(mv)] == 0) {
    // ������ǳ����ŷ�����ô����һ�����������ִ���ŷ�
    if (Ucci2QH.posIrrev[Ucci2QH.nIrrevPosNum].nMoveNum < MAX_IRREV_MOVE) {
      Ucci2QH.posIrrev[Ucci2QH.nIrrevPosNum].MakeMove(mv);
      Ucci2QH.nBanMoveNum = 0;
      return true;
    } else {
      return false;
    }
  } else {
    // ����ǳ����ŷ�����ô��������һ�����������
    if (Ucci2QH.nIrrevPosNum < MAX_IRREV_POS - 1) {
      Ucci2QH.nIrrevPosNum ++;
      Ucci2QH.posIrrev[Ucci2QH.nIrrevPosNum] = Ucci2QH.posIrrev[Ucci2QH.nIrrevPosNum - 1];
      Ucci2QH.posIrrev[Ucci2QH.nIrrevPosNum].MakeMove(mv);
      Ucci2QH.posIrrev[Ucci2QH.nIrrevPosNum].SetIrrev();
      Ucci2QH.posIrrev[Ucci2QH.nIrrevPosNum].ToFen(Ucci2QH.szIrrevFen[Ucci2QH.nIrrevPosNum]);
      Ucci2QH.nBanMoveNum = 0;
      return true;
    } else {
      return false;
    }
  }
}

inline int PieceChar(int pc) {
  if (pc < 16) {
    return '.';
  } else if (pc < 32) {
    return PIECE_BYTE(PIECE_TYPE(pc));
  } else {
    return PIECE_BYTE(PIECE_TYPE(pc)) - 'A' + 'a';
  }
}

// �Ѿ����ӡ����Ļ��
static void PrintPosition(const PositionStruct &pos) {
  int i, j;
  for (i = 3; i <= 12; i ++) {
    for (j = 3; j <= 11; j ++) {
      printf("%c", PieceChar(pos.ucpcSquares[i * 16 + j]));
    }
    printf("\n");
    fflush(stdout);
  }
  if (Ucci2QH.bDebug) {
    for (i = 3; i <= 12; i ++) {
      fprintf(stderr, "Adapter->Qianhong: ");
      for (j = 3; j <= 11; j ++) {
        fprintf(stderr, "%c", PieceChar(pos.ucpcSquares[i * 16 + j]));
      }
      fprintf(stderr, "\n");
      fflush(stderr);
    }
  }
}

// ��UCCI���淢��˼��ָ��
static void RunEngine(void) {
  int i;
  uint32_t dwMoveStr;
  char *lp;
  char szLineStr[LINE_INPUT_MAX_CHAR];
  // ����˼��ָ��Ҫ���������裺

  // 1. ���;�����Ϣ��������ʼ�Ĳ�����FEN����һϵ�к����ŷ�(��ͬ��̨˼���Ĳ²��ŷ�)��
  lp = szLineStr;
  lp += sprintf(lp, "position fen %s - - 0 1", Ucci2QH.szIrrevFen[Ucci2QH.nIrrevPosNum]);
  if (Ucci2QH.posIrrev[Ucci2QH.nIrrevPosNum].nMoveNum > 1) {
    lp += sprintf(lp, " moves");
    for (i = 1; i < Ucci2QH.posIrrev[Ucci2QH.nIrrevPosNum].nMoveNum; i ++) {
      dwMoveStr = MOVE_COORD(Ucci2QH.posIrrev[Ucci2QH.nIrrevPosNum].rbsList[i].mvs.wmv);
      lp += sprintf(lp, " %.4s", (const char *) &dwMoveStr);
    }
  }
  if (Ucci2QH.nStatus == BUSY_PONDER) {
    if (Ucci2QH.posIrrev[Ucci2QH.nIrrevPosNum].nMoveNum == 1) {
      lp += sprintf(lp, " moves");
    }
    dwMoveStr = MOVE_COORD(Ucci2QH.mvPonder);
    lp += sprintf(lp, " %.4s", (const char *) &dwMoveStr);
  }
  Adapter2UCCI(szLineStr);

  // 2. ���ͽ�ֹ�ŷ���Ϣ��
  if (Ucci2QH.nBanMoveNum > 0) {
    lp = szLineStr;
    lp += sprintf(lp, "banmoves");
    for (i = 0; i < Ucci2QH.nBanMoveNum; i ++) {
      dwMoveStr = MOVE_COORD(Ucci2QH.wmvBanList[i]);
      lp += sprintf(lp, " %.4s", (const char *) &dwMoveStr);
    }
    Adapter2UCCI(szLineStr);
  }

  // 3. ����˼��ָ�
  sprintf(szLineStr, Ucci2QH.nStatus == BUSY_PONDER ? "go ponder %s" : "go %s", Ucci2QH.szThinkModes[Ucci2QH.nLevel]);
  Adapter2UCCI(szLineStr);
}

// UCCI������Ϣ�Ľ��չ���
static bool ReceiveUCCI(void) {
  int mv;
  char *lp;
  char szIccs[8];
  char szLineStr[LINE_INPUT_MAX_CHAR];
  if (!UCCI2Adapter(szLineStr)) {
    return false;
  }
  lp = szLineStr;
  if (Ucci2QH.bUcciOkay) {
    if (StrEqvSkip(lp, "bestmove ")) {
      mv = COORD_MOVE(*(uint32_t *) lp);
      lp += sizeof(uint32_t);
      switch (Ucci2QH.nStatus) {
      // һ���յ������ŷ����͸���"bStatus"������Ӧ�Ĵ�����̣���ת�����״̬��

      // 1. "BUSY_WAIT"״̬��˵������"StopEngine()"�жϵģ������κδ���
      case BUSY_WAIT:
        SetStatus(IDLE_NONE);
        break;

      // 2. "BUSY_THINK"״̬�������ִ������ŷ����������޺�̨˼���²��ŷ�������Ӧ����
      case BUSY_THINK:
        MOVE_ICCS(szIccs, mv);
        Adapter2QH(szIccs);
        MakeMove(mv);
        if (Ucci2QH.bBgThink && StrEqvSkip(lp, " ponder ")) {
          Ucci2QH.mvPonder = COORD_MOVE(*(uint32_t *) lp);
          SetStatus(BUSY_PONDER);
          RunEngine();
        } else {
          SetStatus(IDLE_NONE);
        }
        break;

      // 3. "BUSY_HINTS"״̬��ֻҪ�������ŷ����ɣ�
      case BUSY_HINTS:
        MOVE_ICCS(szIccs, mv);
        Adapter2QH(szIccs);
        Adapter2QH("ENDHINTS");
        SetStatus(IDLE_NONE);
        break;

      // 4. "BUSY_PONDER"��"BUSY_PONDERHIT"״̬��ֻҪ������ŷ���¼Ϊ��̨˼��������ɣ�
      case BUSY_PONDER:
      case BUSY_PONDERHIT:
        Ucci2QH.mvPonderFinished = mv;
        SetStatus(Ucci2QH.nStatus == BUSY_PONDER ? IDLE_PONDER_FINISHED : IDLE_PONDERHIT_FINISHED);
        if (Ucci2QH.bBgThink && StrEqvSkip(lp, " ponder ")) {
          Ucci2QH.mvPonderFinishedPonder = COORD_MOVE(*(uint32_t *) lp);
        } else {
          Ucci2QH.mvPonderFinishedPonder = 0;
        }
        break;
      default:
        break;
      };
    } else if (StrEqv(lp, "nobestmove")) {

      // 5. �����û������ŷ��������
      switch (Ucci2QH.nStatus) {
      case BUSY_WAIT:
        break;
      case BUSY_HINTS:
      case BUSY_THINK:
        Adapter2QH("ERROR");
        break;
      case BUSY_PONDER:
      case BUSY_PONDERHIT:
        break;
      default:
        break;
      }
      SetStatus(IDLE_NONE);

    } else if (StrEqv(lp, "bye")) {
      Ucci2QH.bUcciOkay = false;
    }
  } else {
    if (StrEqv(lp, "ucciok")) {
      Ucci2QH.bUcciOkay = true;
    }
  }
  return true;
}

// ��ֹUCCI�����˼��
static void StopEngine(void) {
  int64_t llTime;
  SetStatus(BUSY_WAIT);
  Adapter2UCCI("stop");
  llTime = GetTime();
  while (Ucci2QH.nStatus != IDLE_NONE && (int) (GetTime() - llTime) < 1000) {
    if (!ReceiveUCCI()) {
      Idle();
    }
  }
  Ucci2QH.nStatus = IDLE_NONE;
}

// ǳ������ָ��Ľ��չ���
static bool ReceiveQH(void) {
  int i, j;
  int mv;
  int64_t llTime;
  char *lp;
  char szIccs[8];
  char szLineStr[LINE_INPUT_MAX_CHAR];

  if (!QH2Adapter(szLineStr)) {
    return false;
  }
  lp = szLineStr;
  if (false) {
  // ǳ������Э����յ���ָ����������¼��֣�

  // 1. "SCR"ָ��(��)��
  } else if (StrEqv(lp, "SCR")) {
    PrintPosition(Ucci2QH.posIrrev[Ucci2QH.nIrrevPosNum]);

  // 2. "LEVEL"ָ��(��)��
  } else if (StrEqvSkip(lp, "LEVEL ")) {
    Ucci2QH.nLevel = Str2Digit(lp, 0, Ucci2QH.nLevelNum - 1);
    Adapter2QH("OK");
  // ע�⣺���������ж�"LEVEL "�����ж�"LEVEL"
  } else if (StrEqv(lp, "LEVEL")) {
    sprintf(szLineStr, "%d", Ucci2QH.nLevelNum);
    Adapter2QH(szLineStr);

  // 3. "FEN"ָ������ڲ����棬���������̨˼��״̬��
  } else if (StrEqvSkip(lp, "FEN ")) {
    if (Ucci2QH.nStatus == BUSY_THINK || Ucci2QH.nStatus == BUSY_HINTS) {
      Adapter2QH("ERROR");
      return true;
    }
    Ucci2QH.nIrrevPosNum = 0;
    Ucci2QH.posIrrev[Ucci2QH.nIrrevPosNum].FromFen(lp);
    Ucci2QH.posIrrev[Ucci2QH.nIrrevPosNum].ToFen(Ucci2QH.szIrrevFen[Ucci2QH.nIrrevPosNum]);
    if (Ucci2QH.nStatus == BUSY_PONDER || Ucci2QH.nStatus == BUSY_PONDERHIT) {
      StopEngine();
    } else {
      SetStatus(IDLE_NONE);
    }
    Adapter2UCCI("setoption newgame");
    Adapter2QH("OK");

  // 4. "PLAY"ָ�
  } else if (StrEqvSkip(lp, "PLAY ")) {
    if (Ucci2QH.nStatus == BUSY_THINK || Ucci2QH.nStatus == BUSY_HINTS) {
      Adapter2QH("ERROR");
      return true;
    }
    mv = ICCS_MOVE(lp);
    if (!MakeMove(mv)) {
      Adapter2QH("ERROR");
      return true;
    }
    // �����ŷ�ִ����ϣ������Ǹ��ĺ�̨˼��״̬
    switch (Ucci2QH.nStatus) {
    case IDLE_PONDER_FINISHED:
      SetStatus(mv == Ucci2QH.mvPonder ? IDLE_PONDERHIT_FINISHED : IDLE_NONE);
      break;
    case IDLE_PONDERHIT_FINISHED:
      SetStatus(IDLE_NONE);
      break;
    case BUSY_PONDER:
      if (mv == Ucci2QH.mvPonder) {
        SetStatus(BUSY_PONDERHIT);
        Adapter2UCCI("ponderhit");
      } else {
        StopEngine();
      }
      break;
    case BUSY_PONDERHIT:
      StopEngine();
      break;
    default:
      break;
    }
    Adapter2QH("OK");

  // 5. "LOAD"ָ���һ�����ŷ������������̨˼��״̬��
  } else if (StrEqvSkip(lp, "LOAD ")) {
    i = Str2Digit(lp, 0, 1998); // һ���������999���غϣ���1998���ŷ�
    if (Ucci2QH.nStatus == BUSY_THINK || Ucci2QH.nStatus == BUSY_HINTS) {
      for (j = 0; j < i; j ++) {
        while (!QH2Adapter(szLineStr)) {
          Idle();
        }
      }
      Adapter2QH("ERROR");
      return true;
    }
    for (j = 0; j < i; j ++) {
      while (!QH2Adapter(szLineStr)) {
        Idle();
      }
      mv = ICCS_MOVE(szLineStr);
      MakeMove(mv);
    }
    if (Ucci2QH.nStatus == BUSY_PONDER || Ucci2QH.nStatus == BUSY_PONDERHIT) {
      StopEngine();
    } else {
      SetStatus(IDLE_NONE);
    }
    Adapter2QH("OK");

  // 6. "AI"ָ�����˼��״̬��
  } else if (StrEqv(lp, "AI")) {
    if (Ucci2QH.nStatus == BUSY_THINK || Ucci2QH.nStatus == BUSY_HINTS) {
      Adapter2QH("ERROR");
      return true;
    }
    switch (Ucci2QH.nStatus) {
    case IDLE_NONE:
    case IDLE_PONDER_FINISHED:
      SetStatus(BUSY_THINK);
      RunEngine();
      break;
    case IDLE_PONDERHIT_FINISHED:
      MakeMove(Ucci2QH.mvPonderFinished);
      MOVE_ICCS(szIccs, Ucci2QH.mvPonderFinished);
      Adapter2QH(szIccs);
      if (Ucci2QH.mvPonderFinishedPonder == 0) {
        SetStatus(IDLE_NONE);
      } else {
        Ucci2QH.mvPonder = Ucci2QH.mvPonderFinishedPonder;
        SetStatus(BUSY_PONDER);
        RunEngine();
      }
      break;
    case BUSY_PONDER:
      StopEngine();
      SetStatus(BUSY_THINK);
      RunEngine();
      break;
    case BUSY_PONDERHIT:
      SetStatus(BUSY_THINK);
      break;
    default:
      break;
    }

  // 7. "ABORT"ָ��(��)��
  } else if (StrEqv(lp, "ABORT")) {
    StopEngine();
    Adapter2QH("ABORTED");

  // 8. "QUIT"ָ��(��)��
  } else if (StrEqv(lp, "QUIT")) {
    if (Ucci2QH.nStatus > BUSY_WAIT) {
      StopEngine();
    }
    Adapter2UCCI("quit");
    llTime = GetTime();
    while (Ucci2QH.bUcciOkay && (int) (GetTime() - llTime) < 1000) {
      if (!ReceiveUCCI()) {
        Idle();
      }
    }
    Ucci2QH.bUcciOkay = false;

  // 9. "UNDO"ָ������ŷ������������̨˼��״̬��
  } else if (StrEqv(lp, "UNDO")) {
    if (Ucci2QH.nStatus == BUSY_THINK || Ucci2QH.nStatus == BUSY_HINTS) {
      Adapter2QH("ERROR");
      return true;
    }
    if (Ucci2QH.posIrrev[Ucci2QH.nIrrevPosNum].nMoveNum == 1) {
      if (Ucci2QH.nIrrevPosNum == 0) {
        Adapter2QH("ERROR");
        return true;
      }
      Ucci2QH.nIrrevPosNum --;
    } else {
      Ucci2QH.posIrrev[Ucci2QH.nIrrevPosNum].UndoMakeMove();
    }
    if (Ucci2QH.nStatus == BUSY_PONDER || Ucci2QH.nStatus == BUSY_PONDERHIT) {
      StopEngine();
    } else {
      SetStatus(IDLE_NONE);
    }
    Adapter2QH("OK");

  // 10. "HINTS"ָ�������ʾ��
  } else if (StrEqv(lp, "HINTS")) {
    if (Ucci2QH.nStatus == BUSY_THINK || Ucci2QH.nStatus == BUSY_HINTS) {
      Adapter2QH("ERROR");
      return true;
    }
    if (Ucci2QH.nStatus == BUSY_PONDER || Ucci2QH.nStatus == BUSY_PONDERHIT) {
      // ������ں�̨˼�����������̨˼���Ĳ²��ŷ�����Ϊ��ʾ�ŷ�
      MOVE_ICCS(szIccs, Ucci2QH.mvPonder);
      Adapter2QH(szIccs);
      Adapter2QH("ENDHINTS");
    } else {
      // ���������������˼��һ����ʾ�ŷ�
      SetStatus(BUSY_HINTS);
      RunEngine();
    }

  // 11. "BAN"ָ������ֹ�ŷ���"Ucci2QH.wmvBanList"�Ϳ����ˣ�
  } else if (StrEqvSkip(lp, "BAN ")) {
    Ucci2QH.nBanMoveNum = Str2Digit(lp, 0, MAX_BAN_MOVE);
    for (i = 0; i < Ucci2QH.nBanMoveNum; i ++) {
      while (!QH2Adapter(szLineStr)) {
        Idle();
      }
      Ucci2QH.wmvBanList[i] = ICCS_MOVE(szLineStr);
    }
    if (Ucci2QH.nStatus == BUSY_PONDER || Ucci2QH.nStatus == BUSY_PONDERHIT) {
      StopEngine();
    } else {
      SetStatus(IDLE_NONE);
    }
    Adapter2QH("OK");

  // 12. "BGTHINK"ָ��(��)��
  } else if (StrEqv(lp, "BGTHINK ON")) {
    Ucci2QH.bBgThink = true;
    Adapter2QH("OK");
  } else if (StrEqv(lp, "BGTHINK OFF")) {
    Ucci2QH.bBgThink = false;
    Adapter2QH("OK");

  // 13. "TIMEOUT"ָ��(��)��
  } else if (StrEqv(lp, "TIMEOUT")) {
    Adapter2UCCI("stop");
  }
  return true;
}

// ������
int main(int argc, char **argv) {
  int64_t llTime;
  char szLineStr[MAX_CHAR];
  char *lp;
  FILE *fpIniFile;
  int i, nCurrLevel;

  if (argc < 2) {
    return 0;
  }
  LocatePath(Ucci2QH.szIniFile, "UCCI2QH.INI");
  nCurrLevel = Ucci2QH.nLevelNum = Ucci2QH.nInfoNum = 0;
  Ucci2QH.szEngineName[0] = Ucci2QH.szEngineFile[0] = '\0';
  fpIniFile = fopen(Ucci2QH.szIniFile, "rt");
  if (fpIniFile == NULL) {
    return 0;
  }
  while (fgets(szLineStr, MAX_CHAR, fpIniFile) != NULL) {
    StrCutCrLf(szLineStr);
    lp = szLineStr;
    if (false) {
    } else if (StrEqvSkip(lp, "Name=")) {
      strcpy(Ucci2QH.szEngineName, lp);
    } else if (StrEqvSkip(lp, "File=")) {
      LocatePath(Ucci2QH.szEngineFile, lp);
    } else if (StrEqvSkip(lp, "Info=")) {
      if (Ucci2QH.nLevelNum < MAX_INFO) {
        strcpy(Ucci2QH.szInfoStrings[Ucci2QH.nInfoNum], lp);
        Ucci2QH.nInfoNum ++;
      }
    } else if (StrEqvSkip(lp, "Option=")) {
      if (Ucci2QH.nOptionNum < MAX_OPTION) {
        strcpy(Ucci2QH.szOptionStrings[Ucci2QH.nOptionNum], lp);
        Ucci2QH.nOptionNum ++;
      }
    } else if (StrEqvSkip(lp, "Level=")) {
      if (Ucci2QH.nLevelNum < MAX_LEVEL) {
        strcpy(Ucci2QH.szLevelStrings[Ucci2QH.nLevelNum], lp);
        Ucci2QH.nLevelNum ++;
      }
    } else if (StrEqvSkip(lp, "ThinkMode=")) {
      if (nCurrLevel < Ucci2QH.nLevelNum) {
        strcpy(Ucci2QH.szThinkModes[nCurrLevel], lp);
        nCurrLevel ++;
      }
    }
  }
  fclose(fpIniFile);
  for (; nCurrLevel < Ucci2QH.nLevelNum; nCurrLevel ++) {
    Ucci2QH.szThinkModes[nCurrLevel][0] = '\0';
  }

  if (false) {
  // ǳ���������������������ʽ��

  // 1. �������棺UCCI2QH -plugin [debug]
  } else if (StrEqv(argv[1], "-plugin")) {
    Ucci2QH.bDebug = Ucci2QH.bUcciOkay = Ucci2QH.bBgThink = false;
    Ucci2QH.nLevel = 0;
    SetStatus(IDLE_NONE);
    if (argc > 2) {
      if (StrEqv(argv[2], "debug")) {
        Ucci2QH.bDebug = true;
      }
    }
    Ucci2QH.pipeStdin.Open();
    Ucci2QH.pipeEngine.Open(Ucci2QH.szEngineFile);
    Adapter2UCCI("ucci");
    PreGenInit();
    Ucci2QH.nIrrevPosNum = 0;
    strcpy(Ucci2QH.szIrrevFen[0], cszStartFen);
    Ucci2QH.posIrrev[0].FromFen(Ucci2QH.szIrrevFen[0]);
    llTime = GetTime();
    // �ȴ�10���ӣ���������޷�������������ֱ���˳���
    while (!Ucci2QH.bUcciOkay && (int) (GetTime() - llTime) < 10000) {
      if (!ReceiveUCCI()) {
        Idle();
      }
    }
    if (Ucci2QH.bUcciOkay) {
      for (i = 0; i < Ucci2QH.nOptionNum; i ++) {
        Adapter2UCCI(Ucci2QH.szOptionStrings[i]);
      }
      Adapter2UCCI("setoption newgame");
    }
    while (Ucci2QH.bUcciOkay) {
      if (!(ReceiveUCCI() || ReceiveQH())) {
        Idle();
      }
    }
    Ucci2QH.pipeEngine.Close();
    Adapter2QH("BYE");

  // 2. ��ʾ������Ϣ��UCCI2QH -info
  } else if (StrEqv(argv[1], "-info")) {
    printf("QHPLUGIN V1.3\n");
    printf("%s\n", Ucci2QH.szEngineName);
    printf("LEVELS %d\n", Ucci2QH.nLevelNum);
    for (i = 0; i < Ucci2QH.nLevelNum; i ++) {
      printf("%d - %s\n", i, Ucci2QH.szLevelStrings[i]);
    }
    printf("UNDO 1\n");
    printf("HINTS 1\n");
    printf("RULES 1\n");
    printf("BGTHINK 1\n");
    printf("TIMEOUT 1\n");
    for (i = 0; i < Ucci2QH.nInfoNum; i ++) {
      printf("%s\n", Ucci2QH.szInfoStrings[i]);
    }
    printf("=== UCCI Engine Options ===\n");
    printf("Engine File: %s\n", Ucci2QH.szEngineFile);
    printf("Option List:\n");
    for (i = 0; i < Ucci2QH.nOptionNum; i ++) {
      printf("%s\n", Ucci2QH.szOptionStrings[i]);
    }
    printf("Level List:\n");
    for (i = 0; i < Ucci2QH.nLevelNum; i ++) {
      printf("%s=\"go [ponder] %s\"\n", Ucci2QH.szLevelStrings[i], Ucci2QH.szThinkModes[i]);
    }
    printf("ENDINFO\n");
    fflush(stdout);
  }
  return 0;
}
