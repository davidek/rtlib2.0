#include <json_trace.hpp>
#include <periodicservervm.hpp>
#include <replenishmentserver.hpp>

namespace RTSim {

    using namespace std;
    using namespace MetaSim;
    

    JSONTrace::JSONTrace(const string& name)
    {
        fd.clear();
        fd.open(name.c_str());
        fd << "{" << endl;
        fd << "    \"events\" : \[" << endl; 
        first_event = true;
    }

    JSONTrace::~JSONTrace() {
        fd << "] }" << endl;
        fd.close();
    }

    void JSONTrace::writeTaskEvent(const Task &tt, const std::string &evt_name)
    {
        if (!first_event) 
            fd << "," << endl;
        else
            first_event = false;
        fd << "{ " ;
        fd << "\"time\" : \"" << SIMUL.getTime() << "\", ";  
        fd << "\"event_type\" : \"" << evt_name << "\", ";
        fd << "\"task_name\" : \"" <<  tt.getName() << "\",";
        fd << "\"arrival_time\" : \"" << tt.getArrival() << "\"}";
    }

    void JSONTrace::writeTaskEventCPU(const Task &tt, const std::string &evt_name, TaskEvt& e)
    {
        if (!first_event)
            fd << "," << endl;
        else
            first_event = false;
        fd << "{ " ;
        fd << "\"time\" : \"" << SIMUL.getTime() << "\", ";
        fd << "\"event_type\" : \"" << evt_name << "\", ";
        fd << "\"cpu_num\" : \"" << ((e.getCPU() >= 0) ? to_string(e.getCPU()) : "any") << "\", ";
        fd << "\"task_name\" : \"" <<  tt.getName() << "\",";
        fd << "\"arrival_time\" : \"" << tt.getArrival() << "\"}";
    }

    void JSONTrace::writeServerEvent(const Server &s, const string &evt_name)
    {
        if (!first_event)
            fd << "," << endl;
        else
            first_event = false;
        fd << "{ " ;
        fd << "\"time\" : \"" << SIMUL.getTime() << "\", ";
        fd << "\"event_type\" : \"" << evt_name << "\", ";
        fd << "\"server_name\" : \"" <<  s.getName() << "\", ";
        fd << "\"period\" : \"" <<  s.getPeriod() << "\", ";
        fd << "\"budget\" : \"" << s.getBudget() << "\"}";
    }

    void JSONTrace::writeServerEventCPU(const Server &s, const std::string &evt_name, ServerEvt& e)
    {
        if (!first_event)
            fd << "," << endl;
        else
            first_event = false;
        fd << "{ " ;
        fd << "\"time\" : \"" << SIMUL.getTime() << "\", ";
        fd << "\"event_type\" : \"" << evt_name << "\", ";
        fd << "\"cpu_num\" : \"" << ((e.getCPU() >= 0) ? to_string(e.getCPU()) : "any") << "\", ";
        fd << "\"server_name\" : \"" <<  s.getName() << "\", ";
        fd << "\"period\" : \"" <<  s.getPeriod() << "\", ";
        fd << "\"budget\" : \"" << s.getBudget() << "\"}";
    }

    void JSONTrace::writeServerEventCPU(const ReplenishmentServer &s, const std::string &evt_name, ServerEvt& e)
    {
        if (!first_event)
            fd << "," << endl;
        else
            first_event = false;
        fd << "{ " ;
        fd << "\"time\" : \"" << SIMUL.getTime() << "\", ";
        fd << "\"event_type\" : \"" << evt_name << "\", ";
        fd << "\"cpu_num\" : \"" << ((e.getCPU() >= 0) ? to_string(e.getCPU()) : "any") << "\", ";
        fd << "\"server_name\" : \"" <<  s.getName() << "\", ";
        fd << "\"period\" : \"" <<  s.getPeriod() << "\", ";
        fd << "\"current_budget\" : \"" <<  s.getCurrentBudget() << "\", ";
        fd << "\"budget\" : \"" << s.getBudget() << "\"}";
    }

    void JSONTrace::writeServerEvent(const ReplenishmentServer &s, const std::string &evt_name)
    {
        if (!first_event)
            fd << "," << endl;
        else
            first_event = false;
        fd << "{ " ;
        fd << "\"time\" : \"" << SIMUL.getTime() << "\", ";
        fd << "\"event_type\" : \"" << evt_name << "\", ";
        fd << "\"server_name\" : \"" <<  s.getName() << "\", ";
        fd << "\"period\" : \"" <<  s.getPeriod() << "\", ";
        fd << "\"current_budget\" : \"" <<  s.getCurrentBudget() << "\", ";
        fd << "\"budget\" : \"" << s.getBudget() << "\"}";
    }

