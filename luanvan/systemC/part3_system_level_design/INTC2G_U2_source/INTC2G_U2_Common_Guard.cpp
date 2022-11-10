// ---------------------------------------------------------------------
// $Id: INTC2G_U2_Common_Guard.cpp,v 1.17 2018/12/28 01:14:14 huepham Exp $
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

#include <cstdarg>
#include "INTC2G_U2_AgentController.h"
#include "INTC2G_U2_Common_Guard.h"
#include "BusMasterIf.h"
#include <iomanip>
#include "INTC2G_U2.h"

///Constructor of INTC2G_U2_Common_Guard class
INTC2G_U2_Common_Guard::INTC2G_U2_Common_Guard(std::string name,
                         INTC2G_U2_AgentController* INTC2G_U2_AgentControllerPtr,
                         BusBridgeModuleBase<32,32> *Parent,
                         BusBridgeSlaveIf<32> *SlaveIf,
                         BusMasterIf<32> *MasterIf,
                         SIM_MODE_T simmode,
                         unsigned int ch_num):
                         //sc_module(name),
                         BusBridgeCoreBase<32,32>(Parent,SlaveIf,MasterIf)
{//{{{
    assert((INTC2G_U2_AgentControllerPtr != NULL) && (MasterIf != NULL) && (SlaveIf != NULL));
    mINTC2G_U2_AgentController = INTC2G_U2_AgentControllerPtr;
    BusBridgeCoreBase<32,32>::setTlmTransType(simmode);
    mMasterIf = MasterIf;
    mSlaveIf = SlaveIf;
    mTransTime = SC_ZERO_TIME;
    mTrans = (TlmBasicPayload*)0;
    mIsDebug = false;
    mChannelNum = ch_num;

    mRstState = false;
    mCLKPeriod = 0;
	mSECPara = true; // P_SEC_DIS default equal 1
    Initialize();
}//}}}

///Notify transaction completion in AT mode
void INTC2G_U2_Common_Guard::TransferComplete(void)
{//{{{
    if ((BusBridgeCoreBase<32,32>::mTransType == AT) && (!mIsDebug)) {//AT mode and non-debug transaction
        mSlaveIf->notifyTransComp(*mTrans,mTransTime);
    }
}//}}}

///Set transaction basic info (address, data, length)
void INTC2G_U2_Common_Guard::SetTransBasic(unsigned int access_address,unsigned int data_length , unsigned int data, TlmBasicPayload &outTrans)
{//{{{
      
    unsigned char* data_ptr = 0;
    data_ptr = new unsigned char [1024];
    memset(data_ptr,0,1024);
    memcpy(data_ptr, &data, 4);
    outTrans.set_address(access_address);
    outTrans.set_data_ptr(data_ptr);
    outTrans.set_data_length(data_length);
}//}}}

///Set transaction basic info from inTrans to outTrans.
void INTC2G_U2_Common_Guard::UpdateTransBasic(TlmBasicPayload &inTrans, TlmBasicPayload &outTrans)
{//{{{
    outTrans.set_address(inTrans.get_address());
    outTrans.set_data_ptr(inTrans.get_data_ptr());
    outTrans.set_data_length(inTrans.get_data_length());
}//}}}

///Set transaction G3M extension
void INTC2G_U2_Common_Guard::SetTrans(TlmBasicPayload &inTrans, TlmBasicPayload &outTrans)
{//{{{
    TlmG3mExtension *inG3mExt  = (TlmG3mExtension *)0;
    TlmG3mExtension *outG3mExt = new TlmG3mExtension;
    inTrans.get_extension(inG3mExt);
    if (inG3mExt != NULL) {
        outG3mExt->setDmaAccess(inG3mExt->isDmaAccess());
        outG3mExt->setVmId(inG3mExt->getVmId());
        outG3mExt->setTcId(inG3mExt->getTcId());
        outG3mExt->setPeId(inG3mExt->getPeId());
        outG3mExt->setSpId(inG3mExt->getSpId());
        outG3mExt->setUserMode(inG3mExt->isUserMode());
        outG3mExt->setVirtualMode(inG3mExt->isVirtualMode());
        outG3mExt->setSecure(inG3mExt->isSecure());
        outTrans.set_extension(outG3mExt);
    }
    TlmApbExtension* mApbExt = (TlmApbExtension*)0;
    inTrans.get_extension(mApbExt);
    if (mApbExt != NULL) {
        TlmApbExtension* mApbExtOut = static_cast<TlmApbExtension*>(mApbExt->clone());
        outTrans.set_extension(mApbExtOut);
    }
}//}}}

