// ---------------------------------------------------------------------
// $Id: PY_INTC2G_U2.cpp,v 1.4 2018/12/03 10:08:20 huepham Exp $
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

#include "PY_INTC2G_U2.h"
#include "INTC2G_U2.h"
namespace PY_INTC2G_U2
{
    static PyMethodDef mShApiDef[] = {
        {"INTC2G_U2_AssertReset", AssertReset_Py, METH_VARARGS, ""},
        {"INTC2G_U2_SetCLKFreq", SetCLKFreq_Py, METH_VARARGS, ""},
        {"INTC2G_U2_GetCLKFreq", GetCLKFreq_Py, METH_VARARGS, ""},
        {"INTC2G_U2_ForceRegister", ForceRegister_Py, METH_VARARGS, ""},
        {"INTC2G_U2_ReleaseRegister", ReleaseRegister_Py, METH_VARARGS, ""},
        {"INTC2G_U2_WriteRegister", WriteRegister_Py, METH_VARARGS, ""},
        {"INTC2G_U2_ReadRegister", ReadRegister_Py, METH_VARARGS, ""},
        {"INTC2G_U2_ListRegister", ListRegister_Py, METH_VARARGS, ""},
        {"INTC2G_U2_Help", Help_Py, METH_VARARGS, ""},
        {"INTC2G_U2_MessageLevel", SetMessageLevel_Py, METH_VARARGS, ""},
        {"INTC2G_U2_DumpRegisterRW", DumpRegisterRW_Py, METH_VARARGS, ""},
        {"INTC2G_U2_DumpInterrupt", DumpInterrupt_Py, METH_VARARGS, ""},
        {"INTC2G_U2_P_LOCK_SUPPORT", P_LOCK_SUPPORT_Py, METH_VARARGS, ""},
        {"INTC2G_U2_SetAreaAddress", SetAreaAddress_Py, METH_VARARGS, ""},
        {"INTC2G_U2_P_MPID_NO", P_MPID_NO_Py, METH_VARARGS, ""},
        {"INTC2G_U2_P_RESET_VALUES0_n", P_RESET_VALUES0_n_Py, METH_VARARGS, ""},
        {"INTC2G_U2_EnableRegisterMessage", EnableRegisterMessage_Py, METH_VARARGS, ""},
        {NULL, NULL, 0, NULL}
    };

    PyObject* AssertReset_Py(PyObject* self, PyObject* args)
    {//{{{
        sc_assert((self != NULL) && (args != NULL));
        char* token;
        char* reset_name;
        double start_time;
        double period;
        INTC2G_U2* obj;
        if (PyArg_ParseTuple(args, "ssdd", &token, &reset_name, &start_time, &period) == true) {
            sc_assert((token != NULL)&&(reset_name != NULL));
            if ((obj = dynamic_cast<INTC2G_U2*>(sc_find_object(token))) != NULL) {
                obj->AssertReset(reset_name, start_time, period);
            } else {
                SC_REPORT_ERROR("PY_INTC2G_U2","Can not find the object of INTC2G_U2 class");
            }
        } else {
            SC_REPORT_WARNING("PY_INTC2G_U2","The arguments of INTC2G_U2_AssertReset are wrong");
        }
        return Py_BuildValue("");
    }//}}}

    PyObject* SetCLKFreq_Py(PyObject* self, PyObject* args)
    {//{{{
        sc_assert((self != NULL) && (args != NULL));
        char* token;
        char* clock_name;
        unsigned long long freq;
        char* unit;
        INTC2G_U2* obj;
        if (PyArg_ParseTuple(args, "ssKs", &token, &clock_name, &freq, &unit) == true) {
            sc_assert((token != NULL)&&(clock_name != NULL)&&(unit != NULL));
            if ((obj = dynamic_cast<INTC2G_U2*>(sc_find_object(token))) != NULL) {
                obj->SetCLKFreq(clock_name, freq, unit);
            } else {
                SC_REPORT_ERROR("PY_INTC2G_U2","Can not find the object of INTC2G_U2 class");
            }
        } else {
            SC_REPORT_WARNING("PY_INTC2G_U2","The arguments of INTC2G_U2_SetCLKFreq are wrong");
        }
        return Py_BuildValue("");
    }//}}}

