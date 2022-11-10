// ---------------------------------------------------------------------
// $Id: INTC2G_U2_Func.cpp,v 1.10 2018/12/26 07:03:15 huepham Exp $
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

#include "INTC2G_U2_Func.h"
#include "INTC2G_U2_AgentController.h"


///Constructor of INTC2G_U2_Func class
INTC2G_U2_Func::INTC2G_U2_Func (std::string name, INTC2G_U2_AgentController *INTC2G_U2_AgentControllerPtr, unsigned int ch_num):
                Cintc2g_u2_regif(name, 32, ch_num -2)
{//{{{
    assert(INTC2G_U2_AgentControllerPtr != NULL);
    mINTC2G_U2_AgentController = INTC2G_U2_AgentControllerPtr;
    mINTC2G_U2_AgentController->CommandInit(name);
    Cintc2g_u2_regif::set_instance_name(name);
    mModuleName = name;
    mChNum = ch_num;
}//}}}

///Destructor of INTC2G_U2_Func class
INTC2G_U2_Func::~INTC2G_U2_Func (void)
{//{{{
}//}}}

///Reset operation of INTC2G_U2_Func class
void INTC2G_U2_Func::Reset (const bool is_active)
{//{{{
    Cintc2g_u2_regif::EnableReset(is_active);
}//}}}

///*********************************
/// Function     : RegisterHandler
/// Description  : Handle commands of register IF class
///*********************************
void INTC2G_U2_Func::RegisterHandler (const std::vector<std::string>& args)
{//{{{
    std::string msg_return = "";
    if (args.size() == 1) {//Dump registers name
        vpcl::re_register *reg_p = first_reg_object();
        while (reg_p != NULL) {
            SC_REPORT_INFO(mModuleName.c_str(), reg_p->name().c_str());
            reg_p = next_reg_object();
        }
    } else {
        if (args[1] == "*") {//Set message level for each register
            std::vector<std::string> cmd;
            vpcl::re_register *reg_p = first_reg_object();
            while (reg_p != NULL) {
                cmd = args;
                cmd.erase(cmd.begin()+1);
                cmd.insert(cmd.begin()+1, reg_p->name());
                msg_return = Cintc2g_u2_regif::reg_handle_command(cmd);
                if (msg_return != "") {
                    SC_REPORT_INFO(mModuleName.c_str(), msg_return.c_str());
                }
                cmd.clear();
                reg_p = next_reg_object();
            }
        } else if (args[1] == "DumpRegisterRW") {
                msg_return = Cintc2g_u2_regif::reg_handle_command(args);
                if (msg_return != "") {
                    SC_REPORT_INFO(mModuleName.c_str(), msg_return.c_str());
                }
        } else {
            msg_return = Cintc2g_u2_regif::reg_handle_command(args);
            if (msg_return != "") {
                SC_REPORT_INFO(mModuleName.c_str(), msg_return.c_str());
            }
        }
    }
}//}}}

///Virtual function of BusSlaveFuncIf class
void INTC2G_U2_Func::read (unsigned int offsetAddress, TlmBasicPayload& trans, BusTime_t* t, bool debug)
{//{{{
    memset(trans.get_data_ptr(), 0, trans.get_data_length());
    if (!debug) {
        reg_rd((unsigned int)trans.get_address(), trans.get_data_ptr(), trans.get_data_length());
    } else {
        reg_rd_dbg((unsigned int)trans.get_address(), trans.get_data_ptr(), trans.get_data_length());
    }
}//}}}

///Virtual function of BusSlaveFuncIf class
void INTC2G_U2_Func::write (unsigned int offsetAddress, TlmBasicPayload& trans, BusTime_t* t, bool debug)
{//{{{
    if (mINTC2G_U2_AgentController->CheckClockPeriod("PCLK")) {
        if (!debug) {
            reg_wr((unsigned int)trans.get_address(), trans.get_data_ptr(), trans.get_data_length());
        }
        else {
            reg_wr_dbg((unsigned int)trans.get_address(), trans.get_data_ptr(), trans.get_data_length());
        }
    }
    else {
        re_printf("warning", "Cannot access register when clock is 0.\n");
        return;
    }
}//}}}

// operate P_RESET_VALUE0_m parameter.
void INTC2G_U2_Func::P_RESET_VALUE0_m(unsigned int index, unsigned int value)
{//{{{
    // Set initial value for INTC2GMPIDm registers
    (*INTC2GMPID[index]).UpdateBitInitValue("SPID", value&0x1F);
}//}}}

// Get spid value
unsigned int INTC2G_U2_Func::GetSPIDValue(unsigned int index)
{//{{{
    return ((unsigned int)(*INTC2GMPID[index])["SPID"]);
}//}}}

// call back write of INTC2GMPID_SPID
void INTC2G_U2_Func::cb_INTC2GMPID_SPID(RegCBstr str)
{//{{{
    unsigned int mpid_num = mINTC2G_U2_AgentController->GetMpidNumber();
    if (str.is_wr) {
        if(str.channel >= mpid_num){
            re_printf("warning", "Cannot access reserved area\n");
            (*INTC2GMPID[str.channel]) = str.pre_data;
            return;
        }else{
            if ((*INTC2GKCPROT)["KCE"] == 0) {
                (*INTC2GMPID[str.channel]) = str.pre_data;
            }
        }
    }else{
        if(str.channel >= mpid_num){
            re_printf("warning", "Cannot access reserved area\n");
            return;
        }
    }
}//}}}

