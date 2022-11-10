// ---------------------------------------------------------------------
// $Id: INTC2G_U2.cpp,v 1.12 2018/12/18 09:55:21 huepham Exp $
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


#include "INTC2G_U2.h"
#include "INTC2G_U2_Common_Guard.h"
#include "INTC2G_U2_Func.h"

///Constructor of INTC2G_U2 class
INTC2G_U2::INTC2G_U2 (sc_module_name name,
        unsigned int rLatency,
        unsigned int wLatency,
        unsigned int iCHAN_NUM,
        SIM_MODE_T simmode):
    INTC2G_U2_AgentController(),
    BusBridgeModuleBase<32,32>(name),
    BusBridgeSlaveBase<32,1>(),
    BusMasterBase<32,1>(),
    BusSlaveBase<32,1>(),
    PCLK("PCLK"),
    err_irq("err_irq"),
    mLock(INTC2G_U2_AgentController::DEFAULT_LOCK)
{//{{{
    mChannelNum = iCHAN_NUM;

    if(mChannelNum < INTC2G_U2_AgentController::MIN_CH_NUM) {
        re_printf("warning","INTC2G_U2's id is wrong, it is set to the default min value 3 automatically");
        mChannelNum = 3;
    }else if(mChannelNum > INTC2G_U2_AgentController::MAX_CH_NUM) {
        re_printf("warning","INTC2G_U2's id is wrong, it is set to the default max value 2018 automatically");
        mChannelNum = 2018;
    }
    re_printf("info", "Number of channel is: %d .\n", mChannelNum);
    mPCLKPeriod   = 0;
    mPCLKFreq     = 0;
    mPCLKOrgFreq  = 0;
    mPCLKFreqUnit = "Hz";

    mTimeResolutionValue = 1;
    mTimeResolutionUnit = SC_NS;
    GetTimeResolution(mTimeResolutionValue, mTimeResolutionUnit);

    mResetCurVal        = true;
    mResetPreVal        = true;
    mIsResetHardActive  = false;
    mIsResetCmdReceive  = false;
    mIsResetCmdActive   = false;
    mResetCmdPeriod     = 0;
    mTimeCLKChangePoint = 0;
    mmpid_num           = 8;
    merr_irq_value      = false;

    mDumpInterrupt      = false;
    mEnableRegisterMessage = true;
    // Initialize internal variables
    err_irq.initialize(false);

    //Initialize Bus
    setMasterResetPort32(&resetPort);
    setMasterFreqPort32(&PCLK);
    setInitiatorSocket32("a_is");

    setSlaveFreqPort32(&PCLK);
    setSlaveResetPort32(&resetPort);
    setTargetSocket32("tsp");

    setSlaveResetPort(&resetPort);
    setSlaveFreqPort(&PCLK);
    setTargetSocket("tspa");

    tsp = BusSlaveBase<32,1>::tSocket32[0]; //Alias for binding
    tspa = BusBridgeSlaveBase<32,1>::tSocket[0]; //Alias for binding
    a_is  = iSocket32[0]; //Alias for binding

    //Instance INTC2G_U2_Func
    mINTC2G_U2_Func = new INTC2G_U2_Func((std::string)name, this, mChannelNum);
    sc_assert(mINTC2G_U2_Func != NULL);
    mBusSlaveIf32[0]->setFuncModulePtr(mINTC2G_U2_Func);
    mBusSlaveIf32[0]->setBusProtocol(BUS_APB);
    mBusSlaveIf32[0]->setReadInitLatency(rLatency);
    mBusSlaveIf32[0]->setWriteInitLatency(wLatency);
    mBusSlaveIf32[0]->setTransNmbLmt(0x1);

    //Instance Common_Guard
    mINTC2G_U2_DataHandler = new INTC2G_U2_Common_Guard((std::string)name, this, this, mBusSlaveIf[0], mBusMasterIf32[0], simmode, mChannelNum);
    BusBridgeModuleBase<32,32>::mpCore = mINTC2G_U2_DataHandler; 
    mBusMasterIf32[0]->setFuncModulePtr((BusMasterFuncIf *)mINTC2G_U2_DataHandler);
    mBusMasterIf32[0]->setBusProtocol(BUS_APB);
    mBusMasterIf32[0]->setTransNmbLmt(0x1);
    mBusSlaveIf[0]->setFuncModulePtr( (BusSlaveFuncIf *)mINTC2G_U2_DataHandler);
    mBusSlaveIf[0]->setBusProtocol(BUS_APB);
    mBusSlaveIf[0]->setTransNmbLmt(0x1);

    SC_METHOD(HandlePCLKSignalMethod);
    dont_initialize();
    sensitive << PCLK;

    SC_METHOD(HandlePresetnSignalMethod);
    sensitive << resetPort;

    SC_METHOD(HandleResetHardMethod);
    dont_initialize();
    sensitive << mResetHardEvent;

    SC_METHOD(HandleResetCmdMethod);
    dont_initialize();
    sensitive << mResetCmdEvent;

    SC_METHOD(CancelResetCmdMethod);
    dont_initialize();
    sensitive << mResetCmdCancelEvent;

    SC_METHOD(Asserterr_irqMethod);
    dont_initialize();
    sensitive<<mAsserterr_irqEvent;

    SC_METHOD(Deasserterr_irqMethod);
    dont_initialize();
    sensitive<<mDeasserterr_irqEvent;

    SC_METHOD(HandleErrorMethod);
    dont_initialize();
    sensitive<<mINTC2G_U2_DataHandler->mTransErrorEvent;

    SC_METHOD(Writeerr_irqMethod);
    dont_initialize();
    sensitive<<mWriteerr_irqEvent;

}//}}}

