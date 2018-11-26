//
//  inl
//
//  Created by Federico Spadoni on 29/01/15.
//
//

//#include "physics/CCTask.h"


namespace foundation
{


inline Task::Status::Status()
: _busy(0)
{
}

inline bool Task::Status::isBusy() const
{
    return (_busy.load(std::memory_order_seq_cst) > 0);
}

inline void Task::Status::markBusy(bool busy)
{
    if (busy)
    {
        _busy++;
    }
    else
    {
        _busy--;
    }
}



inline Task::Status* Task::getStatus(void) const
{
    return const_cast<Task::Status*>(_status);
}



} //namespace foundation