///Overwrite virtual functions of BusBridgeCoreBase class
void INTC2G_U2_Common_Guard::read(unsigned int offsetAddress,TlmBasicPayload& trans,BusTime_t* t,bool debug)
{//{{{
    mTrans = &trans;
    mTransTime = *t;
    mIsDebug = debug;
    unsigned char* data = trans.get_data_ptr();
    unsigned int temp = 0;
    memcpy(&temp, data, 4);
    unsigned int update_data = (temp >> 16) & 0xFFFF;
    unsigned int data_length = trans.get_data_length();
    unsigned int data_address = trans.get_address();
    bool is_continue = false;
    if (mCLKPeriod != 0){
        if (debug) {
            is_continue = true;
        } else {
            if (mRstState != false) {
                TransferComplete();
                SC_REPORT_WARNING("INTC2G_U2_Common_Guard","The reading transaction is fail because the reset is active");
            } else {
                is_continue = true;
            }
        }
        if (is_continue) {
            unsigned int ch_id;
            bool is_found_channel = CheckTransAddress(data_address,ch_id);
            if (!is_found_channel){
                if (CheckAccessibility(data_address,trans,debug)) {
                    if (debug || (BusBridgeCoreBase<32,32>::mTransType == LT)) {// LT mode
                        mMasterIf->read(trans,debug,*t,false);
                    } else {//AT mode
                        TlmBasicPayload *sendTrans = new TlmBasicPayload;
                        UpdateTransBasic(trans, *sendTrans );
                        SetTrans(trans, *sendTrans );
                        mRequestMap[sendTrans] = &trans;
                        mMasterIf->read(*sendTrans,debug,*t,false);
                    }
                }    
            }else{
                if(ch_id >= 2 && ch_id <= 2017){
                    // check channel from 2 to 2017 before separating 2 trans
                    if (data_length == 4){
                        //there is no guard violation both 2 data side
                        bool even_valid = CheckAccessibility(data_address,trans,debug);
                        bool odd_valid = CheckAccessibility(data_address + 0x2,trans,debug);
                        if (!odd_valid){
                            mTypeADDR = data_address;
                        }
                        if (odd_valid && even_valid) {
                            if (debug || (BusBridgeCoreBase<32,32>::mTransType == LT)) {// LT mode
                                TlmBasicPayload *newTrans = new TlmBasicPayload;
                                SetTransBasic(data_address, 4, temp, *newTrans );
                                SetTrans(trans, *newTrans );
                                mMasterIf->read(*newTrans,debug,*t,false);
                                //after finish trans, update the trans accroding to newTrans
                                UpdateTransBasic(*newTrans,trans);
                                SetTrans(*newTrans, trans );
                                deleteTrans();
                                mClearTransFifo.push(newTrans);
                            } else {//AT mode
                                TlmBasicPayload *sendTrans = new TlmBasicPayload;
                                SetTransBasic(data_address, 4, temp, *sendTrans );
                                SetTrans(trans, *sendTrans );
                                mRequestMap[sendTrans] = &trans;
                                mMasterIf->read(*sendTrans,debug,*t,false);
                            }
                        //both even and odd data guard violations
                        }else if (!odd_valid && !even_valid) {
                            mTransErrorEvent.notify(SC_ZERO_TIME);
                            TransferComplete();
                        //even data guard violations only
                        }else if (odd_valid && !even_valid) {
                            mTransErrorEvent.notify(SC_ZERO_TIME);
                            if (debug || (BusBridgeCoreBase<32,32>::mTransType == LT)) {// LT mode
                                TlmBasicPayload *newTrans = new TlmBasicPayload;
                                SetTransBasic(data_address + 0x2, 2, update_data, *newTrans );
                                SetTrans(trans, *newTrans );
                                mMasterIf->read(*newTrans,debug,*t,false);
                                unsigned char* data_ptr = newTrans->get_data_ptr();
                                unsigned int temp = 0;
                                memcpy(&temp, data_ptr, 4);
                                unsigned int update_data = (temp << 16) & 0xFFFFFFFF;
                                // after finish trans, update the trans accroding to newTrans
                                SetTransBasic(data_address, 4, update_data, trans );
                                SetTrans(*newTrans, trans );
                                deleteTrans();
                                mClearTransFifo.push(newTrans);
                            } else {//AT mode
                                TlmBasicPayload *sendTrans = new TlmBasicPayload;
                                SetTransBasic(data_address + 0x2, 2, update_data,  *sendTrans );
                                SetTrans(trans, *sendTrans );
                                mRequestMap[sendTrans] = &trans;
                                mMasterIf->read(*sendTrans,debug,*t,false);
                            }
                        //odd data guard violations only
                        }else if (!odd_valid && even_valid) {
                            if (debug || (BusBridgeCoreBase<32,32>::mTransType == LT)) {// LT mode
                                TlmBasicPayload *newTrans = new TlmBasicPayload;
                                SetTransBasic(data_address, 2, temp, *newTrans );
                                SetTrans(trans, *newTrans );
                                mMasterIf->read(*newTrans,debug,*t,false);
                                // after finish trans, update the trans accroding to newTrans
                                unsigned char* data_ptr = newTrans->get_data_ptr();
                                unsigned int temp = 0;
                                memcpy(&temp, data_ptr, 4);
                                SetTransBasic(data_address, 4, temp, trans );
                                SetTrans(*newTrans, trans );
                                deleteTrans();
                                mClearTransFifo.push(newTrans);
                            } else {//AT mode
                                TlmBasicPayload *sendTrans = new TlmBasicPayload;
                                SetTransBasic(data_address, 2, temp, *sendTrans );
                                SetTrans(trans, *sendTrans );
                                mRequestMap[sendTrans] = &trans;
                                mMasterIf->read(*sendTrans,debug,*t,false);
                            }
                        }
                    }else if (data_length == 1 || data_length == 2){
                        //check and send to slave, not split data size.
                        if (CheckAccessibility(data_address,trans,debug)) {
                            if (debug || (BusBridgeCoreBase<32,32>::mTransType == LT)) {// LT mode
                                TlmBasicPayload *newTrans = new TlmBasicPayload;
                                SetTransBasic(data_address,data_length, temp,  *newTrans );
                                SetTrans(trans, *newTrans );
                                mMasterIf->read(*newTrans,debug,*t,false);
                                // after finish trans, update the trans accroding to newTrans
                                UpdateTransBasic(*newTrans,trans);
                                SetTrans(*newTrans, trans );
                                deleteTrans();
                                mClearTransFifo.push(newTrans);
                            } else {//AT mode
                                TlmBasicPayload *sendTrans = new TlmBasicPayload;
                                SetTransBasic(data_address,data_length, temp, *sendTrans );
                                SetTrans(trans, *sendTrans );
                                mRequestMap[sendTrans] = &trans;
                                mMasterIf->read(*sendTrans,debug,*t,false);
                            }
                        } else {
                            mTransErrorEvent.notify(SC_ZERO_TIME);
                            TransferComplete();
                        }
                   }     
                   }else if(ch_id == 0 || ch_id == 1){
                       //check and send to slave, not split data size.
                       if (CheckAccessibility(data_address,trans,debug)) {
                           if (debug || (BusBridgeCoreBase<32,32>::mTransType == LT)) {// LT mode
                               TlmBasicPayload *newTrans = new TlmBasicPayload;
                               SetTransBasic(data_address,data_length, temp, *newTrans );
                               SetTrans(trans, *newTrans );
                               mMasterIf->read(*newTrans,debug,*t,false);
                               // after finish trans, update the trans accroding to newTrans
                               UpdateTransBasic(*newTrans,trans);
                               SetTrans(*newTrans, trans );
                               deleteTrans();
                               mClearTransFifo.push(newTrans);
                           } else {//AT mode
                               TlmBasicPayload *sendTrans = new TlmBasicPayload;
                               SetTransBasic(data_address,data_length, temp, *sendTrans );
                               SetTrans(trans, *sendTrans );
                               mRequestMap[sendTrans] = &trans;
                               mMasterIf->read(*sendTrans,debug,*t,false);
                           }
                       } else {
                           mTransErrorEvent.notify(SC_ZERO_TIME);
                           TransferComplete();
                       }
                  }
             }
        }
    }     
}//}}}