///Destructor of INTC2G_U2 class
INTC2G_U2::~INTC2G_U2 (void)
{//{{{
    delete mINTC2G_U2_Func;
    delete mINTC2G_U2_DataHandler;
}//}}}

///assert err_irq signal method.
void INTC2G_U2::Asserterr_irqMethod(void)
{//{{{
    bool pre_value = err_irq.read();

    merr_irq_value = true;
    mWriteerr_irqEvent.notify(SC_ZERO_TIME);

    if(!pre_value) {
        DumpInterruptMsg("err_irq",true);
    }
    double current_time = sc_time_stamp().to_double();
    mDeasserterr_irqEvent.notify(CalculateClockEdge("PCLK", current_time,mTimeCLKChangePoint)+mPCLKPeriod,mTimeResolutionUnit);
}//}}}

///Deassert err_irq signal method.
void INTC2G_U2::Deasserterr_irqMethod(void)
{//{{{
    bool pre_value = err_irq.read();
    merr_irq_value = false;
    mWriteerr_irqEvent.notify(SC_ZERO_TIME);
    if(pre_value) {
        DumpInterruptMsg("err_irq",false);
    }
}//}}}

///Write err_irq signal method.
void INTC2G_U2::Writeerr_irqMethod(void)
{//{{{
    err_irq.write(merr_irq_value);
}//}}}

///Set address of guard areas
void INTC2G_U2::SetAreaAddress(const unsigned int ch_id, const unsigned int start_addr1, const unsigned int size1, const unsigned int start_addr2, const unsigned int size2)
{//{{{
    re_printf("info","INTC2G_U2 ch_id: %d, start_addr1: %x, size1: %d, start_addr2: %x, size2: %d\n", ch_id, start_addr1, size1, start_addr2, size2);
    if (ch_id >= mChannelNum ) {
        re_printf("error","channel ID %d is invalid with defined channel number(%d) in contructor\n", ch_id, mChannelNum);
    }else{
        if ((ch_id == 0 && size1 == 0) || (ch_id == 1 && size1 == 0)){
            re_printf("error","Invalid setting size1 = 0x0 of channel id %d. This setting is ignore.\n", ch_id);
            return;
        }else if (( 1< ch_id <mChannelNum) && (size1 ==0 || size2 == 0)){
            re_printf("error","Invalid setting size1 = 0x0 or size2 = 0x0 of channel id %d. This setting is ignore.\n", ch_id);
            return;
        }    
        mINTC2G_U2_DataHandler->SetAreaAddr(ch_id, start_addr1, size1, start_addr2, size2);
    }
}//}}}

