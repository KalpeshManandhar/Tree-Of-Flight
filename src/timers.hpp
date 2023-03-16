#pragma once
#include <fstream>
#include <vector>
#include <chrono>
class Timer
{
private:
    // Type aliases to make accessing nested type easier
    using Clock = std::chrono::steady_clock;
    using Second = std::chrono::duration<double, std::ratio<1> >;

    std::chrono::time_point<Clock> m_beg{ Clock::now() };

public:
    void reset()
    {
        m_beg = Clock::now();
    }

    double elapsed() const
    {
        return std::chrono::duration_cast<Second>(Clock::now() - m_beg).count();
    }
};


struct Analyzer {
    struct Trigger {
        double sum = 0.0;
        size_t count = 1;
        size_t id;
        std::string name;
        Timer timer;
        bool triggered = false;
        bool loop_once = false;
    };

    std::vector<Trigger> timers;
    //Temporary timer
    Timer tt;
    //Constant timer
    Timer const_timer;
    size_t total_loops = 1;
    std::string group_name;
    Analyzer(std::string name="") {
        
        group_name = name;
        timers.reserve(20);
        tt.reset();
        const_timer.reset();
    }
    void reset_all() {
        timers.clear();
        total_loops = 1;
        tt.reset();
        const_timer.reset();
    }
    size_t start(size_t id, const char* name = nullptr) {
        Trigger* unt = nullptr;
        for (auto& x : timers)
            if (x.id == id)
                unt = &x;
        if (!unt) {
            Trigger nunit{
                .id = id
            };
            timers.push_back(nunit);
            unt = &timers.back();
        }
        if (name)
            unt->name = name;
        if (unt->triggered)
            return id;
        unt->triggered = true;
        unt->loop_once = true;
        unt->timer.reset();
        return id;
    }
    void end(size_t id) {
        Trigger* unt = nullptr;
        for (auto& x : timers)
            if (x.id == id)
                unt = &x;
        
        if (!unt || !unt->triggered) {
            return;
        }
        double delt = unt->timer.elapsed();
        unt->triggered = false;
        unt->sum += delt;
    }
    void loop() {
        total_loops++;
        tt.reset();
        for (auto& tmr : timers) {
            if (tmr.loop_once)
                tmr.count++;
            tmr.loop_once = false;
            if (tmr.triggered)
                end(tmr.id);
        }
    }

    friend std::ostream& operator << (std::ostream& os, const Analyzer& analyzer) {
        os << std::endl<<analyzer.group_name << " timer results : " << std::endl;
        double total_time = analyzer.const_timer.elapsed();
        double total_sum = 0.0;
        double avg_total = 0.0;
        for (auto& tmr : analyzer.timers) {
            total_sum += tmr.sum;
            avg_total += tmr.sum / tmr.count;
        }
        for (auto& tmr : analyzer.timers) {
            os << "Id : " << tmr.id << '"' << tmr.name << '"' << std::endl;
            os << '\t' << "Average time taken per loop : " 
                << tmr.sum / tmr.count;
            os << "\n\t" << "% Time by average time per loop of timed events : "
                << (100 * tmr.sum) / (tmr.count * avg_total);
            os << "\n\t" << "% Time among all timed events : " 
                << 100.0 * tmr.sum / total_sum;
            os << "\n\t" << "% Time by complete loop events : " 
                << 100.0 * tmr.sum / total_time << '\n';
        }
        os << "Average time per loop : " << total_time / analyzer.total_loops << std::endl;
        os << "Average iterations per loop : " << analyzer.total_loops / total_time << std::endl;
        return os;
    }

};

struct Trigger_Timer {
    Trigger_Timer(Analyzer& tool, size_t id, const char * str = nullptr) :m_tool(tool) {
        m_tool.start(id, str);
        id_n = id;
    }
    size_t  id_n;
    Analyzer& m_tool;
    ~Trigger_Timer() {
        m_tool.end(id_n);
    }
};

#define STRINGIZE(x) STRINGIZE2(x)
#define STRINGIZE2(x) #x
#define LINE_STRING STRINGIZE(__LINE__)

//Used to create a timer on the analyzer object that automatically stops timing with scope rules
//Use only one of these, as identifier name is same
#define SCOPE_TIMER_ID(obj,id) \
Trigger_Timer Scoped_Timer_Object(obj,id,"line : "LINE_STRING);
#define SCOPE_TIMER_NAME(obj, name) \
Trigger_Timer Scoped_Timer_Object(obj,(__COUNTER__^__LINE__+__COUNTER__),#name"line : "LINE_STRING);
#define SCOPE_TIMER(obj) SCOPE_TIMER_ID(obj,(__COUNTER__^__LINE__+__COUNTER__))


//Used to create analyzer which encode in it function name and file name, so should be created only inside functions
#define MAKE_ANALYZER(name) Analyzer(std::string(__FILE__)+":"+__FUNCTION__+":"#name)

//Used to create analyzer which encode in it only file name, so can be created outside functions as global analyzer
#define MAKE_ANALYZER_GL(name) Analyzer(std::string(__FILE__)+" : "#name)


//Used to start timer for the analyzer with encoding of line number, perhaps not, but be careful of name collisions with id
#define START_TIMER(analyzer, name)\
 size_t id##analyzer##name = analyzer.start(__COUNTER__^__LINE__+__COUNTER__,#name",line : "LINE_STRING);
#define START_TIMER_PTR(analyzer, name)\
 size_t id##analyzer##name = analyzer->start(__COUNTER__^__LINE__+__COUNTER__,#name",line : "LINE_STRING);

//Used to stop timer for the analyzer that was created by above START_TIMER
#define END_TIMER(analyzer, name)\
   analyzer.end(id##analyzer##name)
#define END_TIMER_PTR(analyzer, name)\
   analyzer->end(id##analyzer##name)

#define LOG_FILE_DATE_TIME\
 std::string("\nFile Compilation : ")+__TIMESTAMP__+" File Execution : " + \
std::format("{:%Y-%m-%d %X}", std::chrono::current_zone()->to_local(std::chrono::system_clock::now()))\
+"\n"