// call back write of INTC2GOVFCLR_CLRO
void INTC2G_U2_Func::cb_INTC2GOVFCLR_CLRO(RegCBstr str)
{//{{{
    unsigned int CLRO_cur = bit_select(str.data
                            ,(*INTC2GOVFCLR)["CLRO"].mStartAddr
                            ,(*INTC2GOVFCLR)["CLRO"].mEndAddr);
    if (1 == CLRO_cur){
        (*INTC2GOVFSTAT)["OVF"] = 0;
    }
}//}}}

// call back write of INTC2GSPIDERRCLR_SPIDCLR
void INTC2G_U2_Func::cb_INTC2GSPIDERRCLR_SPIDCLR(RegCBstr str)
{//{{{
    unsigned int SPIDCLR_cur = (unsigned int) str.data;
    (*INTC2GSPIDERRSTAT)["SPIDERR"] = (~SPIDCLR_cur)&((unsigned int)(*INTC2GSPIDERRSTAT)["SPIDERR"]);
}//}}}

// call back write of INTC2GKCPROT_KCPROT
void INTC2G_U2_Func::cb_INTC2GKCPROT_KCPROT(RegCBstr str)
{//{{{
    if((*INTC2GKCPROT)["KCPROT"] != (0xA5A5A500 >> 1)) {
        (*INTC2GKCPROT) = str.pre_data;
    }
    // Response data is always 0 when reading KCPROT
    (*INTC2GKCPROT)["KCPROT"] = 0;
}//}}}

// Get permit register value
void INTC2G_U2_Func::GetINTC2GPROT_n(unsigned int channel_id, bool& gen, bool& dbg, bool& um, bool& wg, bool& rg,unsigned int& mpid)
{//{{{
    if (channel_id == 0) {
        gen = (bool)((*INTC2GPROT_IMR)["GEN"]);
        dbg = (bool)((*INTC2GPROT_IMR)["DBG"]);
        um = (bool)((*INTC2GPROT_IMR)["UM"]);
        wg = (bool)((*INTC2GPROT_IMR)["WG"]);
        rg = (bool)((*INTC2GPROT_IMR)["RG"]);
        mpid = (unsigned int)((*INTC2GPROT_IMR)["MPID"]);
    }else if (channel_id == 1) {
        gen = (bool)((*INTC2GPROT_GR)["GEN"]);
        dbg = (bool)((*INTC2GPROT_GR)["DBG"]);
        um = (bool)((*INTC2GPROT_GR)["UM"]);
        wg = (bool)((*INTC2GPROT_GR)["WG"]);
        rg = (bool)((*INTC2GPROT_GR)["RG"]);
        mpid = (unsigned int)((*INTC2GPROT_GR)["MPID"]);
    }else if (channel_id >= 2 && channel_id <= 2017) {
        unsigned int channel_index = channel_id + 30;
        gen = (bool)((*INTC2GPROT_[channel_index])["GEN"]);
        dbg = (bool)((*INTC2GPROT_[channel_index])["DBG"]);
        um = (bool)((*INTC2GPROT_[channel_index])["UM"]);
        wg = (bool)((*INTC2GPROT_[channel_index])["WG"]);
        rg = (bool)((*INTC2GPROT_[channel_index])["RG"]);
        mpid = (unsigned int)((*INTC2GPROT_[channel_index])["MPID"]);
    }
}//}}}

///Call back write of INTC2GPROT_GR_MPID
void INTC2G_U2_Func::cb_INTC2GPROT_GR_MPID(RegCBstr str)
{//{{{   
    unsigned int mpid_num = mINTC2G_U2_AgentController->GetMpidNumber();
    unsigned int GR_register = (unsigned int)(*INTC2GPROT_GR);
 
    // process for LOCK bit
        if ((unsigned int)(*INTC2GKCPROT)["KCE"]){
            if (1 == mINTC2G_U2_AgentController->GetLOCK_SUPPORT()){
                if(mpid_num <= 15){
                    if((((str.pre_data)>>31)&0x1)){
                        (*INTC2GPROT_GR) = str.pre_data;
                    }else if(((*INTC2GPROT_GR)>>31)&0x1){
                        (*INTC2GPROT_GR)["MPID"] = ((((1 << mpid_num) - 1 ) & (*INTC2GPROT_GR)["MPID"]) |(1<<15));
                    }else if(!(((*INTC2GPROT_GR)>>31)&0x1)){
                        (*INTC2GPROT_GR)["MPID"] = (((1 << mpid_num) - 1 ) & (*INTC2GPROT_GR)["MPID"]);
                    }
                }else{
                    (*INTC2GPROT_GR)["MPID"] = ((((1 << mpid_num) - 1 ) & (*INTC2GPROT_GR)["MPID"]));
                }
            }else{
                (*INTC2GPROT_GR)["MPID"] = ((1 << mpid_num) - 1 ) & (*INTC2GPROT_GR)["MPID"];
            }    
        }else {
            (*INTC2GPROT_GR) = str.pre_data;
        }
}//}}}