///Check access permission to peripherals
void INTC2G_U2::HandleErrorMethod(void)
{//{{{
    double current_time = sc_time_stamp().to_double();
    if(mINTC2G_U2_Func->Get_SPIDERR() == 0){
        mINTC2G_U2_Func->SetVar("ADDR", mINTC2G_U2_DataHandler->GetVAR("ADDR"));
        mINTC2G_U2_Func->SetVar("DBG", mINTC2G_U2_DataHandler->GetVAR("DBG"));
        mINTC2G_U2_Func->SetVar("UM", mINTC2G_U2_DataHandler->GetVAR("UM"));
        mINTC2G_U2_Func->SetVar("SEC", mINTC2G_U2_DataHandler->GetVAR("SEC"));
        mINTC2G_U2_Func->SetVar("SPID", mINTC2G_U2_DataHandler->GetVAR("SPID"));
        mINTC2G_U2_Func->SetVar("WRITE", mINTC2G_U2_DataHandler->GetVAR("WRITE"));
    }
    if(((mINTC2G_U2_Func->Get_SPIDERR() >> (mINTC2G_U2_DataHandler->GetVAR("SPID"))) & 0x1) == 0){
        mAsserterr_irqEvent.notify(CalculateClockEdge("PCLK", current_time,mTimeCLKChangePoint),mTimeResolutionUnit);
    }
    mINTC2G_U2_Func->SetVar("SPIDERR",1 << (mINTC2G_U2_DataHandler->GetVAR("SPID")));
}//}}}

///Check clock period value
bool INTC2G_U2::CheckClockPeriod (const std::string clock_name)
{//{{{
    bool clock_status = false;
    sc_dt::uint64 clock_period = 0;
    clock_period = (clock_name == "PCLK") ? mPCLKPeriod : 0;
    if (clock_period > 0) {
        clock_status = true;
    } else {
        clock_status = false;
        re_printf("info","%s frequency is zero\n", clock_name.c_str());
    }
    return clock_status;
}//}}}

///Update PCLK clock
void INTC2G_U2::PCLKUpdate (void)
{//{{{
    mINTC2G_U2_DataHandler->SetCLKPeriod(mPCLKPeriod);
    double current_time = sc_time_stamp().to_double();
    mTimeCLKChangePoint = current_time;
}//}}}

///Check frequency value and frequency unit
void INTC2G_U2::ConvertClockFreq (sc_dt::uint64 &freq_out, std::string &unit_out,
                             sc_dt::uint64 freq_in, std::string unit_in)
{//{{{
    if(unit_in == "GHz"){
        freq_out = freq_in * (sc_dt::uint64)(1.0e+9);
        unit_out = "GHz";
    } else if (unit_in == "MHz") {
        freq_out = freq_in * (sc_dt::uint64)(1.0e+6);
        unit_out = "MHz";
    } else if (unit_in == "KHz") {
        freq_out = freq_in * (sc_dt::uint64)(1.0e+3);
        unit_out = "KHz";
    } else if (unit_in == "Hz") {
        freq_out = freq_in;
        unit_out = "Hz";
    } else {// Frequency unit is wrong, frequency unit is set as Hz default.
        freq_out = freq_in;
        unit_out = "Hz";
        re_printf("warning","Frequency unit (%s) is wrong, frequency unit is set as unit Hz default.\n", unit_in.c_str());
    }
}//}}}

