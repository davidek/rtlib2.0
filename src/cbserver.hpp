#ifndef __CBSERVER_H__
#define __CBSERVER_H__

#include <replenishmentserver.hpp>

namespace RTSim {
    using namespace MetaSim;

    class CBServer : public ReplenishmentServer {
    public:
	typedef enum {ORIGINAL, REUSE_DLINE } policy_t;

	CBServer(Tick q, Tick p, Tick d, bool HR, const std::string &name, 
		 const std::string &sched = "FIFOSched");

        void newRun();
        void endRun();
      

        Tick changeBudget(const Tick &n);

        Tick changeQ(const Tick &n);
        virtual double getVirtualTime();
        Tick get_remaining_budget();

        policy_t get_policy() const { return idle_policy; }
        void set_policy(policy_t p) { idle_policy = p; }

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

    private:
        Tick d;
        int HR;

	/** if the server is in IDLE, and idle_policy==true, the
	    original CBS policy is used (that computes a new deadline
	    as t + P) 

	    If the server is IDLE and t < d and idle_policy==false, then 
	    reuses the old deadline, and computes a new "safe" budget as 
	    floor((d - vtime) * Q / P). 
	*/
	policy_t idle_policy; 
    };
}


#endif
