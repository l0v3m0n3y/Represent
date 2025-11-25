# Represent
api for represent.opennorth.ca Find the elected officials and electoral districts for any Canadian address or postal code, at all levels of government
# main
```cpp
#include "Represent.h"
#include <iostream>

int main() {
   Represent api;
    auto elections = api.get_elections().then([](json::value result) {
        std::cout << result<< std::endl;
    });
    elections.wait();
    
    return 0;
}
```

# Launch (your script)
```
g++ -std=c++11 -o main main.cpp -lcpprest -lssl -lcrypto -lpthread -lboost_system -lboost_chrono -lboost_thread
./main
```
