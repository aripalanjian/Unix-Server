# Systems and Networks II

## Project 2

Contributors: Ari Palanjian

* Note: This program was developed using WSL and tested using a second WSL terminal instance.

## Included Files

- assets
    - screencaps
        - 404notfound.png
        - clientclose.png
        - clientmessage.png
        - exrun.png
        - homepage.png
        - picture.png
    - img.jpg
- templates
    - index.html
    - testPresence.html
- client.cpp
- client.hpp
- main.cpp
- Makefile
- protocol.pdf
- README.md
- server.cpp
- server.hpp

## How to Run
1. Compile program by entering ```make``` in CLI in the same directory as main.
2. Execute the server by entering the command ```./server <portno>```.
3. Execute the client by entering the command ```./client <portno>```.

## Usage
- Upon starting the client you will be presented with the available options
- When finished enter the command exit to leave the client
- In the terminal where the server is running press the keys Ctrl and C

## Example Run

![Starting Server and Client](assets/screencaps/sampleOutput1.png?raw=true)

- Starting Server and Client,running ls command, and client message to server

![Sample Get Request Output](assets/screencaps/sampleOutputGet.png?raw=true)

- Sample get request for index.html

![Sample Incorrect Start and Exit command](assets/screencaps/sampleOutputExit.png?raw=true)

- Sample run if client/server started in incorrect order, and use of exit command

