  // Copyright 2014 GordonLee

#include "network/network.h"
int main() {
    Network network;
    network.Initialize();

	network.Run();
    // network.RunWithAcceptEx();

	/*
	// MEMO: 이거보단 위에 방법이 좋겠지? 똑같나..
    while ( network.RunAcceptThread() ) {
    }
	*/

    network.Cleanup();
    return 0;
}
