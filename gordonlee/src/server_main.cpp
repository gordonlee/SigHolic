
#include <iostream>
#include <list>

#include "network\network_interface.h"

int main() {

    // create network class
    INetwork* network_object = new Network();
    network_object->Initialize(IOModel::DEFAULT);
    
    ::WaitForSingleObject(network_object->get_thread_handle(), INFINITE);

    network_object->Clean();

    return 0;
}