///Call back write of INTC2GPROT_IMR_MPID
void INTC2G_U2_Func::cb_INTC2GPROT_IMR_MPID(RegCBstr str)
{//{{{   
    unsigned int mpid_num = mINTC2G_U2_AgentController->GetMpidNumber();
    unsigned int IMR_register = (unsigned int)(*INTC2GPROT_IMR);
 
    // process for LOCK bit
        if ((unsigned int)(*INTC2GKCPROT)["KCE"]){
            if (1 == mINTC2G_U2_AgentController->GetLOCK_SUPPORT()){
                if(mpid_num <= 15){
                    if((((str.pre_data)>>31)&0x1)){
                        (*INTC2GPROT_IMR) = str.pre_data;
                    }else if(((*INTC2GPROT_IMR)>>31)&0x1){
                        (*INTC2GPROT_IMR)["MPID"] = ((((1 << mpid_num) - 1 ) & (*INTC2GPROT_IMR)["MPID"]) |(1<<15));
                    }else if(!(((*INTC2GPROT_IMR)>>31)&0x1)){
                        (*INTC2GPROT_IMR)["MPID"] = (((1 << mpid_num) - 1 ) & (*INTC2GPROT_IMR)["MPID"]);
                    }
                }else{
                    (*INTC2GPROT_IMR)["MPID"] = ((((1 << mpid_num) - 1 ) & (*INTC2GPROT_IMR)["MPID"]));
                }
            }else{
                (*INTC2GPROT_IMR)["MPID"] = ((1 << mpid_num) - 1 ) & (*INTC2GPROT_IMR)["MPID"];
            }    
        }else {
            (*INTC2GPROT_IMR) = str.pre_data;
        }
}//}}}

///Call back write of INTC2GPROT__MPID
void INTC2G_U2_Func::cb_INTC2GPROT__MPID(RegCBstr str)
{//{{{   
    unsigned int mpid_num = mINTC2G_U2_AgentController->GetMpidNumber();
 
    // process for LOCK bit
        if ((unsigned int)(*INTC2GKCPROT)["KCE"]){
            if (1 == mINTC2G_U2_AgentController->GetLOCK_SUPPORT()){
                if(mpid_num <= 15){
                    if((((str.pre_data)>>31)&0x1)){
                        (*INTC2GPROT_[str.channel]) = str.pre_data;
                    }else if(((*INTC2GPROT_[str.channel])>>31)&0x1){
                        (*INTC2GPROT_[str.channel])["MPID"] = ((((1 << mpid_num) - 1 ) & (*INTC2GPROT_[str.channel])["MPID"]) | (1<<15));
                    }else if(!(((*INTC2GPROT_[str.channel])>>31)&0x1)){
                        (*INTC2GPROT_[str.channel])["MPID"] = (((1 << mpid_num) - 1 ) & (*INTC2GPROT_[str.channel])["MPID"]);
                    }
                }else{
                    (*INTC2GPROT_[str.channel])["MPID"] = ((1 << mpid_num) - 1 ) & (*INTC2GPROT_[str.channel])["MPID"];
                }
            }else{
                (*INTC2GPROT_[str.channel])["MPID"] = ((1 << mpid_num) - 1 ) & (*INTC2GPROT_[str.channel])["MPID"];
            }    
        }else {
            (*INTC2GPROT_[str.channel]) = str.pre_data;
        }
}//}}}

///Set value to specify bit
void INTC2G_U2_Func::SetVar(const std::string name, const unsigned int val)
{//{{{
    uint32_t SPIDERR_cur = (uint32_t)(*INTC2GSPIDERRSTAT)["SPIDERR"]; 
    if(name=="DBG") {
        (*INTC2GERRTYPE)["DBG"] = val;
    }
    else if(name=="SEC") {
        (*INTC2GERRTYPE)["SEC"] = val;
    }
    else if(name=="UM") {
        (*INTC2GERRTYPE)["UM"] = val;
    }
    else if(name=="SPID") {
        (*INTC2GERRTYPE)["SPID"] = val;
    }
    else if(name=="WRITE") {
        (*INTC2GERRTYPE)["WRITE"] = val;
    }
    else if(name=="ADDR") {
        (*INTC2GERRADDR) = val;
    }
    else if(name=="SPIDERR") {
        if((unsigned int) ((*INTC2GSPIDERRSTAT)["SPIDERR"]) != 0) {
            (*INTC2GOVFSTAT)["OVF"] = 0x1;
        }
        (*INTC2GSPIDERRSTAT)["SPIDERR"] = SPIDERR_cur | val;
    }
}//}}}

//Get (*INTC2GSPIDERRSTAT)["SPIDERR"] register
unsigned int INTC2G_U2_Func::Get_SPIDERR(void)
{//{{{
    return (unsigned int)(*INTC2GSPIDERRSTAT)["SPIDERR"];
}//}}}
