#include "../common/IConnectionClient.hpp"

#include <mpi.h>

class MPIRequest: public IRequest
{
    MPI::Request request;
public:
    MPIRequest(MPI::Request request)
    : request(request)
    {
    }
    
    bool test()
    {
        return request.Test();
    }
    
    void wait()
    {
        request.Wait();
    }
};

class MPIConnectionClient: public IConnectionClient
{
public:
    void init(int argc, char ** argv)
    {
        MPI::Init(argc, argv);
    }
    
    int get_num_threads()
    {
        return MPI::COMM_WORLD.Get_size();
    }
    
    int get_thread_id()
    {
        return MPI::COMM_WORLD.Get_rank();
    }
    
    void send(int * data, int size, int thread_id, int tag)
    {
        MPI::COMM_WORLD.Send(data, size, MPI::INT, thread_id, tag);
    }
    
    void recv(int * buf, int size, int thread_id, int tag)
    {
        MPI::COMM_WORLD.Recv(buf, size, MPI::INT, thread_id, tag);
    }
    
    IRequest * async_recv(int * buf, int size, int thread_id, int tag)
    {
        return new MPIRequest(MPI::COMM_WORLD.Irecv(buf, size, MPI::INT, thread_id, tag));
    }
    
    IRequest * async_send(int * buf, int size, int thread_id, int tag)
    {
        return new MPIRequest(MPI::COMM_WORLD.Isend(buf, size, MPI::INT, thread_id, tag));
    }
    
    void finalize()
    {
        MPI::Finalize();
    }
};