///Overwrite virtual functions of BusBridgeCoreBase class
void INTC2G_U2_Common_Guard::write(unsigned int offsetAddress,TlmBasicPayload& trans,BusTime_t* t,bool debug)
{//{{{
    mTrans = &trans;
    mTransTime = *t;
    mIsDebug = debug;
    unsigned char* data = trans.get_data_ptr();
    unsigned int temp = 0;
    memcpy(&temp, data, 4);
    unsigned int update_data = (temp >> 16) & 0xFFFF;
    unsigned int data_length = trans.get_data_length();
    unsigned int data_address = trans.get_address();

    unsigned int ch_id;
    bool is_continue = false;
    if(mCLKPeriod != 0){
        if (debug) {
            is_continue = true;
        } else {
            if (mRstState != false) {
                TransferComplete();
                SC_REPORT_WARNING("INTC2G_U2_Common_Guard","The writing transaction is fail because the reset is active");
            } else {
                is_continue = true;
            }
        }
        if (is_continue) {
            bool is_found_channel = CheckTransAddress(data_address,ch_id);
            if (!is_found_channel){
                if (CheckAccessibility(data_address,trans,debug)) {
                    if (debug || (BusBridgeCoreBase<32,32>::mTransType == LT)) {// LT mode
                        mMasterIf->write(trans,debug,*t,false);
                    } else {//AT mode
                        TlmBasicPayload *sendTrans = new TlmBasicPayload;
                        UpdateTransBasic(trans, *sendTrans );
                        SetTrans(trans, *sendTrans );
                        mRequestMap[sendTrans] = &trans;
                        mMasterIf->write(*sendTrans,debug,*t,false);
                    }
                }    
            }else{
                if(ch_id >= 2 && ch_id <= 2017){
                    // check channel from 2 to 2017 before separating 2 trans
                    if (data_length == 4){
                        //there is no guard violation both 2 data side
                        bool even_valid = CheckAccessibility(data_address,trans,debug);
                        bool odd_valid = CheckAccessibility(data_address + 0x2,trans,debug);
                        if (!odd_valid){
                            mTypeADDR = data_address;
                        }
                        if (odd_valid && even_valid) {
                            if (debug || (BusBridgeCoreBase<32,32>::mTransType == LT)) {// LT mode
                                TlmBasicPayload *newTrans = new TlmBasicPayload;
                                SetTransBasic(data_address, 4, temp, *newTrans );
                                SetTrans(trans, *newTrans );
                                mMasterIf->write(*newTrans,debug,*t,false);
                                deleteTrans();
                                mClearTransFifo.push(newTrans);
                            } else {//AT mode
                                TlmBasicPayload *sendTrans = new TlmBasicPayload;
                                SetTransBasic(data_address, 4, temp, *sendTrans );
                                SetTrans(trans, *sendTrans );
                                mRequestMap[sendTrans] = &trans;
                                mMasterIf->write(*sendTrans,debug,*t,false);
                            }
                        //both even and odd data guard violations
                        }else if (!odd_valid && !even_valid) {
                            mTransErrorEvent.notify(SC_ZERO_TIME);
                            TransferComplete();
                        //even data guard violations only
                        }else if (odd_valid && !even_valid) {
                            mTransErrorEvent.notify(SC_ZERO_TIME);
                            if (debug || (BusBridgeCoreBase<32,32>::mTransType == LT)) {// LT mode
                                TlmBasicPayload *newTrans = new TlmBasicPayload;
                                SetTransBasic(data_address + 0x2, 2, update_data, *newTrans );
                                SetTrans(trans, *newTrans );
                                mMasterIf->write(*newTrans,debug,*t,false);
                                deleteTrans();
                                mClearTransFifo.push(newTrans);
                            } else {//AT mode
                                TlmBasicPayload *sendTrans = new TlmBasicPayload;
                                SetTransBasic(data_address + 0x2, 2, update_data,  *sendTrans );
                                SetTrans(trans, *sendTrans );
                                mRequestMap[sendTrans] = &trans;
                                mMasterIf->write(*sendTrans,debug,*t,false);
                            }
                        //odd data guard violations only
                        }else if (!odd_valid && even_valid) {
                            if (debug || (BusBridgeCoreBase<32,32>::mTransType == LT)) {// LT mode
                                TlmBasicPayload *newTrans = new TlmBasicPayload;
                                SetTransBasic(data_address, 2, temp, *newTrans );
                                SetTrans(trans, *newTrans );
                                mMasterIf->write(*newTrans,debug,*t,false);
                                deleteTrans();
                                mClearTransFifo.push(newTrans);
                            } else {//AT mode
                                TlmBasicPayload *sendTrans = new TlmBasicPayload;
                                SetTransBasic(data_address, 2, temp, *sendTrans );
                                SetTrans(trans, *sendTrans );
                                mRequestMap[sendTrans] = &trans;
                                mMasterIf->write(*sendTrans,debug,*t,false);
                            }
                        }
                    }else if (data_length == 1 || data_length == 2){
                        //check and send to slave, not split data size.
                        if (CheckAccessibility(data_address,trans,debug)) {
                            if (debug || (BusBridgeCoreBase<32,32>::mTransType == LT)) {// LT mode
                                TlmBasicPayload *newTrans = new TlmBasicPayload;
                                SetTransBasic(data_address,data_length, temp,  *newTrans );
                                SetTrans(trans, *newTrans );
                                mMasterIf->write(*newTrans,debug,*t,false);
                                deleteTrans();
                                mClearTransFifo.push(newTrans);
                            } else {//AT mode
                                TlmBasicPayload *sendTrans = new TlmBasicPayload;
                                SetTransBasic(data_address,data_length, temp, *sendTrans );
                                SetTrans(trans, *sendTrans );
                                mRequestMap[sendTrans] = &trans;
                                mMasterIf->write(*sendTrans,debug,*t,false);
                            }
                        } else {
                            mTransErrorEvent.notify(SC_ZERO_TIME);
                            TransferComplete();
                        }
                   }     
                   } else if(ch_id == 0 || ch_id == 1){
                        //check and send to slave, not split data size.
                        if (CheckAccessibility(data_address,trans,debug)) {
                            if (debug || (BusBridgeCoreBase<32,32>::mTransType == LT)) {// LT mode
                                TlmBasicPayload *newTrans = new TlmBasicPayload;
                                SetTransBasic(data_address,data_length, temp, *newTrans );
                                SetTrans(trans, *newTrans );
                                mMasterIf->write(*newTrans,debug,*t,false);
                                deleteTrans();
                                mClearTransFifo.push(newTrans);
                            } else {//AT mode
                                TlmBasicPayload *sendTrans = new TlmBasicPayload;
                                SetTransBasic(data_address,data_length, temp, *sendTrans );
                                SetTrans(trans, *sendTrans );
                                mRequestMap[sendTrans] = &trans;
                                mMasterIf->write(*sendTrans,debug,*t,false);
                            }
                        } else {
                            mTransErrorEvent.notify(SC_ZERO_TIME);
                            TransferComplete();
                        }
                   }
              }
         }
    }     
}//}}}

