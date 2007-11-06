// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2005 University of California
//
// This is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation;
// either version 2.1 of the License, or (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// To view the GNU Lesser General Public License visit
// http://www.gnu.org/copyleft/lesser.html
// or write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#ifndef _THREADPOOL_H_
#define _THREADPOOL_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "ThreadPool.cpp"
#endif

#include "wx/valgen.h"


class CThreadPool : public wxObject {

    DECLARE_DYNAMIC_CLASS(CThreadPool)

public:

    CThreadPool() {};
    CThreadPool(const CThreadPool& val) {};

    ~CThreadPool() {};

    //virtual wxObject* Clone() const { return new CThreadPool(*this); }
    //virtual bool      Copy(const CThreadPool& val);

    void QueueWorkItem();

    // Min and max sizes are "lazy" - initially, the pool size may be less
    // than the minimum - threads are created as needed. When reducing the maximum,
    // no further threads are created but no threads are removed to force compliance
    // with the new maximum.
    int GetCurrentPoolSize();
    int GetMaxPoolSize() { return m_maxPoolSize; }
    int GetMinPoolSize() { return m_minPoolSize; }
    void SetMaxPoolSize(int size) { m_maxPoolSize = size; }
    void SetMinPoolSize(int size) { m_minPoolSize = size; }

    int GetCurrentQueueSize();
    void SetMaxQueueSize(int size);

protected:

private:
    int m_maxPoolSize;
    int m_minPoolSize;

    // Work items are maintained in a FIFO queue. Nothing fancy.

    // Threads are maintained in a stack - starve the least-used threads so they can time out.

    // While threads are active, they are removed from the stack and associated with the relevant work item.

    // nested thread class.

};

// work item class


#endif // _THREADPOOL_H_

