#include <stdio.h>

#include "SocketsConnectionClient.hpp"

#include "../common/NetAlgorithm.hpp"

int main(int argc, char ** argv)
{
    IConnectionClient * connection = new SocketsConnectionClient();
    
    freopen("ans", "w", stdout);
    
    char filename[10];
    sprintf(filename, "logs/log%s", argv[3]);
    freopen(filename, "w", stderr);
    
    play(connection, argc, argv);
    //~ connection->init(argc, argv);
    delete connection;
    
    return 0;
}