///Get time resolution
void INTC2G_U2::GetTimeResolution (sc_dt::uint64 &resolution_value, sc_time_unit &resolution_unit)
{//{{{
    if (sc_get_time_resolution() < sc_time(1, SC_PS)) {
        resolution_value = 1000000000000000LL;
        resolution_unit = SC_FS;
    } else if (sc_get_time_resolution() < sc_time(1, SC_NS)) {
        resolution_value = 1000000000000LL;
        resolution_unit = SC_PS;
    } else if (sc_get_time_resolution() < sc_time(1, SC_US)) {
        resolution_value = 1000000000;
        resolution_unit = SC_NS;
    } else if (sc_get_time_resolution() < sc_time(1, SC_MS)) {
        resolution_value = 1000000;
        resolution_unit = SC_US;
    } else if (sc_get_time_resolution() < sc_time(1, SC_SEC)) {
        resolution_value = 1000;
        resolution_unit = SC_MS;
    } else {
        resolution_value = 1;
        resolution_unit = SC_SEC;
    }
}//}}}

/// Cancel operation events
void INTC2G_U2::CancelEvents (void)
{//{{{
    mAsserterr_irqEvent.cancel();
    mDeasserterr_irqEvent.cancel();
    mWriteerr_irqEvent.cancel();
}//}}}

///Calculate synchronous time
double INTC2G_U2::CalculateClockEdge (const std::string clock_name, const double time_point,const double time_change_point)
{//{{{

    double clock_period;
    if(clock_name == "PCLK") {
        clock_period = (double)mPCLKPeriod;
    }
    double period_num = 0;
    if(clock_period!=0) {
        period_num = (time_point - time_change_point) / clock_period;
    }
    else {
        return 0;
    }
    sc_dt::uint64 mod_period_num = (sc_dt::uint64)period_num;
    double edge_point = 0, mod_period_num_db = 0;
    if ((double)mod_period_num < period_num) {
        mod_period_num++;
    }
    mod_period_num_db = (double)mod_period_num;
    edge_point = mod_period_num_db * clock_period - (time_point - time_change_point);
    
    return edge_point;
}//}}}

///Separate some words from a string to store a vector
void INTC2G_U2::SeparateString (std::vector<std::string> &vtr, const std::string msg)
{//{{{
    std::istringstream cmd;
    cmd.str(msg);
    std::string temp = "a"; //Make temp to be not empty
    while (temp != "") {
        std::string command;
        cmd >> command;
        if (command != "") {
            vtr.push_back(command);
        }
        temp = command;
    }
}//}}}

/// Wrapper for accessing registers
void INTC2G_U2::RegisterHandler (const std::vector<std::string> cmd)
{//{{{
    mINTC2G_U2_Func->RegisterHandler(cmd);

}//}}}

///Execute reset operation
void INTC2G_U2::EnableReset (const std::string reset_name, const bool is_active)
{//{{{
    mINTC2G_U2_Func->Reset(is_active);
    mINTC2G_U2_DataHandler->EnableReset(is_active);
    if (is_active) {
        mmpid_num           = 8;
        merr_irq_value      = false;
        mDumpInterrupt      = false;
        mEnableRegisterMessage = true;
        CancelEvents();
        mWriteerr_irqEvent.notify(SC_ZERO_TIME);
    }
}//}}}

///Process reset function when reset port is active
void INTC2G_U2::HandleResetHardMethod (void)
{//{{{
    if (mResetCurVal == emResetActive) {
        mIsResetHardActive = true;
        re_printf("info","The reset port resetPort is asserted.\n");
        this->EnableReset("resetPort", mIsResetHardActive);
    } else {
        if (mIsResetHardActive) {
            mIsResetHardActive = false;
            re_printf("info","The reset port resetPort is de-asserted.\n");
            this->EnableReset("resetPort", mIsResetHardActive);
        }
    }
    //Cancel AssertReset command when reset port is active
    if (mIsResetCmdReceive) {
        mResetCmdEvent.cancel();
        mResetCmdCancelEvent.cancel();
        mIsResetCmdReceive = false;
        mIsResetCmdActive = false;
    }
}//}}}

///Process reset function when reset command is active
void INTC2G_U2::HandleResetCmdMethod (void)
{//{{{
    std::string reset_name = "resetPort";
    mIsResetCmdActive = true;
    re_printf("info","The model is reset by AssertReset command of %s.\n", reset_name.c_str());
    this->EnableReset(reset_name, mIsResetCmdActive);
    mResetCmdCancelEvent.notify(mResetCmdPeriod, SC_NS);
}//}}}