///Overwrite virtual functions of BusBridgeCoreBase class
void INTC2G_U2_Common_Guard::deleteTrans(void)
{//{{{
    TlmBasicPayload *trans;
    TlmG3mExtension *G3mExt;
    while(!mClearTransFifo.empty()){
        trans = mClearTransFifo.front();
        G3mExt = (TlmG3mExtension *)0;
        trans->get_extension(G3mExt);
        trans->release_extension(G3mExt);
        mRequestMap.erase(trans);
        mClearTransFifo.pop();
        delete trans;
    }
}//}}}

///Overwrite virtual functions of BusBridgeCoreBase class
void INTC2G_U2_Common_Guard::notifyTransComp(TlmBasicPayload &trans, BusTime_t &t)
{//{{{
    if(!mRequestMap.empty()){
        // in AT mode, update the orignial trans from the newTran.
        // the trans in input parameter is newTrans, the mRequestMap[trans] is original trans.
        // after finish trans, update the trans accroding to newTrans
        unsigned char* data_ptr = trans.get_data_ptr();
        unsigned char* update_data_ptr;
        update_data_ptr = new unsigned char [1024];
        memset(update_data_ptr,0,1024);
        unsigned int temp = 0;
        memcpy(&temp, data_ptr, 4);
        unsigned int update_data = (temp << 16) & 0xFFFFFFFF;
        memcpy(update_data_ptr, &update_data, 4);
        unsigned int org_length = (*mRequestMap[&trans]).get_data_length();
        unsigned int data_address = (*mRequestMap[&trans]).get_address();
        unsigned int ch_id;
        bool is_found_channel = CheckTransAddress(data_address,ch_id);
        bool is_read = trans.is_read();
       
        if (!is_found_channel){// not match any channel
            (*mRequestMap[&trans]).set_data_ptr(data_ptr);
        }else if (is_read){
            if(ch_id >= 2 && ch_id <= 2017){
                // check channel from 2 to 2017 before separating 2 trans
                if (org_length == 4){
                    //there is no guard violation both 2 data side
                    bool even_valid = CheckAccessibility(data_address,trans,mIsDebug);
                    bool odd_valid = CheckAccessibility(data_address + 0x2,trans,mIsDebug);
                    if (odd_valid && even_valid) {
                        //after finish trans, update the trans accroding to newTrans
                        (*mRequestMap[&trans]).set_data_ptr(data_ptr);
                        //even data guard violations only
                    }else if (odd_valid && !even_valid) {
                        (*mRequestMap[&trans]).set_data_ptr(update_data_ptr);
                        //odd data guard violations only
                    }else if (!odd_valid && even_valid) {
                        (*mRequestMap[&trans]).set_data_ptr(data_ptr);
                    }
                }else if (org_length == 1 || org_length == 2){
                    (*mRequestMap[&trans]).set_data_ptr(data_ptr);
                } 
            }else if(ch_id == 0 || ch_id == 1){
                //check and send to slave, not split data size.
                (*mRequestMap[&trans]).set_data_ptr(data_ptr);
            }
        }
        mSlaveIf->notifyTransComp(*mRequestMap[&trans],t);
        deleteTrans();
        mClearTransFifo.push(&trans);
    }
}//}}}

