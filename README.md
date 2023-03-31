# Space Invaders (C++ and Allegro5)

by: José Luis Aguilera Luzania

<p align="center">
  <img src="https://github.com/JoseLuis-AL/Cpp-Allegro5-Space-Invaders/blob/main/ReadmeResources/Logo.png?raw=true" alt="Logo"/>
</p>


This is a very simple Space Invaders clone, done in 3 days with C++ and Allegro5. The game is made as a simple practice in my free time.

The game contains a basic main screen, basic gameplay, and a basic GameOver screen, saves the highest recorded score and displays it during gameplay.

The game contains sounds, animations and score saves.

For the creation of the game, the following page was used as a reference for graphics, gameplay, sounds and information in general:

- [Play Guide for Space Invaders, the 1978 Taito Hit Arcade Game (classicgaming.cc)](http://www.classicgaming.cc/classics/space-invaders/play-guide)
- [Sound Effects and Music from Space Invaders the Classic Arcade Game (classicgaming.cc)](http://www.classicgaming.cc/classics/space-invaders/sounds)

### Concept
After reading about the history of Space Invaders, its gameplay and the limitations that the original game had, I created a concept using Aseprite to help me visualize what the finished game would look like.

![[Concept.png]]


### Prototype  
Initially I created a prototype with the basic elements, the graphics were simple rectangles with no sound and no movement except for the player to test collisions and points.


![[Prototype.png]]


### Final
After several iterations, the game contains all the basic elements of the original game, with the same limitations such as:
- Invaders can have up to three lasers on screen.
- Invaders increase their speed when others are destroyed.
- The player can only have one laser on screen at any time.
- Shields can resist multiple lasers.

![[Final.png]]


### Main menu
![[MainMenu.png]]


### Game Over
![[GameOver.png]]


## Adicional
In order to have a more consistent function that generates random numbers, a simpler syntax and to test an external library, the `random` library by [effolkronium](https://github.com/effolkronium) was used.
