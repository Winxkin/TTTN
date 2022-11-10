// ---------------------------------------------------------------------
// $Id: INTC2G_U2_AgentController.h,v 1.9 2018/12/18 09:55:21 huepham Exp $
//
// Copyright(c) 2018 Renesas Electronics Corporation
// Copyright(c) 2018 Renesas Design Vietnam Co., Ltd.
// RENESAS ELECTRONICS CONFIDENTIAL AND PROPRIETARY.
// This program must be used solely for the purpose for which
// it was furnished by Renesas Electronics Corporation. No part of this
// program may be reproduced or disclosed to others, in any
// form, without the prior written permission of Renesas Electronics
// Corporation.
// ---------------------------------------------------------------------

#ifndef __INTC2G_U2_AGENT_H__
#define __INTC2G_U2_AGENT_H__

#include <systemc.h>
#include <iostream>
#include <cstring>
#include <iterator>
#include <map>
#include <stdarg.h>

class INTC2G_U2;

class INTC2G_U2_AgentController {
#include "INTC2G_U2_cmdif.h"

public:
      static const int DEFAULT_LOCK = 0;
      static const int MIN_CH_NUM = 3;
      static const int MAX_CH_NUM = 2018;
public:
  // std::string mGroupId;
  unsigned int mChannelNum;
  INTC2G_U2_AgentController(void) {;}
  virtual ~INTC2G_U2_AgentController(void) {;}

  virtual bool CheckClockPeriod(const std::string clock_name) = 0;
  virtual void GetINTC2GPROT_n(unsigned int channel_id, bool& gen, bool& dbg, bool& um, bool& wg, bool& rg, unsigned int& mpid) = 0;
  virtual unsigned int GetLOCK_SUPPORT() = 0;
  virtual unsigned int GetMpidNumber() = 0;
  virtual unsigned int GetSPIDValue(unsigned int index) = 0;
  virtual void LOCK_SUPPORT(unsigned int value) = 0;
};


#endif//__INTC2G_U2_AGENT_H__
