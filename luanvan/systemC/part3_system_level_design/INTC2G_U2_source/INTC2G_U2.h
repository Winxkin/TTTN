// ---------------------------------------------------------------------
// $Id: INTC2G_U2.h,v 1.11 2018/12/18 09:55:21 huepham Exp $
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

#ifndef __INTC2G_U2_H__
#define __INTC2G_U2_H__

#include "BusBridgeModuleBase.h"
#include "BusBridgeSlaveBase.h"
#include "BusMasterBase.h"
#include "BusSlaveBase.h"
#include "INTC2G_U2_AgentController.h"


template<unsigned int BUSWIDTH, unsigned int S_NUM> class BusBridgeSlaveBase;
template<unsigned int BUSWIDTH, unsigned int S_NUM> class BusMasterBase;
template<unsigned int BUSWIDTH> class TlmTargetSocket;
template<unsigned int BUSWIDTH> class TlmInitiatorSocket;
class INTC2G_U2_Func;
class INTC2G_U2_Common_Guard;
class INTC2G_U2:  public INTC2G_U2_AgentController,
public BusBridgeModuleBase<32,32>,
public BusBridgeSlaveBase<32,1>,
public BusMasterBase<32,1>,
public BusSlaveBase<32,1>
{
private:
    enum eResetActiveLevel {    // Define active level of reset signal
        emResetActive = 0
    };


    INTC2G_U2_Func        *mINTC2G_U2_Func;                 //INTC2G_U2_Func class instance
    INTC2G_U2_Common_Guard *mINTC2G_U2_DataHandler;          //INTC2G_U2_Common_Guard class instance

    sc_dt::uint64   mPCLKPeriod;                //Period value of clock
    sc_dt::uint64   mPCLKFreq;                  //Frequency value of clock
    sc_dt::uint64   mPCLKOrgFreq;               //Original frequency of clock
    std::string     mPCLKFreqUnit;              //Frequency unit of clock

    sc_dt::uint64   mTimeResolutionValue;       //Time resolution value of simulation
    sc_time_unit    mTimeResolutionUnit;        //Time resolution unit of simulation
    bool            mEnableRegisterMessage;     // Enable/Disable info/warning/error message of register IF

    bool            mResetCurVal;               //Store current value of reset signals
    bool            mResetPreVal;               //Store previous value of reset signals
    bool            mIsResetHardActive;         //Reset operation status of reset signals
    bool            mIsResetCmdReceive;         //Receive status of AssertReset command of reset signals
    bool            mIsResetCmdActive;          //Active status of AssertReset command of reset signals
    double          mResetCmdPeriod;            //Period of AssertReset command of reset signals
    double          mTimeCLKChangePoint;        //The time clock is changed its value
    bool            mDumpInterrupt;             // Enable/Disable dumping the interrupt info
    
    bool            merr_irq_value;             //Value of err_irq output
    unsigned int    mLock;                      //Store the value which is set from Python
    unsigned int    mmpid_num;

    /// Declare events
    sc_event mResetHardEvent;                   //Call HandleResetHardMethod when reset is active
    sc_event mResetCmdEvent;                    //Call HandleResetCmdMethod when AssertReset is called
    sc_event mResetCmdCancelEvent;              //Call CancelResetCmdMethod when AssertReset is over
    sc_event mClkZeroEvent;                     //Notify clock is zero

    sc_event mAsserterr_irqEvent;               //Assert err_irq output
    sc_event mDeasserterr_irqEvent;             //Deassert err_irq output

    sc_event mWriteerr_irqEvent;                //Write to err_irq output


public:
    SC_HAS_PROCESS(INTC2G_U2);
    INTC2G_U2(sc_module_name name,
            unsigned int rLatency,
            unsigned int wLatency,
            unsigned int iCHAN_NUM,
            SIM_MODE_T simmode);
    ~INTC2G_U2(void);

    sc_in<sc_dt::uint64> PCLK;
    sc_out<bool> err_irq;

    //Socket declaration
    TlmTargetSocket<32> *tsp;
    TlmTargetSocket<32> *tspa;
    TlmInitiatorSocket<32> *a_is;


    void AssertReset (const std::string reset_name, const double start_time, const double period);
    void SetCLKFreq (const std::string clock_name, const sc_dt::uint64 freq, const std::string unit);
    void GetCLKFreq (const std::string clock_name);
    void ForceRegister (const std::string reg_name, const unsigned int reg_value);
    void ReleaseRegister (const std::string reg_name);
    void WriteRegister (const std::string reg_name, const unsigned int reg_value);
    void ReadRegister (const std::string reg_name);
    void ListRegister (void);

    void Help (const std::string type);
    void SetMessageLevel (const std::string msg_lv);
    void DumpRegisterRW (const std::string is_enable);
    void EnableRegisterMessage(const std::string is_enable);
    void DumpInterrupt (const std::string is_enable);

    void SetAreaAddress (const unsigned int ch_id, const unsigned int start_addr1, const unsigned int size1,const unsigned int start_addr2, const unsigned int size2);
    void LOCK_SUPPORT(unsigned int value);      //Set mLock
    unsigned int GetLOCK_SUPPORT();            //Get mLock
    unsigned int GetMpidNumber();            //Get mpid
    void SetMpidNumber(const unsigned int mpid_num);//set mpid
    void SetResetValues0(const unsigned int mpid_no,const unsigned int value);
    void GetINTC2GPROT_n(unsigned int ch_id, bool& gen, bool& dbg, bool& um, bool& wg, bool& rg, unsigned int& mpid);
    unsigned int GetSPIDValue(unsigned int index);
        
private:
    /// Declare methods
    void HandleErrorMethod (void);
    void HandlePCLKSignalMethod (void);

    void HandlePresetnSignalMethod (void);
    void HandleResetHardMethod (void);
    void HandleResetCmdMethod (void);
    void CancelResetCmdMethod (void);

    void Asserterr_irqMethod (void);
    void Deasserterr_irqMethod (void);
    void Writeerr_irqMethod (void);

    /// Internal functions
    void DumpInterruptMsg (const std::string intr_name, const bool value);              //Dump interrupt message
    void EnableReset (const std::string reset_name, const bool is_active);
    void ConvertClockFreq (sc_dt::uint64 &freq_out, std::string &unit_out,
            sc_dt::uint64 freq_in, std::string unit_in);
    void SeparateString (std::vector<std::string> &vtr, const std::string msg);
    void GetTimeResolution (sc_dt::uint64 &resolution_value, sc_time_unit &resolution_unit);
    double CalculateClockEdge (const std::string clock_name, const double time_point,const double time_change_point);

    void CancelEvents (void);
    void RegisterHandler (const std::vector<std::string> cmd);
    void PCLKUpdate (void);

    bool CheckClockPeriod (const std::string clock_name);

};
#endif //__INTC2G_U2_H__
//