    PyObject* GetCLKFreq_Py(PyObject* self, PyObject* args)
    {//{{{
        sc_assert((self != NULL) && (args != NULL));
        char* token;
        char* clock_name;
        INTC2G_U2* obj;
        if (PyArg_ParseTuple(args, "ss", &token, &clock_name) == true) {
            sc_assert((token != NULL)&&(clock_name != NULL));
            if ((obj = dynamic_cast<INTC2G_U2*>(sc_find_object(token))) != NULL) {
                obj->GetCLKFreq(clock_name);
            } else {
                SC_REPORT_ERROR("PY_INTC2G_U2","Can not find the object of INTC2G_U2 class");
            }
        } else {
            SC_REPORT_WARNING("PY_INTC2G_U2","The arguments of INTC2G_U2_GetCLKFreq are wrong");
        }
        return Py_BuildValue("");
    }//}}}

    PyObject* ListRegister_Py(PyObject* self, PyObject* args)
    {//{{{
        sc_assert((self != NULL) && (args != NULL));
        char* token;
        INTC2G_U2* obj;
        if (PyArg_ParseTuple(args, "s", &token) == true) {
            sc_assert((token != NULL));
            if ((obj = dynamic_cast<INTC2G_U2*>(sc_find_object(token))) != NULL) {
                obj->ListRegister();
            } else {
                SC_REPORT_ERROR("PY_INTC2G_U2","Can not find the object of INTC2G_U2 class");
            }
        } else {
            SC_REPORT_WARNING("PY_INTC2G_U2","The INTC2G_U2_ListRegister has not any arguments");
        }
        return Py_BuildValue("");
    }//}}}

    PyObject* Help_Py(PyObject* self, PyObject* args)
    {//{{{
        sc_assert((self != NULL) && (args != NULL));
        char* token;
        char* kind;
        INTC2G_U2* obj;
        if (PyArg_ParseTuple(args, "ss", &token, &kind) == true) {
            sc_assert((token != NULL) && (kind != NULL));
            if ((obj = dynamic_cast<INTC2G_U2*>(sc_find_object(token))) != NULL) {
                obj->Help(kind);
            } else {
                SC_REPORT_ERROR("PY_INTC2G_U2","Can not find the object of INTC2G_U2 class");
            }
        } else {
            SC_REPORT_WARNING("PY_INTC2G_U2","The arguments of INTC2G_U2_Help are wrong");
        }
        return Py_BuildValue("");
    }//}}}

    PyObject* SetMessageLevel_Py(PyObject* self, PyObject* args)
    {//{{{
        sc_assert((self != NULL) && (args != NULL));
        char* token;
        char* level;
        INTC2G_U2* obj;
        if (PyArg_ParseTuple(args, "ss", &token, &level) == true) {
            sc_assert((token != NULL) && (level != NULL));
            if ((obj = dynamic_cast<INTC2G_U2*>(sc_find_object(token))) != NULL) {
                obj->SetMessageLevel(level);
            } else {
                SC_REPORT_ERROR("PY_INTC2G_U2","Can not find the object of INTC2G_U2 class");
            }
        } else {
            SC_REPORT_WARNING("PY_INTC2G_U2","The arguments of INTC2G_U2_MessageLevel are wrong");
        }
        return Py_BuildValue("");
    }//}}}

    PyObject* DumpRegisterRW_Py(PyObject* self, PyObject* args)
    {//{{{
        sc_assert((self != NULL) && (args != NULL));
        char* token;
        char* is_enable;
        INTC2G_U2* obj;
        if (PyArg_ParseTuple(args, "ss", &token, &is_enable) == true) {
            sc_assert((token != NULL) && (is_enable != NULL));
            if ((obj = dynamic_cast<INTC2G_U2*>(sc_find_object(token))) != NULL) {
                obj->DumpRegisterRW(is_enable);
            } else {
                SC_REPORT_ERROR("PY_INTC2G_U2","Can not find the object of INTC2G_U2 class");
            }
        } else {
            SC_REPORT_WARNING("PY_INTC2G_U2","The arguments of INTC2G_U2_DumpRegisterRW are wrong");
        }
        return Py_BuildValue("");
    }//}}}
    