    void JSONTrace::probe(ArrEvt& e)
    {
        Task& tt = *(e.getTask());
        //writeTaskEvent(tt, "arrival");
		writeTaskEventCPU(tt, "arrival", e);
    }
    
    void JSONTrace::probe(EndEvt& e)
    {
        Task& tt = *(e.getTask());
        //writeTaskEvent(tt, "end_instance");
		writeTaskEventCPU(tt, "end_instance", e);
    }
    
    void JSONTrace::probe(SchedEvt& e)
    {
        Task& tt = *(e.getTask());
		//writeTaskEvent(tt, "scheduled");
        writeTaskEventCPU(tt, "scheduled", e);
    }
    
    void JSONTrace::probe(DeschedEvt& e)
    {
        Task& tt = *(e.getTask());
		 //writeTaskEvent(tt, "descheduled");
		writeTaskEventCPU(tt, "descheduled", e);
    }
    
    void JSONTrace::probe(DeadEvt& e)
    {
        Task& tt = *(e.getTask());
        //writeTaskEvent(tt, "dline_miss");
        writeTaskEventCPU(tt, "dline_miss", e);
    }

    void JSONTrace::probe(ServerBudgetExhaustedEvt &e)
    {
        Server& s = *(e.getServer());
        writeServerEvent(s, "budget_exhausted");
    }

    void JSONTrace::probe(ServerDMissEvt &e)
    {
        Server& s = *(e.getServer());
        writeServerEventCPU(s, "dline_miss", e);
    }

    void JSONTrace::probe(ServerRechargingEvt &e)
    {
        Server& s = *(e.getServer());
        writeServerEvent(s, "recharging");
    }

    void JSONTrace::probe(ServerScheduledEvt &e)
    {
        Server *s = e.getServer();
        if(ReplenishmentServer *rServ = dynamic_cast<ReplenishmentServer *> (s)){
            writeServerEventCPU(*rServ, "scheduled", e);
        }
        else
            writeServerEventCPU(*s, "scheduled", e);
    }

    void JSONTrace::probe(ServerDescheduledEvt &e)
    {
        Server *s = e.getServer();
        if(ReplenishmentServer *rServ = dynamic_cast<ReplenishmentServer *> (s)){
            writeServerEventCPU(*rServ, "descheduled", e);
        }
        else
            writeServerEventCPU(*s, "descheduled", e);
        //writeServerEventCPU(%s, "descheduled", e);
    }

  void JSONTrace::probe(ServerReplenishmentEvt &e)
  {
      ReplenishmentServer *s = e.getServer();
      /*if(ReplenishmentServer *rServ = dynamic_cast<ReplenishmentServer *> (s)){
          writeServerEvent(*rServ, "replenishment");
      }
      else*/
          writeServerEvent(*s, "replenishment");
  }

    void JSONTrace::attachToTask(Task* t)
    {
        new Particle<ArrEvt, JSONTrace>(&t->arrEvt, this);
        new Particle<EndEvt, JSONTrace>(&t->endEvt, this);
        new Particle<SchedEvt, JSONTrace>(&t->schedEvt, this);
        new Particle<DeschedEvt, JSONTrace>(&t->deschedEvt, this);
        new Particle<DeadEvt, JSONTrace>(&t->deadEvt, this);
    }

    void JSONTrace::attachToServer(Server *s)
    {
        new Particle<ServerBudgetExhaustedEvt, JSONTrace>(&s->_bandExEvt, this);
        new Particle<ServerRechargingEvt, JSONTrace>(&s->_rechargingEvt, this);
        new Particle<ServerDMissEvt, JSONTrace>(&s->_dlineMissEvt, this);
        new Particle<ServerScheduledEvt, JSONTrace>(&s->_schedEvt, this);
        new Particle<ServerDescheduledEvt, JSONTrace>(&s->_deschedEvt, this);
        if(ReplenishmentServer *rs = dynamic_cast<ReplenishmentServer *> (s))
            new Particle<ServerReplenishmentEvt, JSONTrace>(&rs->_replEvt, this);
    }

    void JSONTrace::attachToPeriodicServerVM(PeriodicServerVM *VM)
    {
        attachToServer(VM->getImplementation());
    }
}
