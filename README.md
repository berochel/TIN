# miniTorrent

A P2P file sharing application

## Compilation and execution

`g++ -std=c++17 -o tracker tracker.cpp`
`g++ -std=c++17 -o client tracker.cpp`

`./server tracker_info.txt 1`
`./client 127.0.0.1 2217 tracker_info.txt` - Change IP and Port for each client.
