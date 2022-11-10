// ---------------------------------------------------------------------
// $Id: INTC2G_U2_Common_Guard.h,v 1.6 2018/12/21 08:25:15 huepham Exp $
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

#ifndef __INTC2G_U2_DATAHANDLER_H__
#define __INTC2G_U2_DATAHANDLER_H__
#include "BusBridgeCoreBase.h"
#include "INTC2G_U2_AgentController.h"

class INTC2G_U2_AgentController;
class INTC2G_U2_Common_Guard : //public sc_module,
                     public BusBridgeCoreBase<32,32>
{
public:
    INTC2G_U2_Common_Guard(std::string name,
                   INTC2G_U2_AgentController* INTC2G_U2_AgentControllerPtr,
                   BusBridgeModuleBase<32,32> *Parent,
                   BusBridgeSlaveIf<32> *SlaveIf,
                   BusMasterIf<32> *MasterIf,
                   SIM_MODE_T simmode,
                   unsigned int ch_num);
    virtual ~INTC2G_U2_Common_Guard(){;}


    bool SetAreaAddr(const unsigned int ch_id, const unsigned int start_addr1, const unsigned int size1, const unsigned int start_addr2, const unsigned int size2);

    void EnableReset (const bool is_active);                                            //Enable/Disable reset state
    void SetCLKPeriod (const sc_dt::uint64 clk_period);

    unsigned int GetVAR(const std::string name);
    sc_event                        mTransErrorEvent;

private:

    INTC2G_U2_AgentController* mINTC2G_U2_AgentController;
    BusMasterIf<32> *mMasterIf;
    BusBridgeSlaveIf<32> *mSlaveIf;
    std::map<TlmBasicPayload*,TlmBasicPayload*> mRequestMap; //Bus request map for AT mode
    std::queue<TlmBasicPayload *>  mClearTransFifo; //Clear transaction list for AT mode
    TlmBasicPayload* mTrans;//Transaction info
    BusTime_t mTransTime;//Internal time
    bool mIsDebug;//Is debug transaction
    unsigned int mChannelNum;

    void TransferComplete(void);
    void UpdateTransBasic(TlmBasicPayload &inTrans, TlmBasicPayload &outTrans);
    void SetTransBasic(unsigned int access_address,unsigned int data_length , unsigned int data, TlmBasicPayload &outTrans);
    void SetTrans(TlmBasicPayload &inTrans, TlmBasicPayload &outTrans);

    struct AreaInformation {
        unsigned int ch_id;
        unsigned int start_addr;
        unsigned int end_addr;
        AreaInformation(unsigned int _ch_id = 0,
                        unsigned int _start_addr = 0,
                        unsigned int _end_addr = 0)
        {
            this->ch_id = _ch_id;
            this->start_addr = _start_addr;
            this->end_addr = _end_addr;
        }
    };
    std::vector<AreaInformation> mAreaInfo;



    //Overwrite virtual functions of BusBridgeCoreBase class
    void read(unsigned int offsetAddress,TlmBasicPayload& trans,BusTime_t* t,bool debug);
    void write(unsigned int offsetAddress,TlmBasicPayload& trans,BusTime_t* t,bool debug);
    void deleteTrans(void);
    void notifyTransComp(TlmBasicPayload &trans, BusTime_t &t);

    //private function and variables
    bool CheckTransAddress(const unsigned int trans_addr, unsigned int &ch_id);
    bool CheckAccessibility(unsigned int access_addr, TlmBasicPayload &trans, bool debug);
    void CancelEvents (void);
    void Initialize (void);
    bool mRstState;
    sc_dt::uint64                   mCLKPeriod;


    unsigned int                    mTypeADDR;
    bool                            mTypeDBG;
    bool                            mTypeUM;
    bool                            mTypeSEC;
    unsigned int                    mTypeSPID;
    bool                            mTypeWRITE;
    bool                            mSECPara;


};
#endif
//
