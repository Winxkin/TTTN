// ---------------------------------------------------------------------
// $Id: INTC2G_U2_Func.h,v 1.9 2018/12/21 07:54:37 huepham Exp $
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
#ifndef __INTC2G_U2_FUNC_H__
#define __INTC2G_U2_FUNC_H__

#include "BusSlaveFuncIf.h"
#include "intc2g_u2_regif.h"

class INTC2G_U2_AgentController;
class INTC2G_U2_Func: public Cintc2g_u2_regif,
                public BusSlaveFuncIf
{
public:
  INTC2G_U2_Func (std::string name, INTC2G_U2_AgentController *INTC2G_U2_AgentControllerPtr, unsigned int ch_num);
  ~INTC2G_U2_Func (void);

  void Reset (const bool is_active);
  void RegisterHandler (const std::vector<std::string> &args);
  void SetVar(const std::string name, const unsigned int val);
  unsigned int Get_SPIDERR(void);

  /// Virtual functions of BusSlaveFuncIf class
  void read (unsigned int offsetAddress, TlmBasicPayload& trans, BusTime_t* t, bool debug);
  void write (unsigned int offsetAddress, TlmBasicPayload& trans, BusTime_t* t, bool debug);
  void P_RESET_VALUE0_m(unsigned int value, unsigned int index);
  void GetINTC2GPROT_n(unsigned int channel_id, bool& gen, bool& dbg, bool& um, bool& wg, bool& rg, unsigned int& mpid);
  unsigned int GetSPIDValue(unsigned int index);

private:
  INTC2G_U2_AgentController *mINTC2G_U2_AgentController;
  std::string mModuleName;        //Module name
  unsigned int mChNum;

/// Declare virtual functions of INTC2G_U2_regif class
    void cb_INTC2GOVFCLR_CLRO(RegCBstr str);
    void cb_INTC2GSPIDERRCLR_SPIDCLR(RegCBstr str);
    void cb_INTC2GKCPROT_KCPROT(RegCBstr str);
    void cb_INTC2GPROT_GR_MPID(RegCBstr str);
    void cb_INTC2GPROT_IMR_MPID(RegCBstr str);
    void cb_INTC2GPROT__MPID(RegCBstr str);
    void cb_INTC2GMPID_SPID(RegCBstr str);
};
#endif //__INTC2G_U2_FUNC_H__

