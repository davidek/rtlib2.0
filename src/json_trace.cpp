#include <simul.hpp>
#include <json_trace.hpp>
#include <periodicservervm.hpp>
#include <replenishmentserver.hpp>

#include <typeinfo>
#include <cxxabi.h>

/**
 * @brief Demangle a mangled name
 *
 * Wrapper around __cxa_demangle that relieves the caller from handling
 * the allocated buffer by copying to C++ automatic variables...
 *
 * Not the most efficient thing in the world, but does the job.
 */
static inline std::string demangle(const char *name) {
    int status;
    char *buf = abi::__cxa_demangle(name, 0, 0, &status);
    std::string ret(buf);
    free(buf);
    return ret;
}


namespace RTSim {

    using namespace std;
    using namespace MetaSim;
    

    JSONTrace::JSONTrace(const string& name)
    {
        fd.clear();
        fd.open(name.c_str());
        fd << "{" << endl;
        fd << "  \"events\" : \[" << endl;
        first_event = true;
    }

    JSONTrace::~JSONTrace() {
        fd << endl;
        fd << "  ]" << endl;
        fd << "}" << endl;
        fd.close();
    }

    //  UTILITY FUNCTIONS  *************************************
    void JSONTrace::_start() {
        if (!first_event)
            fd << "," << endl;
        fd << "    {";
        first_event = false;
    }

    void JSONTrace::_end() {
        fd << "}";
        fd.flush();
    }

    void JSONTrace::_pair(const std::string &key, const std::string &val) {
        // TODO: escape key and val, if needed... or use a library for json
        fd << "\"" << key << "\": \"" << val << "\"";
    }
    void JSONTrace::_pair(const std::string &key, const Tick &val) {
        // TODO: escape key, if needed... or use a library for json
        fd << "\"" << key << "\": \"" << val << "\"";
    }

    void JSONTrace::_sep() {
        fd << ", ";
    }

    void JSONTrace::_time() {
        _pair("time", SIMUL.getTime());
    }

    void JSONTrace::writeTaskEvent(
            TaskEvt& e,
            const std::string &evt_name,
            const std::string &resource,
            const std::string &cl_name)
    {
        const Task &tt = *(e.getTask());
        _start();
        _time(); _sep();
        _pair("event_type", evt_name); _sep();
        _cpu_num(e.getCPU()); _sep();
        _task_info(tt);
        if (! resource.empty()) {
            _sep(); _pair("resource", resource);
        }
        if (! cl_name.empty()) {
            _sep(); _pair("event_class", resource);
        }
        _end();
    }

    void JSONTrace::_task_info(const Task &t) {
        _pair("task_name", t.getName());
        _sep();
        _pair("arrival_time", t.getArrival());
    }
    void JSONTrace::_cpu_num(int cpu) {
        _pair("cpu_num", ((cpu >= 0) ? to_string(cpu) : "any"));
    }

    void JSONTrace::writeServerEvent(const Server &s, const string &evt_name)
    {
        _start();
        fd << "\"time\" : \"" << SIMUL.getTime() << "\", ";
        fd << "\"event_type\" : \"" << evt_name << "\", ";
        fd << "\"server_name\" : \"" <<  s.getName() << "\", ";
        fd << "\"period\" : \"" <<  s.getPeriod() << "\", ";
        fd << "\"budget\" : \"" << s.getBudget();
        _end();
    }

    void JSONTrace::writeServerEventCPU(const Server &s, const std::string &evt_name, ServerEvt& e)
    {
        _start();
        fd << "\"time\" : \"" << SIMUL.getTime() << "\", ";
        fd << "\"event_type\" : \"" << evt_name << "\", ";
        fd << "\"cpu_num\" : \"" << ((e.getCPU() >= 0) ? to_string(e.getCPU()) : "any") << "\", ";
        fd << "\"server_name\" : \"" <<  s.getName() << "\", ";
        fd << "\"period\" : \"" <<  s.getPeriod() << "\", ";
        fd << "\"budget\" : \"" << s.getBudget();
        _end();
    }

    void JSONTrace::writeServerEventCPU(const ReplenishmentServer &s, const std::string &evt_name, ServerEvt& e)
    {
        _start();
        fd << "\"time\" : \"" << SIMUL.getTime() << "\", ";
        fd << "\"event_type\" : \"" << evt_name << "\", ";
        fd << "\"cpu_num\" : \"" << ((e.getCPU() >= 0) ? to_string(e.getCPU()) : "any") << "\", ";
        fd << "\"server_name\" : \"" <<  s.getName() << "\", ";
        fd << "\"period\" : \"" <<  s.getPeriod() << "\", ";
        fd << "\"current_budget\" : \"" <<  s.getCurrentBudget() << "\", ";
        fd << "\"budget\" : \"" << s.getBudget();
        _end();
    }

    void JSONTrace::writeServerEvent(const ReplenishmentServer &s, const std::string &evt_name)
    {
        _start();
        fd << "\"time\" : \"" << SIMUL.getTime() << "\", ";
        fd << "\"event_type\" : \"" << evt_name << "\", ";
        fd << "\"server_name\" : \"" <<  s.getName() << "\", ";
        fd << "\"period\" : \"" <<  s.getPeriod() << "\", ";
        fd << "\"current_budget\" : \"" <<  s.getCurrentBudget() << "\", ";
        fd << "\"budget\" : \"" << s.getBudget();
        _end();
    }

