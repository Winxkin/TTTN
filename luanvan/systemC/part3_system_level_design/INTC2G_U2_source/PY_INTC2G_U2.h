// ---------------------------------------------------------------------
// $Id: PY_INTC2G_U2.h,v 1.4 2018/12/03 10:08:20 huepham Exp $
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
#ifndef __PY_INTC2G_U2_H__
#define __PY_INTC2G_U2_H__
#include <Python.h>

namespace PY_INTC2G_U2
{
    static PyObject* AssertReset_Py (PyObject* self, PyObject* args);
    static PyObject* SetCLKFreq_Py (PyObject* self, PyObject* args);
    static PyObject* GetCLKFreq_Py (PyObject* self, PyObject* args);
    static PyObject* ForceRegister_Py (PyObject* self, PyObject* args);
    static PyObject* ReleaseRegister_Py (PyObject* self, PyObject* args);
    static PyObject* WriteRegister_Py (PyObject* self, PyObject* args);
    static PyObject* ReadRegister_Py (PyObject* self, PyObject* args);
    static PyObject* ListRegister_Py (PyObject* self, PyObject* args);
    static PyObject* Help_Py (PyObject* self, PyObject* args);
    static PyObject* SetMessageLevel_Py (PyObject* self, PyObject* args);
    static PyObject* DumpRegisterRW_Py (PyObject* self, PyObject* args);
    static PyObject* DumpInterrupt_Py (PyObject* self, PyObject* args);
    static PyObject* P_LOCK_SUPPORT_Py (PyObject* self, PyObject* args);
    static PyObject* SetAreaAddress_Py (PyObject* self, PyObject* args);
    static PyObject* P_MPID_NO_Py (PyObject* self, PyObject* args); //ducla change
    static PyObject* P_RESET_VALUES0_n_Py (PyObject* self, PyObject* args);
    static PyObject* EnableRegisterMessage_Py (PyObject* self, PyObject* args);

    void SetPyExtCmd(void);
}
#endif
