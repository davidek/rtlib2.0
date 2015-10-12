#ifndef __SPORADICSERVER_H__
#define __SPORADICSERVER_H__

#include <replenishmentserver.hpp>


namespace RTSim {
    
    using namespace MetaSim;
    
    class SporadicServer : public ReplenishmentServer {
    public:
        SporadicServer(Tick q, Tick p, const std::string &name,
                       const std::string &sched = "FIFOSched");
        
        void newRun();
        void endRun();
        
        
        Tick changeBudget(const Tick &n);
        
        Tick changeQ(const Tick &n);
        virtual double getVirtualTime();
        
    protected:
        
        /// from idle to active contending (new work to do)
        virtual void idle_ready();
        
        /// from active non contending to active contending (more work)
        virtual void releasing_ready();
        
        /// from active contending to executing (dispatching)
        virtual void ready_executing();
        
        /// from executing to active contenting (preemption)
        virtual void executing_ready();
        
        /// from executing to active non contending (no more work)
        virtual void executing_releasing();
        
        /// from active non contending to idle (no lag)
        virtual void releasing_idle();
        
        /// from executing to recharging (budget exhausted)
        virtual void executing_recharging();
        
        /// from recharging to active contending (budget recharged)
        virtual void recharging_ready();
        
        /// from recharging to active contending (budget recharged)
        virtual void recharging_idle();
        
        void onReplenishment(Event *e);
        
        void onIdle(Event *e);
        
        void prepare_replenishment(const Tick &t);
        
        void check_repl();
        
    };
}


#endif
