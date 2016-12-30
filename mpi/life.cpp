#include <stdio.h>

#include "../common/ConnectionClientFactory.hpp"

#include "../common/NetAlgorithm.hpp"

int main(int argc, char ** argv)
{
    IConnectionClient * connection = get_client("mpi");
    
    play(connection, argc, argv);
    delete connection;
    
    return 0;
}