///Check access ability to peripherals
bool INTC2G_U2_Common_Guard::CheckAccessibility(unsigned int access_addr, TlmBasicPayload &trans, bool debug)
{//{{{
    bool is_access_permitted = true;
    bool is_pe_trans = true;
    bool is_read = trans.is_read();
    bool is_write = trans.is_write();
    bool trans_type;
    if ((is_read) && (!is_write)){
        trans_type = false; // read trans
    }
    else {
        trans_type = true; // write trans
    }

    TlmG3mExtension *ExtensionInfo;
    trans.get_extension(ExtensionInfo);
    assert(ExtensionInfo != NULL);
    bool access_um = ExtensionInfo->isUserMode();
    unsigned char access_spid = ExtensionInfo->getSpId();
//    unsigned int addr = (unsigned int)trans.get_address();
    unsigned int addr = access_addr;
    unsigned int ch_id;

    bool is_found_channel = CheckTransAddress(addr,ch_id);
    if(is_found_channel && ch_id<mChannelNum) {
        bool gen_bit = false;
        bool dbg_bit = false;
        bool um_bit = false;
        bool wg_bit = false;
        bool rg_bit = false;
        unsigned int mpid = 0;
        unsigned int spidValue;
        unsigned int mpid_num = mINTC2G_U2_AgentController->GetMpidNumber();

        mINTC2G_U2_AgentController->GetINTC2GPROT_n(ch_id, gen_bit, dbg_bit, um_bit, wg_bit, rg_bit, mpid);
        if (1 == gen_bit) {
            if (debug && dbg_bit && (ExtensionInfo->getPeId() == 0xFF)) {
                is_access_permitted = true;
            }
            else if (access_um && !um_bit) {
                    is_access_permitted = false;
            }
            else {
                if (mpid == 0 && wg_bit == 0 && rg_bit == 0) {
                is_access_permitted = false;
		} else {
		    bool is_spid_enable  = false;
		    bool is_write_enable = false;
		    bool is_read_enable  = false;
		    for (unsigned char id = 0; id < mpid_num; id ++) {
		        if ((mpid >> id) & 0x1) {
		            unsigned int spid = 0;
		            spid = mINTC2G_U2_AgentController->GetSPIDValue(id);
		            if(access_spid == spid ){
		                is_spid_enable = true;
                            }
	                }	
                        if( wg_bit || is_spid_enable) {
                            is_write_enable = true;
                        }
                        if( rg_bit || is_spid_enable) {
                            is_read_enable = true;
                        }
	                if ((trans_type && !is_write_enable) || (!trans_type && !is_read_enable)) {
                            is_access_permitted = false;
                        } else {
                            is_access_permitted = true; // PASSED SPID checking
	                    break;
                        }			     	
		     }
		}					 
            }
        }
        else {
            is_access_permitted = true;
        }
    }

    if(!is_access_permitted) {
        mTypeADDR = access_addr;
        mTypeSEC = ExtensionInfo->isSecure();
        mTypeDBG = debug;
        mTypeUM = access_um;
        mTypeSPID = access_spid;
        mTypeWRITE = trans_type;
    }
    return is_access_permitted;
}//}}}

