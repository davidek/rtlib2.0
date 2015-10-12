#include <texttrace.hpp>

namespace RTSim {
        using namespace std;
        using namespace MetaSim;

        TextTrace::TextTrace(const string& name)
		{

            fd.open(name.c_str());
		}

		TextTrace::~TextTrace()
		{
            fd.flush();
            fd.clear();
			fd.close();
		}

		void TextTrace::probe(ArrEvt& e)
		{
			Task* tt = e.getTask();
            fd << "[Time:" << SIMUL.getTime() << "]\t";  
            fd << tt->getName() << " arrived at " 
            << tt->getArrival() << endl;                
		}

		void TextTrace::probe(EndEvt& e)
		{
			Task* tt = e.getTask();
			fd << "[Time:" << SIMUL.getTime() << "]\t";
			fd << tt->getName()<<" ended, its arrival was " 
				<< tt->getArrival() << endl;
		}

		void TextTrace::probe(SchedEvt& e)
		{
			Task* tt = e.getTask();
			fd << "[Time:" << SIMUL.getTime() << "]\t";  
			fd << tt->getName()<<" scheduled on CPU #"<< e.getCPU() <<"; its arrival was " 
				<< tt->getArrival() << endl; 
		}

		void TextTrace::probe(DeschedEvt& e)
		{
			Task* tt = e.getTask();
			fd << "[Time:" << SIMUL.getTime() << "]\t";  
			fd << tt->getName()<<" descheduled from CPU #"<< e.getCPU() <<";its arrival was " 
				<< tt->getArrival() << endl;
		}

		void TextTrace::probe(DeadEvt& e)
		{
			Task* tt = e.getTask();
			fd << "[Time:" << SIMUL.getTime() << "]\t";  
			fd << tt->getName()<<" missed its arrival was " 
               << tt->getArrival() << endl;
        }

        void TextTrace::probe(ServerBudgetExhaustedEvt &e)
        {
            Server* s = e.getServer();
            fd << "[Time:" << SIMUL.getTime() << "]\t";
            fd << s->getName() <<" exhausts the budget " << endl;
        }

        void TextTrace::probe(ServerRechargingEvt &e)
        {
            Server* s = e.getServer();
            fd << "[Time:" << SIMUL.getTime() << "]\t";
            fd << s->getName() <<" recharges its budget" << endl;
        }

        void TextTrace::probe(ServerScheduledEvt &e)
        {
            Server* s = e.getServer();
            fd << "[Time:" << SIMUL.getTime() << "]\t";
            fd << s->getName() <<" scheduled on CPU #"<< e.getCPU() <<"; its arrival was "
               << s->getArrival() << endl;
        }

        void TextTrace::probe(ServerDescheduledEvt &e)
        {
            Server* s = e.getServer();
            fd << "[Time:" << SIMUL.getTime() << "]\t";
            fd << s->getName() <<" descheduled from CPU #"<< e.getCPU() <<"; its arrival was "
               << s->getArrival() << endl;
        }

        void TextTrace::probe(ServerReplenishmentEvt &e)
        {
            ReplenishmentServer* s = e.getServer();
            fd << "[Time:" << SIMUL.getTime() << "]\t";
            fd << s->getName() <<" has a replenishment; The current budget is "
               << s->getCurrentBudget() << "(the total is " << s->getBudget() << ")"
               <<endl;
        }

		void TextTrace::attachToTask(Task* t)
		{
			new Particle<ArrEvt, TextTrace>(&t->arrEvt, this);
			new Particle<EndEvt, TextTrace>(&t->endEvt, this);
			new Particle<SchedEvt, TextTrace>(&t->schedEvt, this);
			new Particle<DeschedEvt, TextTrace>(&t->deschedEvt, this);
			new Particle<DeadEvt, TextTrace>(&t->deadEvt, this);
		}

        void TextTrace::attachToServer(Server *s)
        {
            new Particle<ServerBudgetExhaustedEvt, TextTrace>(&s->_bandExEvt, this);
            new Particle<ServerRechargingEvt, TextTrace>(&s->_rechargingEvt, this);
            new Particle<ServerScheduledEvt, TextTrace>(&s->_schedEvt, this);
            new Particle<ServerDescheduledEvt, TextTrace>(&s->_deschedEvt, this);
            if(ReplenishmentServer *rs = dynamic_cast<ReplenishmentServer *> (s))
                new Particle<ServerReplenishmentEvt, TextTrace>(&rs->_replEvt, this);
        }

        void TextTrace::attachToPeriodicServerVM(PeriodicServerVM *VM)
        {
            attachToServer(VM->getImplementation());
        }
    
        VirtualTrace::VirtualTrace(map<string, int> *r)
        {
            results = r;
        }
        
        VirtualTrace::~VirtualTrace()
        {
            
        }
    
        void VirtualTrace::probe(EndEvt& e)
        {
            Task* tt = e.getTask();
            auto tmp_wcrt = SIMUL.getTime() - tt->getArrival();
            
            if ((*results)[tt->getName()] < tmp_wcrt)
            {
                (*results)[tt->getName()] = tmp_wcrt;
            }
        }

        void VirtualTrace::attachToTask(Task* t)
        {
            new Particle<EndEvt, VirtualTrace>(&t->endEvt, this);
        }
    
};
