#include <stdio.h>

#include "SocketsConnectionClient.hpp"

#include "../common/NetAlgorithm.hpp"

int main(int argc, char ** argv)
{
    IConnectionClient * connection = new SocketsConnectionClient();
    
    freopen("ans", "w", stdout);
    if (atoi(argv[3]) == 0)
        freopen("log", "w", stderr);
    play(connection, argc, argv);
    //~ connection->init(argc, argv);
    delete connection;
    
    return 0;
}
