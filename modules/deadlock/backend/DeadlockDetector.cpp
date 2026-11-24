#include "DeadlockDetector.h"
#include <sstream>
namespace deadlock {
namespace {
std::string vec(const std::vector<int>& v) { std::ostringstream s; s << '['; for(size_t i=0;i<v.size();++i) { if(i) s << ", "; s << v[i]; } s << ']'; return s.str(); }
std::string validate(const Matrix& a,const Matrix& b,const std::vector<int>& v,bool maxMode) {
    if(a.empty() || v.empty()) return "At least one process and resource type are required.";
    if(b.size()!=a.size()) return "Matrix row counts differ.";
    for(int x:v) if(x<0) return "Available resources cannot be negative.";
    for(size_t i=0;i<a.size();++i) {
        if(a[i].size()!=v.size() || b[i].size()!=v.size()) return "Matrix dimensions do not match resource count.";
        for(size_t j=0;j<v.size();++j) {
            if(a[i][j]<0 || b[i][j]<0) return "Resource values cannot be negative.";
            if(maxMode && a[i][j]>b[i][j]) return "Allocation exceeds maximum claim.";
        }
    }
    return {};
}
bool fits(const std::vector<int>& need,const std::vector<int>& work) { for(size_t j=0;j<work.size();++j) if(need[j]>work[j]) return false; return true; }
Result evaluate(const Matrix& a,const Matrix& pending,const std::vector<int>& available,bool detection) {
    Result result; result.valid=true;
    std::vector<int> work=available;
    std::vector<bool> done(a.size(),false);
    if(detection) for(size_t i=0;i<a.size();++i) {
        bool zero=true; for(int x:a[i]) if(x) zero=false;
        if(zero) { done[i]=true; result.steps.push_back("P"+std::to_string(i)+" holds no resources; excluded from deadlock set."); }
    }
    result.steps.push_back("Initial Work = "+vec(work));
    bool progress=true;
    while(progress) {
        progress=false;
        for(size_t i=0;i<a.size();++i) {
            if(done[i] || !fits(pending[i],work)) continue;
            const auto before=work;
            for(size_t j=0;j<work.size();++j) work[j]+=a[i][j];
            done[i]=true; progress=true; result.sequence.push_back(static_cast<int>(i));
            result.steps.push_back("P"+std::to_string(i)+" can finish: Need/Request "+vec(pending[i])+" <= Work "+vec(before)+"; releases "+vec(a[i])+"; Work becomes "+vec(work));
        }
    }
    for(size_t i=0;i<done.size();++i) if(!done[i]) result.unfinished.push_back(static_cast<int>(i));
    result.safe=result.unfinished.empty();
    result.steps.push_back(result.safe ? (detection?"No deadlocked processes detected.":"System is in a SAFE state.") : (detection?"Deadlocked processes remain.":"UNSAFE state: no complete safe sequence exists."));
    return result;
}
}
Result bankers(const Matrix& allocation,const Matrix& maximum,const std::vector<int>& available) {
    Result r; r.error=validate(allocation,maximum,available,true); if(!r.error.empty()) return r;
    Matrix need=maximum;
    for(size_t i=0;i<need.size();++i) for(size_t j=0;j<available.size();++j) need[i][j]-=allocation[i][j];
    return evaluate(allocation,need,available,false);
}
Result detect(const Matrix& allocation,const Matrix& request,const std::vector<int>& available) {
    Result r; r.error=validate(allocation,request,available,false); if(!r.error.empty()) return r;
    return evaluate(allocation,request,available,true);
}
RequestResult requestResources(const Matrix& allocation,const Matrix& maximum,const std::vector<int>& available,int pid,const std::vector<int>& request) {
    RequestResult out; auto baseline=bankers(allocation,maximum,available);
    if(!baseline.valid) {out.reason=baseline.error; return out;}
    if(pid<0 || static_cast<size_t>(pid)>=allocation.size() || request.size()!=available.size()) {out.reason="Invalid process or request dimensions.";return out;}
    for(size_t j=0;j<request.size();++j) {
        if(request[j]<0) {out.reason="Requests cannot be negative.";return out;}
        if(request[j]>maximum[pid][j]-allocation[pid][j]) {out.reason="Request exceeds remaining maximum need.";return out;}
        if(request[j]>available[j]) {out.reason="Insufficient currently available resources; process must wait.";return out;}
    }
    Matrix next=allocation; auto avail=available;
    for(size_t j=0;j<request.size();++j) {next[pid][j]+=request[j];avail[j]-=request[j];}
    out.safety=bankers(next,maximum,avail);out.granted=out.safety.safe;
    out.reason=out.granted?"Request granted: tentative allocation remains safe.":"Request denied: tentative allocation would be unsafe.";
    return out;
}
}
