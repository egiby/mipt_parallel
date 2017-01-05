#ifndef _ICONNECTION_CLIENT
#define _ICONNECTION_CLIENT

class IRequest
{
public:
    virtual bool test() = 0;
    virtual void wait() = 0;
};

class IConnectionClient
{
public:
    virtual void init(int argc, char ** argv) = 0;
    virtual int get_num_threads() = 0;
    virtual int get_thread_id() = 0;
    virtual void send(int * data, int size, int thread_id, int tag) = 0;
    virtual void recv(int * buf, int size, int thread_id, int tag) = 0;
    virtual IRequest * async_recv(int * buf, int size, int thread_id, int tag) = 0;
    virtual IRequest * async_send(int * buf, int size, int thread_id, int tag) = 0;
    
    virtual void finalize() = 0;
    
    virtual ~IConnectionClient()
    {
    }
};

#endif
