# Asteroids Network
Repository of Network game. The classic asteroids for you to play with your friends cooperatively online. Only for two players.

## Authors
* [Alex Morales](https://github.com/AlexMG99)
* [Laia Martinez](https://github.com/LaiaMartinezMotis)
* [Alejandro Paris](https://github.com/AlejandroParis)

## Tutorial/Instructions
This game has a maxim number of 2 players.
There are two types of spaceships, each one of them has diferent stats.

 **- Speed Ship:** 
Advanced Speed ++
Rotation Speed ++
Life - - 

 
  **- Tank Ship:** 
Advanced Speed - -
Rotation Speed - -
Life + +

### Gameplay
Double click on the Networks.exe.
When the window shows up, press "Start Server", to connect with the server.
Then doble click on the Networks.exe, another window will show. Now you should put a name where it puts "Player Name", and the click on "Connect to Server".
Then the two players will be connected on the same server.

Press Start. 
A meteorite will show up. Your main goal will be to destroy it without dying.
Be Careful!!! When you shoot a big meteorite, will be divided into smaller meteorites.

If you die, you have to press the *respawn* button in order to keep playing. You will find the button on the top of the screen.


 ### Controls
* __[Down arrow]:__ Move Straight
* __[Left arrow]:__ Shoot
* __[D]:__ Right Rotation
* __[A]:__ Left Rotation


## Features
**UDP virtual connection** (Alex Morales, Alejandro París, Laia Martínez) Completely achieved.
- Module Networking Client and Server, Starting the practice by disconnecting clients that 
did not send packets (ping) in a certain time did not give problems to implement it.

**World state Replication** (Alex Morales, Alejandro París) Completely achieved.
- The server sends different types of commands (create, update, destroy) each time that generates, modificates or eliminates an object. The the client replicates the same commands on his/her world.

- With the world state replication we had some problems to understand correctly how it works.

- In this part an important bug arises in which the ship when firing disappeared. We solved this bug by detecting the problem of the code in which we always eliminated the first object in the list that contains all 
the objects in the game, therefore, when we try to eliminate the laser, the ship was eliminated, the first 
object that was created, the first on the list.



**Reliability on top of UDP** (Alex Morales, Alejandro París, Laia Martínez) Completely achieved.
- In a real situation where a packet information may be lost, it ensures that packets are shipped correctly. It also has them processed in the correct order.

- At this point, after implementing everything, we had no way of checking if our implementations were working correctly when working from home.

- We did not know how to correctly implement the delegate on success and on failure functions. Luckily, the teacher solved it in class the next day.

- When disconnecting and reconnecting, we could not get the delivery manager to clean properly. To solve this problem we create a new clear function to correctly clean the delivery manager.

**Improving latency handling**
- **Client side prediction** (Alex Morales)  Completely achieved.

It processes inputs in real time from the player and when we receive the package with the position it should be in and the input it stayed in, we undo these inputs from the client and apply those from the server, to have the same position in both. 
A problem that arose in this section was that the inputdatafrom gave us a negative value. We solved it with a check that prevents negative values ​​from being processed.
- **Entity interpolation** (Alex Morales, Alejandro París, Laia Martínez) Completely achieved.

-From the client's point it interpolates the position, starting from the one received in the previous packet and the one received in the current one. In order to make the movement feel better. 
The problem we found in this section was the ignorance of the existence of the variable has interpolatedenable
and therefore in the client we did not pass the position correctly and in the game it was out of square.
To solve this problem we asked colleagues and when we discovered the variable we used it so that everything worked well. 