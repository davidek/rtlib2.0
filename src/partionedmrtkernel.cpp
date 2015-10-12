/***************************************************************************
 *   begin                : Sat Aug 15 17:28:58 CEST 2015
 *   copyright            : (C) 2015 by Daniel Casini
 *   email                : daniel.casini@gmail.com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <algorithm>

#include <simul.hpp>

#include <cpu.hpp>
#include <partionedmrtkernel.hpp>
#include <mrtkernel.hpp>
#include <resmanager.hpp>
#include <scheduler.hpp>
#include <task.hpp>
#include <edfsched.hpp>
#include "TaskAllocation.hpp"
#include <periodicservervm.hpp>
#include <virtualmachine.hpp>


namespace RTSim {

    using namespace std;
    using namespace MetaSim;

    template<class IT>
    void clean_mapcontainer_content(IT b, IT e)
    {
        for (IT i=b; i!=e; i++){
            delete i->second;
        }
    }
    template<class IT>
    void clean_mapcontainer_key(IT b, IT e)
    {
        for (IT i=b; i!=e; i++){
            delete i->first;
        }
    }

	CPU *PartionedMRTKernel::getProcessor(const AbsRTTask* t) const
	{
        /* at instead [] because the [] will create the element
        in the map if no correspondence exists for the given key:
        this is not compatible with a const function */
		CPU *cp = _taskCPUMap.at(t); 
		return cp;
	}


    PartionedMRTKernel::PartionedMRTKernel(const string& name) : MRTKernel(name), _nCPU(0),
        _schedFactory(0), _taskAllocator(0){
        _CPUFactory = 0;
    }

	PartionedMRTKernel::PartionedMRTKernel(int n, const string& name) : MRTKernel(name), _nCPU(n)
    { 
        _taskAllocator = new FirstFitDecreasingTaskAllocation();
        internalConstructor(n);
    }

    PartionedMRTKernel::PartionedMRTKernel(int n, const string& name, AbsSchedulerFactory *schedFactory, AbsTaskAllocation *taskAllocationStrategy) : MRTKernel(name), _nCPU(n), _schedFactory(schedFactory)
    { 
        _taskAllocator = taskAllocationStrategy;
        _CPUFactory = new uniformCPUFactory;
        internalConstructor(n);
    }

	PartionedMRTKernel::PartionedMRTKernel(int n, const string& name, absCPUFactory *cpuFactory, AbsSchedulerFactory *schedFactory, AbsTaskAllocation *taskAllocationStrategy) : MRTKernel(name, cpuFactory), _nCPU(n), _schedFactory(schedFactory)
    { 
        _taskAllocator = taskAllocationStrategy;
        internalConstructor(n);
    }

    void PartionedMRTKernel::createCPUFromFactory(int index){

		CPU *c = _CPUFactory->createCPU();
        c->setIndex(index);

		Scheduler *sched = _schedFactory->createScheduler();

		_cpuSchedulerMap[c] = sched;
		_cpuSchedulerMap[c]->setKernel(this);
        _m_currExe[c] = NULL;
        _isContextSwitching[c] = false;
        _beginEvt[c] = new BeginDispatchMultiEvt(*this, *c);
        _endEvt[c] = new EndDispatchMultiEvt(*this, *c);
	}

	void PartionedMRTKernel::internalConstructor(int n)
    {
        for(int i=0; i<n; i++)  {
            createCPUFromFactory(i);
        }
    }

    PartionedMRTKernel::~PartionedMRTKernel()
    {
    }

    void PartionedMRTKernel::addCPU(CPU *c, Scheduler *s) 
    { 
		_nCPU++;
        _cpuSchedulerMap[c] = s;
		_cpuSchedulerMap[c]->setKernel(this);

		DBGENTER(_KERNEL_DBG_LEV);

        _m_currExe[c] = NULL; 
        _isContextSwitching[c] = false;
        _beginEvt[c] = new BeginDispatchMultiEvt(*this, *c);
        _endEvt[c] = new EndDispatchMultiEvt(*this, *c);
    }

    void PartionedMRTKernel::addCPU(CPU *c)
    {
        if(!_schedFactory)
            throw UndefinedSchedulerFactoryException();

        _nCPU++;
        _cpuSchedulerMap[c] = _schedFactory->createScheduler();
        _cpuSchedulerMap[c]->setKernel(this);


        _m_currExe[c] = NULL;
        _isContextSwitching[c] = false;
        _beginEvt[c] = new BeginDispatchMultiEvt(*this, *c);
        _endEvt[c] = new EndDispatchMultiEvt(*this, *c);
    }

    void PartionedMRTKernel::addCPU()
    {
        if(!_schedFactory)
            throw UndefinedSchedulerFactoryException();
        if(!_CPUFactory)
            throw UndefinedCPUFactoryException();

        CPU *c = _CPUFactory->createCPU();
        _nCPU++;
        _cpuSchedulerMap[c] = _schedFactory->createScheduler();
        _cpuSchedulerMap[c]->setKernel(this);

        _m_currExe[c] = NULL;
        _isContextSwitching[c] = false;
        _beginEvt[c] = new BeginDispatchMultiEvt(*this, *c);
        _endEvt[c] = new EndDispatchMultiEvt(*this, *c);
    }
    
    void PartionedMRTKernel::addTask(AbsRTTask &t, const string &param)
    {
         t.setKernel(this);
        _handled.push_back(&t); 
		_taskParam[&t] = param;
        _m_oldExe[&t] = NULL;
        _m_dispatched[&t] = NULL;
    }
	
	void PartionedMRTKernel::addTask(AbsRTTask &t, const string &param, CPU *c)
    {
        if(!_cpuSchedulerMap.count(c))
            throw UndefinedCPUException("The given CPU does not belong to the kernel");

         t.setKernel(this);
        _handled.push_back(&t); 
		_taskSchedulerMap[&t] = _cpuSchedulerMap[c];
		_taskCPUMap[&t] = c;
		_cpuSchedulerMap[c]->addTask(&t, param);
        _m_oldExe[&t] = NULL;
        _m_dispatched[&t] = NULL;
		
    }

    void PartionedMRTKernel::suspend(AbsRTTask *task)
    {
        DBGENTER(_MRTKERNEL_DBG_LEV);

        _taskSchedulerMap[task]->extract(task);
        CPU *p = getProcessor(task);
        if (p != NULL){
            task->deschedule();

            _m_currExe[p] = NULL;
            _m_oldExe[task] = p;
            _m_dispatched[task] = NULL;
        }
    }

    void PartionedMRTKernel::onArrival(AbsRTTask *t)
    {
        _taskSchedulerMap[t]->insert(t);
        
        MRTKernel::dispatch(_taskCPUMap[t]);
    }
	
	void PartionedMRTKernel::onBeginDispatchMulti(BeginDispatchMultiEvt* e)
    {
        // if necessary, deschedule the task.
        CPU * p = e->getCPU();
        AbsRTTask *dt  = _m_currExe[p];
        AbsRTTask *st  = NULL;

        if ( dt != NULL ) {
            _m_oldExe[dt] = p;
            _m_currExe[p] = NULL;
            _m_dispatched[dt] = NULL;
            dt->deschedule();
        }

        // select the first non dispatched task in the queue
        int i = 0;
        while ((st = _cpuSchedulerMap[p]->getTaskN(i)) != NULL) 
            if (_m_dispatched[st] == NULL) break;
            else i++;

        if (st == NULL) {
            DBGPRINT("Nothing to schedule, finishing");
        }

        DBGPRINT_4("Scheduling task ", taskname(st), " on cpu ", p);
        
        if (st) _m_dispatched[st] = p;

        _endEvt[p]->setTask(st);
        _isContextSwitching[p] = true;

        Tick overhead (_contextSwitchDelay);
        _endEvt[p]->post(SIMUL.getTime() + overhead);        
    }

    void PartionedMRTKernel::onEndDispatchMulti(EndDispatchMultiEvt* e)
    {
        // performs the "real" context switch
        DBGENTER(_KERNEL_DBG_LEV);


        AbsRTTask *st = e->getTask();
        CPU *p = e->getCPU();

        _m_currExe[p] = st;

        DBGPRINT_2("CPU: ", p);
        DBGPRINT_2("Task: ", taskname(st));
        
        // st could be null (because of an idling processor)
        if (st) st->schedule();

		_isContextSwitching[p] = false;
        _cpuSchedulerMap[p]->notify(st);
    }
	

    void PartionedMRTKernel::onEnd(AbsRTTask *task)
    {
        DBGENTER(_KERNEL_DBG_LEV);

        CPU *p = getProcessor(task);

        if (p == NULL) 
            throw RTKernelExc("Received a onEnd of a non executing task"); 

        _taskSchedulerMap[task]->extract(task);
        _m_oldExe[task] = p;
        _m_currExe[p] = NULL;
        _m_dispatched[task] = NULL;

        MRTKernel::dispatch(p);
    }
	
	 void PartionedMRTKernel::newRun()
    {
        for (ITCPU i=_m_currExe.begin(); i!=_m_currExe.end(); i++) {
            if (i->second != NULL)
                _taskSchedulerMap[i->second]->extract(i->second);
            i->second=NULL;
        }

        map<const AbsRTTask *, CPU *>::iterator j = _m_dispatched.begin();
        for ( ; j != _m_dispatched.end(); ++j )
            j->second = NULL;

        _m_dispatched.clear();

        j = _m_oldExe.begin();
        for ( ; j != _m_oldExe.end(); ++j )
            j->second = NULL;
        _m_oldExe.clear();
    }

    void PartionedMRTKernel::endRun()
    { 
        Scheduler *sched;
        for (ITCPU i=_m_currExe.begin(); i!=_m_currExe.end(); i++) {
            if (i->second != NULL){
                sched = _cpuSchedulerMap[i->first];
                sched->extract(i->second);
            }
            i->second=NULL;
        }
    }
	

	void PartionedMRTKernel::allocateTask()
	{
        if(!_taskAllocator)
            throw UndefinedAllocationStrategyException();

        _taskCPUMap.clear();
		_taskAllocator->allocate(const_cast<PartionedMRTKernel *>(this));
	}

    void PartionedMRTKernel::addVM(PeriodicServerVM &VM, CPU* cpu)
    {
        if(!_cpuSchedulerMap.count(cpu))
            throw UndefinedCPUException("The given CPU does not belong to the kernel");

        Scheduler *sched = _cpuSchedulerMap[cpu];

        if(dynamic_cast<EDFScheduler*>(sched)){
            VM.setImplementation(new CBServer(VM.getBudget(), VM.getPeriod(), VM.getPeriod(),
                                              'hard', VM.getName()));
        }
        else if(dynamic_cast<RMScheduler*>(sched))
        {
            VM.setImplementation(new SporadicServer(VM.getBudget(), VM.getPeriod(), VM.getName()));
        }
        else
            throw UndefinedVMImplementationException("Does not exist a VM implementation for the given scheduler");

        Server *implementation = VM.getImplementation();
        _taskCPUMap[implementation] = cpu;
        addTask(*implementation, "", cpu);

    }

    void PartionedMRTKernel::addVM(PeriodicServerVM &VM)
    {
        if(!_schedFactory)
            throw UndefinedSchedulerFactoryException();

        if(dynamic_cast<EDFSchedulerFactory *>(_schedFactory))
        {
            VM.setImplementation(new CBServer(VM.getBudget(), VM.getPeriod(), VM.getPeriod(),
                                              'hard', VM.getName()));
        }
        else if(dynamic_cast<RMSchedulerFactory *>(_schedFactory))
        {
            VM.setImplementation(new SporadicServer(VM.getBudget(), VM.getPeriod(), VM.getName()));
        }
        else
            throw UndefinedVMImplementationException("Does not exist a VM implementation for the given scheduler");

        Server *implementation = VM.getImplementation();
        addTask(*implementation, "");
    }

    map<const AbsRTTask *, CPU *> PartionedMRTKernel::getTaskCPUMap() const
    {
        return _taskCPUMap;
    }
    
    AbsTaskAllocation *PartionedMRTKernel::getTaskAllocator() const
    {
        return _taskAllocator;
    }
    
    CPU *PartionedMRTKernel::getCPUFromTask(AbsRTTask *task){
        return _taskCPUMap[task];
    }
    
}