    //  GENERIC EVENT ***************************************
    void JSONTrace::probe(Event &e)
    {
        _start();
        _time();
        _pair("event_type", "UNKNOWN"); _sep();
        _pair("event_class", demangle(typeid(e).name()));
        _end();
    }

    //  TASK EVENTS ***************************************
    void JSONTrace::probe(TaskEvt& e)
    {
        writeTaskEvent(e, "UNKNOWN-TaskEvt", "", demangle(typeid(e).name()));
    }
    void JSONTrace::probe(ArrEvt& e)
    {
        writeTaskEvent(e, "arrival");
    }
    void JSONTrace::probe(EndEvt& e)
    {
        writeTaskEvent(e, "end_instance");
    }
    void JSONTrace::probe(SchedEvt& e)
    {
        writeTaskEvent(e, "scheduled");
    }
    void JSONTrace::probe(DeschedEvt& e)
    {
        writeTaskEvent(e, "descheduled");
    }
    void JSONTrace::probe(DeadEvt& e)
    {
        writeTaskEvent(e, "dline_miss");
    }
    void JSONTrace::probe(WaitEvt& e)
    {
        writeTaskEvent(e, "wait", e.getInstr()->getResource());
    }
    void JSONTrace::probe(SignalEvt& e)
    {
        writeTaskEvent(e, "signal", e.getInstr()->getResource());
    }

    //  SERVER EVENTS ***************************************
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

    //  OTHER EVENTS ***************************************
    void JSONTrace::probe(EndInstrEvt &e)
    {
        const char *instr_type = "";
        string resource;
        string instr_cl;

        Task &task = *(e.getInstruction()->getTask());
        if (WaitInstr *i =
                dynamic_cast<WaitInstr *>(e.getInstruction())) {
            resource = i->getResource();
            instr_type = "wait";
        } else if (SignalInstr *i =
                dynamic_cast<SignalInstr *>(e.getInstruction())) {
            resource = i->getResource();
            instr_type = "signal";
        } else {
            instr_cl = demangle(typeid(*e.getInstruction()).name());
        }

        _start();
        _time(); _sep();
        _pair("event_type", "end_instr"); _sep();
        _task_info(task); _sep();
        _pair("instr_type", instr_type);
        if (! resource.empty()) {
            _sep(); _pair("resource", resource);
        }
        if (! instr_cl.empty()) {
            _sep(); _pair("instr_class", instr_cl);
        }
        _end();
    }

    void JSONTrace::probe(SystemCeilingChangedEvt &e)
    {
        _start();
        _time(); _sep();
        _pair("event_type", "system_ceiling_changed"); _sep();
        _pair("ceiling", e.systemCeiling());
        _end();
    }



    // attachTo* METHODS  ******************************
    void JSONTrace::attachToTask(Task* t)
    {
        new Particle<ArrEvt, JSONTrace>(&t->arrEvt, this);
        new Particle<EndEvt, JSONTrace>(&t->endEvt, this);
        new Particle<SchedEvt, JSONTrace>(&t->schedEvt, this);
        new Particle<DeschedEvt, JSONTrace>(&t->deschedEvt, this);
        new Particle<DeadEvt, JSONTrace>(&t->deadEvt, this);
        new Particle<FakeArrEvt, JSONTrace>(&t->fakeArrEvt, this);

        typedef std::vector< Instr * > InstrList;
        typedef std::vector< Instr * >::const_iterator  ConstInstrIT;

        InstrList instructions = t->getInstrQueue();
        for (ConstInstrIT it=instructions.begin(); it<instructions.end(); it++){
            attachToInstr(*it);
        }
    }

    void JSONTrace::attachToInstr(Instr *i) {
        if (ExecInstr *ii = dynamic_cast<ExecInstr *>(i)) {
            new Particle<EndInstrEvt, JSONTrace>(&ii->_endEvt, this);
        }
        else if (WaitInstr *ii = dynamic_cast<WaitInstr *>(i)) {
            new Particle<WaitEvt, JSONTrace>(&ii->_waitEvt, this);
            new Particle<EndInstrEvt, JSONTrace>(&ii->_endEvt, this);
        }
        else if (SignalInstr *ii = dynamic_cast<SignalInstr *>(i)) {
            new Particle<SignalEvt, JSONTrace>(&ii->_signalEvt, this);
            new Particle<EndInstrEvt, JSONTrace>(&ii->_endEvt, this);
        }
        else if (ThreInstr *ii = dynamic_cast<ThreInstr *>(i)) {
            new Particle<ThreEvt, JSONTrace>(&ii->_threEvt, this);
            new Particle<EndInstrEvt, JSONTrace>(&ii->_endEvt, this);
        }
        else if (SchedInstr *ii = dynamic_cast<SchedInstr *>(i)) {
            new Particle<SchedIEvt, JSONTrace>(&ii->_threEvt, this);
            new Particle<EndInstrEvt, JSONTrace>(&ii->_endEvt, this);
        }
        else if (SuspendInstr *ii = dynamic_cast<SuspendInstr *>(i)) {
            new Particle<MetaSim::GEvent<SuspendInstr>, JSONTrace>(&ii->suspEvt, this);
            new Particle<MetaSim::GEvent<SuspendInstr>, JSONTrace>(&ii->resumeEvt, this);
        }
        else {
            DBGENTER("JSONTrace");
            DBGPRINT("WARNING: JSONTrace got an unknown instruction, nothing will be done");
        }
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

    void JSONTrace::attachToSRPResMan(SRPResManager *resman)
    {
        new Particle<SystemCeilingChangedEvt, JSONTrace>(&resman->_ceilingChangedEvt, this);
    }
}
