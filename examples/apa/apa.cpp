/**
  Example of usage of the Arbitrary Processor Affinity (APA) kernel/scheduler

  @author Davide Kirchner
*/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <apamrtkernel.hpp>
#include <apasched.hpp>
#include <texttrace.hpp>
#include <json_trace.hpp>
#include <rttask.hpp>
#include <instr.hpp>
#include <SchedulerFactory.hpp>

using namespace MetaSim;
using namespace RTSim;

int main()
{
    // Compile the library with debugging enabled, or this will have no effect
    SIMUL.dbg.enable("All");
    SIMUL.dbg.setStream("debug.txt");

    TextTrace ttrace("trace.txt");
    JSONTrace jsontrace("trace.json");

    cout << "Creating Scheduler and kernel" << endl;
    APAScheduler sched(new EDFSchedulerFactory());
    APAMRTKernel kern(&sched, 3, "APAMRTKernel");

    cout << "Creating Tasks" << endl;
    // Task definition inspired from fig. 7.19 (the SRP example) in
    // _Hard Real-Time Computing Systems_ by G. Buttazzo

    #define mktask(t, p) PeriodicTask t(p, p, 0, #t);

    PeriodicTask t1(12, 12, 0, "Task1");
    t1.insertCode("fixed(4);");
    //t1.setAbort(false);

    PeriodicTask t2(17, 17, 0, "Task2");
    t2.insertCode("fixed(4);");
    //t2.setAbort(false);

    PeriodicTask t3(24, 24, 0, "Task3");
    t3.insertCode("fixed(6);");
    //t3.setAbort(false);

    cout << "Setting up traces" << endl;
    // new way
    ttrace.attachToTask(&t1);
    ttrace.attachToTask(&t2);
    ttrace.attachToTask(&t3);

    jsontrace.attachToTask(&t1);
    jsontrace.attachToTask(&t2);
    jsontrace.attachToTask(&t3);

    cout << "Adding tasks to kernel" << endl;
    // TODO: compute preemption level directly from the periods
    kern.addTask(t1, "0xff");
    kern.addTask(t2, "0xff");
    kern.addTask(t3, "0xff");

    cout << "Computing ceilings" << endl;

    cout << "Ready to run!" << endl;
    SIMUL.run(75);
}
