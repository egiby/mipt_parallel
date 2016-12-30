#ifndef _CONNECTION_CLIENT_FACTORY
#define _CONNECTION_CLIENT_FACTORY

#include <string>

#include "../mpi/MPIConnectionClient.hpp"
//~ #include "../sockets/SocketsConnectionClient.hpp"

IConnectionClient * get_client(std::string name)
{
    if (name == "mpi")
        return new MPIConnectionClient();
    //~ if (name == "sockets")
        //~ return new SocketsConnectionClient();
    
    return 0;
}

#endif
