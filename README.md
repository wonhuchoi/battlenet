# Team-05 - BattleNet
**Narrative**

The player, a burgeoning computer virus finds themselves at the entrance to a portal into an unsuspecting PC. Their sole objective - get in and create the most damage possible. It’s a suicide mission.

**Intro**

The game starts with the player given a random generation of units that they can choose from. The player must then balance their resource - data - to either build their army or explore further to find new units. Certain units will combo with other units to provide special bonuses.

**Rounds**

From there, the actual game begins. The player will go through rounds of procedurally generated AI controlled enemies which get more difficult as the game goes on. The beginning of the round involves a planning phase where players see their next enemy and will be able to strategically position their units. Once the player is ready, the battle proceeds automatically (autobattler style). At the end of the round all units are reset.

To aid players with increased difficulty of levels, at the end of each round, various options will be given to the player to “salvage” from their most recent foe. These can range from one time bonuses for the next fight, unit specific bonuses (items), or even new units. Variety of options and their strengths will be given to players based on their success in the battle (based on the number of their units that survive).

**Conclusion**

The game ends when the player loses a battle. The player's goal is to attain the highest score possible.  Degree of success in rounds will be used to calculate scores at the end of the game. This will give the player the interesting option of choosing whether to “econ” by choosing to increase their per round score, or focus on even beating the next round. We will give players the option to pick units that are either stronger in battle or that count as stronger in score calculation.

**Technical Elements**
* Overall 2D Geometry
  * The game will be played on a two dimensional grid.
* Controllable Units
  * The Player will be able to place and move their units around the map.
  * Unit placement as well as the auto battle simulation will happen on the 2D grid, requiring basic 2D transformations of the units.
* AI Logic for Player and Enemy Units
  * As battles are simulated automatically after unit placement, we will need to implement deterministic pathfinding and logic algorithms for all units during battle.
* Distinct Game Phases
  * The game will have multiple states: menu, unit setup, battles, and an ending screen. Each will have their own properties and will require different input from the user.
* Possible Player Movement
  * If required, player movement can be implemented with a controllable unit that buffs adjacent allies.
* Geometric Art Style / Sprites
  * Assets will be rendered in 8-bit or similar aesthetics to emulate the “hacking” nature of the game
* Background Music
  * If open source soundtracks can be acquired, BGM will be implemented.
* Dynamic Unit Appearance
  * Salvaged Items after Battles can be used to upgrade units, altering their appearance as a bonus. Ex. Opacity, Color, Gradient, Border

**Advanced Technical Elements**
* Pausing
  * The player may pause the game and resume play as needed. Entities will remain in place and logic will be halted until the game is resumed.
  * Alternative: games cannot be paused and the player will have to continue playing until the end screen
* Particle effects
  * Enhance the user experience with particle effects when the AI / user attacks a unit, certain items are purchased, etc.
  * Alternative: no/minimal visual effects, less polished	
* Networked or Local Multiplayer (Future iterations if time allows)
  * Autobattlers play best when you can play with friends so, if we have the time, we would love to be able to implement networked multiplayer!
  * Alternative: single player only, cannot play with others
* Saving + Loading
  * The player can save the game and return to the saved game state at any time. 
  * Alternative: the player will be unable to save / load a saved game and will start fresh every time

**Devices**

Mouse and Keyboard: Click on units and items to acquire them. Drag units or items onto the board to place them.
 
# Development (Mac/Linux)

### Build
From top level directory
```
cmake . && make
```

### Run
From top level directory
```
./build/BattleNet
```

### VSCode Debug
Make sure you have gdb installed and add this launch config to your .vscode folder:
```
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "Debug",
            "type": "gdb",
            "request": "launch",
            "target": "./build/BattleNet",
            "cwd": "${workspaceRoot}",
            "valuesFormatting": "parseText"
        }
    ]
}
```
Then press `f5` to debug your latest build. (**Doesn't auto-build for you, `f7` will auto build if you have cmake set up properly in vscode!**)