    PyObject* EnableRegisterMessage_Py(PyObject* self, PyObject* args)
    {//{{{
        sc_assert((self != NULL) && (args != NULL));
        char* token;
        char* is_enable;
        INTC2G_U2* obj;
        if ((bool)PyArg_ParseTuple(args, "ss", &token, &is_enable) == true) {
            sc_assert((token != NULL) && (is_enable != NULL));
            if ((obj = dynamic_cast<INTC2G_U2*>(sc_find_object(token))) != NULL) {
                obj->EnableRegisterMessage(is_enable);
            } else {
                SC_REPORT_ERROR("PY_INTC2G_U2","Can not find the object of INTC2G_U2 class");
            }
        } else {
            SC_REPORT_WARNING("PY_INTC2G_U2","The arguments of INTC2G_U2_EnableRegisterMessage are wrong");
        }
        return Py_BuildValue("");
     }//}}}

    PyObject* DumpInterrupt_Py(PyObject* self, PyObject* args)
    {//{{{
        sc_assert((self != NULL) && (args != NULL));
        char* token;
        char* is_enable;
        INTC2G_U2* obj;
        if (PyArg_ParseTuple(args, "ss", &token, &is_enable) == true) {
            sc_assert((token != NULL) && (is_enable != NULL));
            if ((obj = dynamic_cast<INTC2G_U2*>(sc_find_object(token))) != NULL) {
                obj->DumpInterrupt(is_enable);
            } else {
                SC_REPORT_ERROR("PY_INTC2G_U2","Can not find the object of INTC2G_U2 class");
            }
        } else {
            SC_REPORT_WARNING("PY_INTC2G_U2","The arguments of INTC2G_U2_DumpInterrupt are wrong");
        }
        return Py_BuildValue("");
    }//}}}
    
    PyObject* P_LOCK_SUPPORT_Py(PyObject* self, PyObject* args)
    {//{{{
        sc_assert((self != NULL) && (args != NULL));
        char* token;
        unsigned int value;
        INTC2G_U2* obj;
        if (PyArg_ParseTuple(args, "sI", &token, &value) == true) {
            sc_assert(token != NULL);
            if ((obj = dynamic_cast<INTC2G_U2*>(sc_find_object(token))) != NULL) {
                obj->LOCK_SUPPORT(value);
            } else {
                SC_REPORT_ERROR("PY_INTC2G_U2","Can not find the object of INTC2G_U2 class");
            }
        } else {
            SC_REPORT_WARNING("PY_INTC2G_U2","The arguments of INTC2G_U2_P_LOCK_SUPPORT are wrong");
        }
        return Py_BuildValue("");
    }//}}}

    PyObject* ForceRegister_Py(PyObject* self, PyObject* args)
    {//{{{
        sc_assert((self != NULL) && (args != NULL));
        char* token;
        char* reg_name;
        unsigned int reg_value;
        INTC2G_U2* obj;
        if (PyArg_ParseTuple(args, "ssI", &token, &reg_name, &reg_value) == true) {
            sc_assert((token != NULL) && (reg_name != NULL));
            if ((obj = dynamic_cast<INTC2G_U2*>(sc_find_object(token))) != NULL) {
                obj->ForceRegister(reg_name, reg_value);
            } else {
                SC_REPORT_ERROR("PY_INTC2G_U2","Can not find the object of INTC2G_U2 class");
            }
        } else {
            SC_REPORT_WARNING("PY_INTC2G_U2","The arguments of INTC2G_U2_ForceRegister are wrong");
        }
        return Py_BuildValue("");
    }//}}}

    PyObject* ReleaseRegister_Py(PyObject* self, PyObject* args)
    {//{{{
        sc_assert((self != NULL) && (args != NULL));
        char* token;
        char* reg_name;
        INTC2G_U2* obj;
        if (PyArg_ParseTuple(args, "ss", &token, &reg_name) == true) {
            sc_assert((token != NULL) && (reg_name != NULL));
            if ((obj = dynamic_cast<INTC2G_U2*>(sc_find_object(token))) != NULL) {
                obj->ReleaseRegister(reg_name);
            } else {
                SC_REPORT_ERROR("PY_INTC2G_U2","Can not find the object of INTC2G_U2 class");
            }
        } else {
            SC_REPORT_WARNING("PY_INTC2G_U2","The arguments of INTC2G_U2_ReleaseRegister are wrong");
        }
        return Py_BuildValue("");
    }//}}}

