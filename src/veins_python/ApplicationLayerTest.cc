//
// Copyright (C) 2006-2020 Christoph Sommer <sommer@cms-labs.org>
//
// Documentation for these modules is at http://veins.car2x.org/
//
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#include "veins_python/ApplicationLayerTest.h"

#include "veins_python/ApplicationLayerTestMessage_m.h"

using namespace veins;
using namespace veins_python;

// ----------------------------------------------------------------------

#include <cstdlib>
#include <iostream>

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#if !(PY_MAJOR_VERSION == 3 && PY_MINOR_VERSION >= 6)
#error Python version 3.6 or compatible required
#endif

/** Define custom veins2py module **/

static PyObject* veins2py_testfun(PyObject* self, PyObject* args)
{
    if (!PyArg_ParseTuple(args, ":testfun")) return NULL;

    EV_INFO << "Veins2py testfun returning " << 42 << std::endl;

    return PyLong_FromLong(42);
}

static PyMethodDef veins2py__Methods[] = {
    {"testfun", veins2py_testfun, METH_VARARGS, "veins2py test function."},
    {NULL, NULL, 0, NULL}};

static PyModuleDef veins2py__Module = {
    PyModuleDef_HEAD_INIT,
    "veins2py", NULL, -1, veins2py__Methods,
    NULL, NULL, NULL, NULL};

static PyObject* PyInit_veins2py(void)
{
    return PyModule_Create(&veins2py__Module);
}

/** end: module **/

void test_python()
{

    // Set PYTHONHOME to baked-in value, if not provided via environment
#ifdef PYTHON_HOME
    if (!std::getenv("PYTHONHOME")) {
        Py_SetPythonHome(Py_DecodeLocale(PYTHON_HOME, NULL));
    }
#endif

    // Make custom veins2py module available
    PyImport_AppendInittab("veins2py", &PyInit_veins2py);

    // Init Python
    Py_Initialize();

    // Add cwd to search path
    PyObject* sys = PyImport_ImportModule("sys");
    PyObject* path = PyObject_GetAttrString(sys, "path");
    PyList_Append(path, PyUnicode_FromString("."));

    // Import application's py2veins module
    PyObject* pName = PyUnicode_DecodeFSDefault("py2veins");
    PyObject* pModule = PyImport_Import(pName);
    Py_DECREF(pName);
    if (pModule == NULL) {
        PyErr_Print();
        throw cRuntimeError("Failed to load module");
        return;
    }

    // Grab function from module
    PyObject* pFunc = PyObject_GetAttrString(pModule, "funtest");
    if (!(pFunc && PyCallable_Check(pFunc))) {
        if (PyErr_Occurred())
            PyErr_Print();
        throw cRuntimeError("Cannot find function");
        return;
    }

    // Build tuple of 2 arguments
    PyObject* pArgs = PyTuple_New(2);
    {
        // first argument
        int i = 0;
        PyObject* pValue = PyLong_FromLong(3);
        if (!pValue) {
            Py_DECREF(pArgs);
            Py_DECREF(pModule);
            throw cRuntimeError("Cannot convert argument");
            return;
        }
        PyTuple_SetItem(pArgs, i, pValue);
    }
    {
        // second argument
        int i = 1;
        PyObject* pValue = PyLong_FromLong(8);
        if (!pValue) {
            Py_DECREF(pArgs);
            Py_DECREF(pModule);
            throw cRuntimeError("Cannot convert argument");
            return;
        }
        PyTuple_SetItem(pArgs, i, pValue);
    }

    // Call function
    PyObject* pValue = PyObject_CallObject(pFunc, pArgs);
    Py_DECREF(pArgs);
    if (pValue == NULL) {
        Py_DECREF(pFunc);
        Py_DECREF(pModule);
        PyErr_Print();
        throw cRuntimeError("Call failed");
        return;
    }

    // Print output
    EV_INFO << "Py2veins funtest returned " << PyLong_AsLong(pValue) << std::endl;

    // Release resources for function call
    Py_DECREF(pValue);
    Py_XDECREF(pFunc);

    // Release resources for module
    Py_DECREF(pModule);

    // Teardown
    if (Py_FinalizeEx() < 0) {
        throw cRuntimeError("Error finalizing Python");
        return;
    }
}

// ----------------------------------------------------------------------

Define_Module(veins_python::ApplicationLayerTest);

void ApplicationLayerTest::initialize(int stage)
{
    DemoBaseApplLayer::initialize(stage);
    if (stage == 0) {
        sentMessage = false;
        lastDroveAt = simTime();
        currentSubscribedServiceId = -1;
    }

    test_python();
}

void ApplicationLayerTest::onWSA(DemoServiceAdvertisment* wsa)
{
    if (currentSubscribedServiceId == -1) {
        mac->changeServiceChannel(static_cast<Channel>(wsa->getTargetChannel()));
        currentSubscribedServiceId = wsa->getPsid();
        if (currentOfferedServiceId != wsa->getPsid()) {
            stopService();
            startService(static_cast<Channel>(wsa->getTargetChannel()), wsa->getPsid(), "Mirrored Traffic Service");
        }
    }
}

void ApplicationLayerTest::onWSM(BaseFrame1609_4* frame)
{
    ApplicationLayerTestMessage* wsm = check_and_cast<ApplicationLayerTestMessage*>(frame);

    findHost()->getDisplayString().setTagArg("i", 1, "green");

    if (mobility->getRoadId()[0] != ':') traciVehicle->changeRoute(wsm->getDemoData(), 9999);
    if (!sentMessage) {
        sentMessage = true;
        // repeat the received traffic update once in 2 seconds plus some random delay
        wsm->setSenderAddress(myId);
        wsm->setSerial(3);
        scheduleAt(simTime() + 2 + uniform(0.01, 0.2), wsm->dup());
    }
}

void ApplicationLayerTest::handleSelfMsg(cMessage* msg)
{
    if (ApplicationLayerTestMessage* wsm = dynamic_cast<ApplicationLayerTestMessage*>(msg)) {
        // send this message on the service channel until the counter is 3 or higher.
        // this code only runs when channel switching is enabled
        sendDown(wsm->dup());
        wsm->setSerial(wsm->getSerial() + 1);
        if (wsm->getSerial() >= 3) {
            // stop service advertisements
            stopService();
            delete (wsm);
        }
        else {
            scheduleAt(simTime() + 1, wsm);
        }
    }
    else {
        DemoBaseApplLayer::handleSelfMsg(msg);
    }
}

void ApplicationLayerTest::handlePositionUpdate(cObject* obj)
{
    DemoBaseApplLayer::handlePositionUpdate(obj);

    // stopped for for at least 10s?
    if (mobility->getSpeed() < 1) {
        if (simTime() - lastDroveAt >= 10 && sentMessage == false) {
            findHost()->getDisplayString().setTagArg("i", 1, "red");
            sentMessage = true;

            ApplicationLayerTestMessage* wsm = new ApplicationLayerTestMessage();
            populateWSM(wsm);
            wsm->setDemoData(mobility->getRoadId().c_str());

            // host is standing still due to crash
            if (dataOnSch) {
                startService(Channel::sch2, 42, "Traffic Information Service");
                // started service and server advertising, schedule message to self to send later
                scheduleAt(computeAsynchronousSendingTime(1, ChannelType::service), wsm);
            }
            else {
                // send right away on CCH, because channel switching is disabled
                sendDown(wsm);
            }
        }
    }
    else {
        lastDroveAt = simTime();
    }
}