///Set start address and size for each guarded area
bool INTC2G_U2_Common_Guard::SetAreaAddr(const unsigned int ch_id, const unsigned int start_addr1, const unsigned int size1, const unsigned int start_addr2, const unsigned int size2)
{//{{{
    bool is_reWrite = false;
    unsigned int reWrite_idx = 0;
    unsigned int start_address1 = start_addr1;
    unsigned int end_address1 = start_addr1 + size1 - 1;
    unsigned int start_address2 = start_addr2;
    unsigned int end_address2 = start_addr2 + size2 - 1;
    AreaInformation temp1;
    temp1.ch_id = ch_id;
    temp1.start_addr = start_address1;
    temp1.end_addr = end_address1;
    
    AreaInformation temp2;
    temp2.ch_id = ch_id;
    temp2.start_addr = start_address2;
    temp2.end_addr = end_address2;
    bool is_valid = true;
    bool is_valid_1 = true;
    bool is_valid_2 = true;
    bool is_valid_3 = true;
    unsigned int overlap_ch_id = 0;
    // Check overlap or not
    if ((ch_id == 0) || (ch_id == 1)){
        for (unsigned int i=0; i<mAreaInfo.size(); i++) {
            if (ch_id == mAreaInfo[i].ch_id) {
                is_reWrite = true;
                reWrite_idx = i;
            }
            else if ((start_address1 > mAreaInfo[i].end_addr) || (end_address1 < mAreaInfo[i].start_addr)) {
                is_valid = true;
            }
            else {
                is_valid = false;
                break;
            }
        }    
        if (is_reWrite && is_valid) {
            mAreaInfo[reWrite_idx].start_addr = temp1.start_addr;
            mAreaInfo[reWrite_idx].end_addr   = temp1.end_addr;
        }else if (is_valid) {
            mAreaInfo.push_back(temp1);
        }else{
            std::stringstream stream;
            stream<<"Address 0x"<< std::hex <<start_address1<< std::dec <<" and size "<<size1<<" are invalid. They are overlapped with others channel id ";
            SC_REPORT_WARNING("INTC2G_U2_Common_Guard",stream.str().c_str());
        }
    }else if (ch_id >= 2 && ch_id <= 2017) {
        for (unsigned int i=0; i<mAreaInfo.size(); i++) {
            if(ch_id == mAreaInfo[i].ch_id){
                is_reWrite = true;
                reWrite_idx = i;
            }else{
                if ((start_address1 > mAreaInfo[i].end_addr) || (end_address1 < mAreaInfo[i].start_addr)) {
                    is_valid_1 = true;
                } else {
                    is_valid_1 = false;
                    break;
                }
                if ((start_address2 > mAreaInfo[i].end_addr) || (end_address2 < mAreaInfo[i].start_addr)) {
                    is_valid_2 = true;
                } else {
                    is_valid_2 = false;
                    break;
                }
            }
        }
        if ((start_address2 > end_address1) || (end_address2 < start_address1)) {
            is_valid_3 = true;
        } else {
            is_valid_3 = false;
        }
        is_valid = is_valid_1 && is_valid_2 && is_valid_3 ;
        if (is_reWrite && is_valid) {
            for (unsigned int i=0; i<mAreaInfo.size(); i++) {
                if(ch_id == mAreaInfo[i].ch_id){
                    mAreaInfo.erase(mAreaInfo.begin()+i);
                    mAreaInfo.push_back(temp1);
                    mAreaInfo.push_back(temp2);
                    break;
                }
            }    
        }else if (is_valid) {
            mAreaInfo.push_back(temp1);
            mAreaInfo.push_back(temp2);
        }else{
            std::stringstream stream;
            stream<<"Address 0x"<< std::hex <<start_address1<< std::dec <<" and size "<<size1<<" Or Address 0x"<< std::hex <<start_address2<< std::dec <<" and size "<<size2<<" are invalid. They are overlapped with others channel id ";
            SC_REPORT_WARNING("INTC2G_U2_Common_Guard",stream.str().c_str());
        }
    }
    return is_valid;
}//}}}