///Cancel reset function when reset command is active
void INTC2G_U2::CancelResetCmdMethod (void)
{//{{{
    std::string reset_name = "resetPort";
    mIsResetCmdActive = false;
    mIsResetCmdReceive = false;
    re_printf("info","Reset period of %s is over.\n", reset_name.c_str());
    this->EnableReset(reset_name, mIsResetCmdActive);
}//}}}

///Handle resetPort signal
void INTC2G_U2::HandlePresetnSignalMethod (void)
{//{{{
    mResetCurVal = resetPort.read();
    if (mResetCurVal != mResetPreVal) {   //mResetPreVal is "true" at initial state
        mResetPreVal = mResetCurVal;
        // Reset signals are asynchronous
        mResetHardEvent.notify(SC_ZERO_TIME);
    }
}//}}}

///Handle PCLK signal
void INTC2G_U2::HandlePCLKSignalMethod (void)
{//{{{
    sc_dt::uint64 freq_value = PCLK.read();
    SetCLKFreq("PCLK", freq_value, "Hz");
}//}}}

///Enable/disable dumping interrupt information
void INTC2G_U2::DumpInterrupt (const std::string is_enable)
{//{{{
    std::vector<std::string> word_vector;
    SeparateString(word_vector, is_enable);
    if (word_vector.size() == 1) {
        if (word_vector[0] == "true") {
            mDumpInterrupt = true;
        } else if (word_vector[0] == "false") {
            mDumpInterrupt = false;
        } else {
            re_printf("warning","Invalid argument: INTC2G_U2_DumpInterrupt %s\n", is_enable.c_str());
        }
    } else if (word_vector.size() == 0){
        std::string temp = (mDumpInterrupt) ? "true" : "false";
        re_printf("info","INTC2G_U2_DumpInterrupt %s\n",temp.c_str());
    } else {
        re_printf("warning","Invalid argument: INTC2G_U2_DumpInterrupt %s\n", is_enable.c_str());
    }
}//}}}

///Enable/disable dumping message when users access registers
void INTC2G_U2::DumpRegisterRW (const std::string is_enable)
{//{{{
    std::vector<std::string> cmd;
    cmd.push_back("reg");
    cmd.push_back("DumpRegisterRW");
    SeparateString(cmd, is_enable);
    mINTC2G_U2_Func->RegisterHandler(cmd);
}//}}}

/// Enable/disable dumping info/warning/error message of register IF
void INTC2G_U2::EnableRegisterMessage(const std::string is_enable)
{//{{{
    std::vector<std::string> word_vector;
    SeparateString(word_vector, is_enable);
    if (word_vector.size() == 1) {
        if (word_vector[0] == "true") {
            mEnableRegisterMessage = true;
        } else if (word_vector[0] == "false") {
            mEnableRegisterMessage = false;
        } else {
            re_printf("warning","Invalid argument: INTC2G_U2_EnableRegisterMessage %s\n", is_enable.c_str());
        }
    } else if (word_vector.size() == 0){
        std::string temp = (mEnableRegisterMessage) ? "true" : "false";
        re_printf("info","INTC2G_U2_EnableRegisterMessage %s\n",temp.c_str());
    } else {
        re_printf("warning","Invalid argument: INTC2G_U2_EnableRegisterMessage %s\n", is_enable.c_str());
    }
}//}}}

