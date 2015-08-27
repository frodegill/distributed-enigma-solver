# distributed-enigma-solver
A small client/server-application I'm making for solving the geocache GC1J548

To clean:
./clean.sh

To compile:
./make.sh

To run server:
./server/enigma-solver-server files/english_words.txt files/encrypted.txt

To run clients (preferably one per CPU core):
./client/enigma-solver-client <server IP>