///Check whether the address of transaction in guarded areas
bool INTC2G_U2_Common_Guard::CheckTransAddress(const unsigned int trans_addr, unsigned int &ch_id)
{//{{{
    bool result = false;
    for (unsigned int i=0; i<mAreaInfo.size(); i++) {
        if ((trans_addr >= mAreaInfo[i].start_addr) && (trans_addr <= mAreaInfo[i].end_addr)) {
            ch_id = mAreaInfo[i].ch_id;
            std::stringstream stream;
            stream << "Access address 0x" << std::hex << trans_addr << std::dec << " is inside the protected area of channel " << ch_id;
            SC_REPORT_INFO("INTC2G_U2_Common_Guard",stream.str().c_str());
            result = true;
            break;
        }
    }
    if (!result) {
        ch_id = 0xFFFFFFFF;
        std::stringstream stream;
        stream<<"Access address 0x"<<std::hex<<trans_addr<<" does not belong to any protected address range  ";
        SC_REPORT_INFO("INTC2G_U2_CommonGuard",stream.str().c_str());
    }
    return result;
}//}}}

void INTC2G_U2_Common_Guard::EnableReset (const bool is_active)
{//{{{
    mRstState = is_active;
    if (is_active) {
        CancelEvents();
        Initialize();
    }
}//}}}