///Set message level (fatal, error, warning, info)
void INTC2G_U2::SetMessageLevel (const std::string msg_lv)
{//{{{
    std::vector<std::string> cmd;
    cmd.push_back("command");
    cmd.push_back("MessageLevel");
    SeparateString(cmd, msg_lv);
    std::string msg_return = handleCommand(cmd);
    if (msg_return != "") {
        SC_REPORT_INFO(this->basename(), msg_return.c_str());
    }

    if (mEnableRegisterMessage){
        //Set message level for Register IF class
        cmd.clear();
        cmd.push_back("reg");
        cmd.push_back("MessageLevel");
        SeparateString(cmd, msg_lv);

        mINTC2G_U2_Func->RegisterHandler(cmd);

        //Set message level for each register in Register IF class
        cmd.clear();
        cmd.push_back("reg");
        cmd.push_back("*");
        cmd.push_back("MessageLevel");
        SeparateString(cmd, msg_lv);

        mINTC2G_U2_Func->RegisterHandler(cmd);
    }else{
        //Disable message level for Register IF class (except fatal level)
        cmd.clear();
        cmd.push_back("reg");
        cmd.push_back("MessageLevel");
        cmd.push_back("fatal");

        mINTC2G_U2_Func->RegisterHandler(cmd);

        //Disable message level for each register in Register IF class (except fatal level)
        cmd.clear();
        cmd.push_back("reg");
        cmd.push_back("*");
        cmd.push_back("MessageLevel");
        cmd.push_back("fatal");

        mINTC2G_U2_Func->RegisterHandler(cmd);
    }
}//}}}

///Dump help message of all parameters or commands
void INTC2G_U2::Help (const std::string type)
{//{{{
    std::vector<std::string> word_vector;
    SeparateString(word_vector, type);
    if (word_vector.size() == 1) {
        if (word_vector[0] == "parameters") {
            SC_REPORT_INFO(this->basename(),"INTC2G_U2_MessageLevel (fatal|error|warning|info)                       Set debug message level (Default: fatal|error).");
            SC_REPORT_INFO(this->basename(),"INTC2G_U2_DumpRegisterRW (true/false)                                   Enable/disable dumping access register (Default: false).");
            SC_REPORT_INFO(this->basename(),"INTC2G_U2_DumpInterrupt (true/false)                                    Enable/disable dumping interrupt information (Default: false).");
            SC_REPORT_INFO(this->basename(),"INTC2G_U2_EnableRegisterMessage (true/false)                            Enable/disable dumping info/warning/error message of register IF (Default: true)");
            SC_REPORT_INFO(this->basename(),"INTC2G_U2_P_LOCK_SUPPORT (true/false)                                     Check value of parameter and judge LOCK bit's setting of INTC2G_U2PROT0_m register is used or not (Default: false).");
            SC_REPORT_INFO(this->basename(),"INTC2G_U2_P_MPID_NO (MPID_number)                                       This number indicate the number of MPID bit (in INTC2GPROT_m/GR register) and INTC2GMPIDn register (Default: 8).");
            SC_REPORT_INFO(this->basename(),"INTC2G_U2_P_RESET_VALUES0_n (index, value)                              Initial value of MPID register. The number of INTC2GMPID  register depend on INTC2G_U2_P_MPID_NO parameter (Default: 0).");
        } else if (word_vector[0] == "commands") {
            SC_REPORT_INFO(this->basename(),"INTC2G_U2_AssertReset (reset_name, start_time, period)                  Assert and de-assert reset signal.");
            SC_REPORT_INFO(this->basename(),"INTC2G_U2_SetCLKFreq (clock_name, freq, unit)                           Set clock frequency value of model.");
            SC_REPORT_INFO(this->basename(),"INTC2G_U2_GetCLKFreq (clock_name)                                       Get clock frequency value of model.");
            SC_REPORT_INFO(this->basename(),"INTC2G_U2_ForceRegister (reg_name, reg_value)                           Force INTC2G_U2’s register with setting value.");
            SC_REPORT_INFO(this->basename(),"INTC2G_U2_ReleaseRegister (reg_name)                                    Release INTC2G_U2’s register from force value.");
            SC_REPORT_INFO(this->basename(),"INTC2G_U2_WriteRegister (reg_name, reg_value)                           Write a value to a INTC2G_U2’s register.");
            SC_REPORT_INFO(this->basename(),"INTC2G_U2_ReadRegister (reg_name)                                       Read a value from a INTC2G_U2’s register.");
            SC_REPORT_INFO(this->basename(),"INTC2G_U2_ListRegister ()                                               Dump name of model's registers.");
            SC_REPORT_INFO(this->basename(),"INTC2G_U2_SetAreaAddress (channel_id, start_addr1, size1, start_addr2, size2)     Set Address of protected area.");
        } else {
            re_printf("warning","The name (%s) of INTC2G_U2_Help argument is wrong (commands or parameters)\n", type.c_str());
        }
    } else {
        re_printf("warning","The name (%s) of INTC2G_U2_Help argument is wrong (commands or parameters)\n", type.c_str());
    }
}//}}}

