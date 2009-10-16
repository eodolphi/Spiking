#include "AssociativeNetwork.h"


int main(void)
{
    AssociativeNetwork *network = new AssociativeNetwork(100, 20,3,10,6);
    network->propegate(1000);
    delete network;

}


