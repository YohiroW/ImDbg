#include "TraceServices/Model/Threads.h"

class ImDbgCPUTracerScopeAnalyzer
{

};

class FImDbgCPUTracerProvider 
: public TraceServices::IEditableThreadProvider
, public TraceServices::IEditableTimingProfilerProvider
{
public:
    // IEditableThreadProvider interface
    virtual void AddThread(uint32 Id, const TCHAR* Name, EThreadPriority Priority) override;

    // IEditableTimingProfilerProvider interface
	virtual uint32 AddCpuTimer(FStringView Name, const TCHAR* File, uint32 Line) override;
	virtual void SetTimerNameAndLocation(uint32 TimerId, FStringView Name, const TCHAR* File, uint32 Line) override;
	virtual uint32 AddMetadata(uint32 OriginalTimerId, TArray<uint8>&& Metadata) override;
	virtual TArrayView<const uint8> GetMetadata(uint32 TimerId) const override;
	virtual IEditableTimeline<FTimingProfilerEvent>& GetCpuThreadEditableTimeline(uint32 ThreadId) override;
};