/**
  Example of usage of the SRP module

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
#include <kernel.hpp>
#include <srpsched.hpp>
#include <srpresman.hpp>
#include <texttrace.hpp>
#include <json_trace.hpp>
#include <rttask.hpp>
#include <instr.hpp>

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
    EDF_SRPScheduler sched;
    RTKernel kern(&sched);
    SRPResManager resman;

    kern.setResManager(&resman);
    sched.setResManager(&resman);

    cout << "Creating Tasks" << endl;
    // Task definition inspired from fig. 7.19 (the SRP example) in
    // _Hard Real-Time Computing Systems_ by G. Buttazzo

    resman.addResource("R1");
    resman.addResource("R2");
    resman.addResource("R3");

    PeriodicTask t1(12, 12, 4, "Task1");
    t1.insertCode("fixed(1);");
    t1.insertCode("wait(R3);");
      t1.insertCode("fixed(1);");
      t1.insertCode("wait(R1);fixed(1);signal(R1);");
      t1.insertCode("fixed(1);");
    t1.insertCode("signal(R3);");
    t1.insertCode("fixed(1);");
    //t1.setAbort(false);

    PeriodicTask t2(17, 17, 2, "Task2");
    t2.insertCode("fixed(1);");
    t2.insertCode("wait(R3);");
      t2.insertCode("fixed(1);");
        t2.insertCode("wait(R2);fixed(2);signal(R2);");
      t2.insertCode("fixed(1);");
    t2.insertCode("signal(R3);");
    t2.insertCode("fixed(1);");
    t2.insertCode("wait(R1);fixed(1);signal(R1);");
    //t2.setAbort(false);

    PeriodicTask t3(24, 24, 0, "Task3");
    t3.insertCode("fixed(1);");
    t3.insertCode("wait(R2);");
      t3.insertCode("fixed(2);");
      t3.insertCode("wait(R1);fixed(2);signal(R1);");
      t3.insertCode("fixed(1);");
    t3.insertCode("signal(R2);");
    t3.insertCode("fixed(1);");
    t3.insertCode("wait(R3);fixed(2);signal(R3);");
    t3.insertCode("fixed(1);");
    //t3.setAbort(false);

    cout << "Setting up traces" << endl;
    // new way
    ttrace.attachToTask(&t1);
    ttrace.attachToTask(&t2);
    ttrace.attachToTask(&t3);

    jsontrace.attachToTask(&t1);
    jsontrace.attachToTask(&t2);
    jsontrace.attachToTask(&t3);
    jsontrace.attachToSRPResMan(&resman);

    cout << "Adding tasks to kernel" << endl;
    // TODO: compute preemption level directly from the periods
    kern.addTask(t1, "3");
    kern.addTask(t2, "2");
    kern.addTask(t3, "1");

    cout << "Computing ceilings" << endl;
    resman.ceilingsFromTask(&t1);
    resman.ceilingsFromTask(&t2);
    resman.ceilingsFromTask(&t3);

    cout << "Ready to run!" << endl;
    SIMUL.run(75);
}