    PyObject* WriteRegister_Py(PyObject* self, PyObject* args)
    {//{{{
        sc_assert((self != NULL) && (args != NULL));
        char* token;
        char* reg_name;
        unsigned int reg_value;
        INTC2G_U2* obj;
        if (PyArg_ParseTuple(args, "ssI", &token, &reg_name, &reg_value) == true) {
            sc_assert((token != NULL) && (reg_name != NULL));
            if ((obj = dynamic_cast<INTC2G_U2*>(sc_find_object(token))) != NULL) {
                obj->WriteRegister(reg_name, reg_value);
            } else {
                SC_REPORT_ERROR("PY_INTC2G_U2","Can not find the object of INTC2G_U2 class");
            }
        } else {
            SC_REPORT_WARNING("PY_INTC2G_U2","The arguments of INTC2G_U2_WriteRegister are wrong");
        }
        return Py_BuildValue("");
    }//}}}

    PyObject* ReadRegister_Py(PyObject* self, PyObject* args)
    {//{{{
        sc_assert((self != NULL) && (args != NULL));
        char* token;
        char* reg_name;
        INTC2G_U2* obj;
        if (PyArg_ParseTuple(args, "ss", &token, &reg_name) == true) {
            sc_assert((token != NULL) && (reg_name != NULL));
            if ((obj = dynamic_cast<INTC2G_U2*>(sc_find_object(token))) != NULL) {
                obj->ReadRegister(reg_name);
            } else {
                SC_REPORT_ERROR("PY_INTC2G_U2","Can not find the object of INTC2G_U2 class");
            }
        } else {
            SC_REPORT_WARNING("PY_INTC2G_U2","The arguments of INTC2G_U2_ReadRegister are wrong");
        }
        return Py_BuildValue("");
    }//}}}

    PyObject* SetAreaAddress_Py(PyObject* self, PyObject* args)
    {//{{{
        sc_assert(args != NULL);
        char* token;
        unsigned int ch_id;
        unsigned int start_addr1;
        unsigned int size1;
        unsigned int start_addr2;
        unsigned int size2;
        INTC2G_U2* obj;
        if (PyArg_ParseTuple(args, "sIIIII", &token, &ch_id, &start_addr1, &size1, &start_addr2, &size2) == true) {
            sc_assert((token != NULL) && (ch_id != NULL));
            if ((obj = dynamic_cast<INTC2G_U2*>(sc_find_object(token))) != NULL) {
                obj->SetAreaAddress(ch_id, start_addr1, size1, start_addr2, size2);
            } else {
                SC_REPORT_ERROR("PY_INTC2G_U2","Can not find the object of INTC2G_U2 class");
            }
        } else {
            SC_REPORT_ERROR("PY_INTC2G_U2","The arguments of INTC2G_U2_SetAreaAddress are wrong");
        }
        return Py_BuildValue("");
    }//}}}
    
    PyObject* P_MPID_NO_Py(PyObject* self, PyObject* args)
    {//{{{
        sc_assert(args != NULL);
        char* token;
        unsigned int mpid_no;
        INTC2G_U2* obj;
        if (PyArg_ParseTuple(args, "sI", &token, &mpid_no) == true) {
            sc_assert((token != NULL) && (mpid_no != NULL));
            if ((obj = dynamic_cast<INTC2G_U2*>(sc_find_object(token))) != NULL) {
                obj->SetMpidNumber(mpid_no);
            } else {
                SC_REPORT_ERROR("PY_INTC2G_U2","Can not find the object of INTC2G_U2 class");
            }
        } else {
            SC_REPORT_ERROR("PY_INTC2G_U2","The arguments of INTC2G_U2_P_MPID_NO are wrong");
        }
        return Py_BuildValue("");
    }//}}}
 // ducla add
    PyObject* P_RESET_VALUES0_n_Py (PyObject* self, PyObject* args)
    {//{{{
        sc_assert(args != NULL);
        char* token;
        unsigned int mpid_no;
        unsigned int val;
        INTC2G_U2* obj;
        if (PyArg_ParseTuple(args, "sII", &token, &mpid_no, &val) == true) {
            sc_assert((token != NULL) && (mpid_no != NULL));
            if ((obj = dynamic_cast<INTC2G_U2*>(sc_find_object(token))) != NULL) {
                obj->SetResetValues0(mpid_no,val);
            } else {
                SC_REPORT_ERROR("PY_INTC2G_U2","Can not find the object of INTC2G_U2 class");
            }
        } else {
            SC_REPORT_ERROR("PY_INTC2G_U2","The arguments of RESET_VALUES0_n are wrong");
        }
        return Py_BuildValue("");
    }//}}}

    void SetPyExtCmd(void)
    {//{{{
        Py_InitModule("SCHEAP", mShApiDef);
    }//}}}
}
// vim600: set foldmethod=marker :
