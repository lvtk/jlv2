/*
    This file is part of the lvtk_core module for the JUCE Library
    Copyright (c) 2013 - Michael Fisher <mfisher31@gmail.com>.

    Permission to use, copy, modify, and/or distribute this software for any purpose with
    or without fee is hereby granted, provided that the above copyright notice and this
    permission notice appear in all copies.

    THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD
    TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN
    NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
    DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
    IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
    CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#ifndef LVTK_JUCE_WORKTHREAD_H
#define LVTK_JUCE_WORKTHREAD_H

class Worker;

/** A worker thread
    Capable of scheduling non-realtime work from a realtime context. */
class WorkThread :  public Thread
{
public:
    
    WorkThread (const String& name, uint32 bufsize, int32 priority = 5);
    ~WorkThread();
    
    inline static uint32 requiredSpace (uint32 msgSize) { return msgSize + (2 * sizeof (uint32)); }
    
protected:
    
    friend class Worker;
    
    /** Register a worker for scheduling. Does not take ownership */
    void registerWorker (Worker* worker);
    
    /** Deregister a worker from scheduling. Does not delete the worker */
    void removeWorker (Worker* worker);
    
    /** Schedule non-realtime work
        Workers will call this in Worker::scheduleWork */
    bool scheduleWork (Worker* worker, uint32 size, const void* data);

private:
    
    uint32 bufferSize;
    
    Worker* getWorker (uint32 workerId) const;
    Array<Worker*, CriticalSection> workers;
    
    uint32 nextWorkId;
    
    Semaphore sem;
    bool doExit;
    
    ScopedPointer<RingBuffer> requests;  ///< requests to process
    
    /** @internal Validate a ringbuffer for message completeness */
    bool validateMessage (RingBuffer& ring);
    
    /** @internal The work thread function */
    void run();
};


class Worker
{
public:
    
    /** Create a new Worker
        @param thread The WorkThread to use when scheduling
        @param bufsize Size to use for internal response buffers */
    Worker (WorkThread& thread, uint32 bufsize);
    virtual ~Worker();
    
    /** Schedule work (realtime thread).
        Work will be scheduled, and the thread will call Worker::processRequest
        when the data is queued */
    bool scheduleWork (uint32 size, const void* data);
    
    /** Respond from work (worker thread). Call this during processRequest if you
        need to send a response into the realtime thread. 
        @see processWorkResponses, @see processResponse */
    bool respondToWork (uint32 size, const void* data);
    
    /** Deliver pending responses (realtime thread)
        This must be called regularly from the realtime thread. For each read
        response, Worker::processResponse will be called */
    void processWorkResponses();
    
protected:
    
    /** Process work (worker thread) */
    virtual void processRequest (uint32 size, const void* data) = 0;
    
    /** Process work responses (realtime thread) */
    virtual void processResponse (uint32 size, const void* data) = 0;
    
private:
    
    WorkThread& owner;
    uint32 workId;                       ///< The thread assigned id for this worker
    Atomic<int> working;
    
    ScopedPointer<RingBuffer> responses; ///< responses from work
    HeapBlock<uint8>          response;  ///< buffer to write a response
    
    bool validateMessage (RingBuffer& ring);
    
    friend class WorkThread;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Worker)
};


#endif // LVTK_JUCE_WORKTHREAD_H