///List all registers of model
void INTC2G_U2::ListRegister (void)
{//{{{
    std::vector<std::string> cmd;
    cmd.push_back("reg");
    mINTC2G_U2_Func->RegisterHandler(cmd);
}//}}}

///Read value of register by software
void INTC2G_U2::ReadRegister (const std::string reg_name)
{//{{{
    std::vector<std::string> word_vector;
    std::vector<std::string> cmd;
    if (reg_name != "") {
        SeparateString(word_vector, reg_name);
        if (word_vector.size() == 1) {
            cmd.push_back("reg");
            SeparateString(cmd, reg_name);
            RegisterHandler(cmd);
        } else {
            re_printf("warning","Register name (%s) is invalid\n", reg_name.c_str());
        }
    } else {
        re_printf("warning","Register name (%s) is invalid\n", reg_name.c_str());
    }
}//}}}

///Write value to registers by software
void INTC2G_U2::WriteRegister (const std::string reg_name, const unsigned int reg_value)
{//{{{
    std::vector<std::string> word_vector;
    std::vector<std::string> cmd;
    if (reg_name != "") {
        SeparateString(word_vector, reg_name);
        if (word_vector.size() == 1) {
            cmd.push_back("reg");
            SeparateString(cmd, reg_name);
            std::ostringstream reg_value_str;
            reg_value_str<<"0x"<<std::hex<<reg_value;
            cmd.push_back(reg_value_str.str());
            RegisterHandler(cmd);
        } else {
            re_printf("warning","Register name (%s) is invalid\n", reg_name.c_str());
        }
    } else {
        re_printf("warning","Register name (%s) is invalid\n", reg_name.c_str());
    }
}//}}}

///Release forced value after forcing registers
void INTC2G_U2::ReleaseRegister (const std::string reg_name)
{//{{{
    std::vector<std::string> word_vector;
    std::vector<std::string> cmd;
    if (reg_name != "") {
        SeparateString(word_vector, reg_name);
        if (word_vector.size() == 1) {
            cmd.push_back("reg");
            SeparateString(cmd, reg_name);
            cmd.push_back("release");
            RegisterHandler(cmd);
        } else {
            re_printf("warning","Register name (%s) is invalid\n", reg_name.c_str());
        }
    } else {
        re_printf("warning","Register name (%s) is invalid\n", reg_name.c_str());
    }
}//}}}

///Force value to register
void INTC2G_U2::ForceRegister (const std::string reg_name, const unsigned int reg_value)
{//{{{
    std::vector<std::string> word_vector;
    std::vector<std::string> cmd;
    if (reg_name != "") {
        SeparateString(word_vector, reg_name);
        if (word_vector.size() == 1) {
            cmd.push_back("reg");
            SeparateString(cmd, reg_name);
            cmd.push_back("force");
            std::ostringstream reg_value_str;
            reg_value_str<<"0x"<<std::hex<<reg_value;
            cmd.push_back(reg_value_str.str());
            RegisterHandler(cmd);
        } else {
            re_printf("warning","Register name (%s) is invalid\n", reg_name.c_str());
        }
    } else {
        re_printf("warning","Register name (%s) is invalid\n", reg_name.c_str());
    }
}//}}}

///Get clock value
void INTC2G_U2::GetCLKFreq (const std::string clock_name)
{//{{{
    std::vector<std::string> word_vector;
    SeparateString(word_vector, clock_name);
    if (word_vector.size() == 1) {
        if (word_vector[0] == "PCLK") {
            re_printf("info","PCLK frequency is %0.0f %s\n", (double)mPCLKOrgFreq, mPCLKFreqUnit.c_str());
        } else {
            re_printf("warning","Clock name (%s) is invalid\n", word_vector[0].c_str());
        }
    } else {
        re_printf("warning","Clock name (%s) is invalid\n", clock_name.c_str());
    }
}//}}}