/// Cancel operation events
void INTC2G_U2_Common_Guard::CancelEvents (void)
{//{{{
    mTransErrorEvent.cancel();
}//}}}

/// Initialize internal variables
void INTC2G_U2_Common_Guard::Initialize (void)
{//{{{
    mTypeADDR    = 0;
    mTypeDBG = false;
    mTypeUM  = false;
    mTypeSEC  = false;
    mTypeSPID = 0;
    mTypeWRITE = false;
}//}}}

//Set clock operation.
void INTC2G_U2_Common_Guard::SetCLKPeriod (const sc_dt::uint64 clk_period)
{//{{{
    mCLKPeriod = clk_period;
}//}}}

unsigned int INTC2G_U2_Common_Guard::GetVAR(const std::string name)
{//{{{
    if(name=="ADDR") {
        return mTypeADDR;
    }
    else if(name=="DBG") {
        return (unsigned int)mTypeDBG;
    }
    else if(name=="UM") {
        return (unsigned int)mTypeUM;
    }
    else if(name=="SEC") {
        return (unsigned int)mTypeSEC;
    }
    else if(name=="SPID") {
        return mTypeSPID;
    }
    else if(name=="WRITE") {
        return (unsigned int)mTypeWRITE;
    }
}//}}}

// vim600: set foldmethod=marker :
