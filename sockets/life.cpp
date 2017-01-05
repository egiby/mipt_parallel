#include <stdio.h>

#include "SocketsConnectionClient.hpp"

#include "../common/NetAlgorithm.hpp"

int main(int argc, char ** argv)
{
    IConnectionClient * connection = new SocketsConnectionClient();
    
    play(connection, argc, argv);
    //~ connection->init(argc, argv);
    delete connection;
    
    return 0;
}