///Set clock value and clock unit
void INTC2G_U2::SetCLKFreq (const std::string clock_name, const sc_dt::uint64 freq, const std::string unit)
{//{{{
    std::vector<std::string> word_vector;
    SeparateString(word_vector, clock_name);
    if (word_vector.size() == 1) {
        if (word_vector[0] == "PCLK") {
            mPCLKOrgFreq = freq;
            ConvertClockFreq (mPCLKFreq, mPCLKFreqUnit, freq, unit);
            if (mPCLKFreq > 0) {
                mPCLKPeriod = (sc_dt::uint64)(((1/(double)mPCLKFreq)*(double)mTimeResolutionValue) + 0.5);
            } else {
                mPCLKPeriod = 0;
                // Cancel events
                CancelEvents();
            }
            PCLKUpdate();
        } else {
            re_printf("warning","Clock name (%s) is invalid\n", word_vector[0].c_str());
        }
    } else {
        re_printf("warning","Clock name (%s) is invalid\n", clock_name.c_str());
    }
}//}}}

///Assert reset by software
void INTC2G_U2::AssertReset (const std::string reset_name, const double start_time, const double period)
{//{{{
    if (reset_name == "resetPort") {
        if ((!mIsResetHardActive)&&(!mIsResetCmdReceive)) {
            mIsResetCmdReceive = true;
            re_printf("info","The model will be reset (resetPort) for %f ns after %f ns.\n", period, start_time);
            mResetCmdEvent.notify(start_time, SC_NS);
            mResetCmdPeriod = period;
        } else {
            re_printf("warning","The software reset of resetPort is called in the reset operation of the model. So it is ignored\n");
        }
    } else {
        re_printf("warning","The reset name (%s) is wrong. It should be resetPort\n",reset_name.c_str());
    }
}//}}}

///DumpInterrupt Message 
void INTC2G_U2::DumpInterruptMsg (const std::string intr_name, const bool value)
{//{{{
   if (mDumpInterrupt) {
        if (value) {
            re_printf("info","INT [%s] Assert.\n",intr_name.c_str());
        } else {
            re_printf("info","INT [%s] De-assert.\n",intr_name.c_str());
        }
   }
}//}}}

// Get LOCK_SUPPORT value
unsigned int INTC2G_U2::GetLOCK_SUPPORT()
{//{{{
   return mLock;
}//}}}

// Check LOCK_SUPPORT value
void INTC2G_U2::LOCK_SUPPORT(unsigned int value)
{//{{{
    mLock =  (bool)value;
}//}}}

// Get attribute of permitted register
void INTC2G_U2::GetINTC2GPROT_n(unsigned int ch_id, bool& gen, bool& dbg, bool& um, bool& wg, bool& rg, unsigned int& mpid)
{//{{{
  
    mINTC2G_U2_Func->GetINTC2GPROT_n(ch_id, gen, dbg, um, wg, rg, mpid);
}//}}}

/// Get MPID number
unsigned int INTC2G_U2::GetMpidNumber()
{//{{{
    return mmpid_num;
}//}}}

/// Set MPID number
void INTC2G_U2::SetMpidNumber(const unsigned int mpid_num)
{//{{{
    if (mpid_num >= 0 && mpid_num <= 16){
        mmpid_num = mpid_num;
    }else{
        re_printf("warning","MPID number = %d is out of range\n",mpid_num);
    }
}//}}}

/// Get SPID value
unsigned int INTC2G_U2::GetSPIDValue(unsigned int index)
{//{{{
    return mINTC2G_U2_Func->GetSPIDValue(index);
}//}}}

/// Set initial value of INTC2GMPIDm registers
void INTC2G_U2::SetResetValues0(const unsigned int index, const unsigned int value)
{//{{{
    if(index <= mmpid_num){
        mINTC2G_U2_Func->P_RESET_VALUE0_m(index, value);
    }else{
        re_printf("warning","index number = %d is out of range (larger MPID number)\n",index);
    }
}//}}}
