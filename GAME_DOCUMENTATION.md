# BlitzMail 3D - Rural Level Game Documentation

## Overview
BlitzMail 3D is an OpenGL-based 3D game where you play as a mailman delivering packages to rural houses within time limits.

## Game Objective
- Pick up mail packages scattered around the rural environment
- Deliver packages to the correct houses before time runs out
- Earn points for faster deliveries
- Complete all deliveries without losing all lives to win

## Controls

### Movement
- **W** - Move forward
- **A** - Move left
- **S** - Move backward
- **D** - Move right
- **Space** - Jump
- **C** - Toggle crouch
- **Mouse** - Look around (mouse is hidden and captured)

### Camera
- **V** - Toggle between first-person and third-person camera views

### Game
- **ESC** - Exit game

## Gameplay Mechanics

### Package System
- Packages appear as brown boxes scattered around the level
- Walk close to a package to automatically pick it up
- Each package has a 30-second delivery timer
- Only one package can be carried at a time
- The carried package appears in front of your view

### Delivery System
- Each package must be delivered to a specific house
- Walk close to the target house to automatically deliver
- Successful delivery awards:
  - Base points: 100
  - Time bonus: Remaining seconds × 10
- After delivery, you can pick up the next package

### Lives System
- Start with 3 lives
- Lose 1 life when package timer reaches zero
- Red screen vignette flashes when losing a life
- Game over when all lives are lost

### Scoring
- Base delivery: 100 points
- Time bonus: Up to 300 points (30 seconds × 10)
- Maximum per delivery: 400 points
- Final score displayed on win/lose screens

## Game Environment

### Delivery Targets
- 5 rural houses positioned around the map
- Houses serve as delivery destinations

### Obstacles
- **Trees** - Block movement, must navigate around them
- **Rocks** - Scattered obstacles
- **Fences** - Barrier obstacles
- **Crops/Wheat** - Environmental decoration

### Lighting System
- **Sun** - Rotates around the scene, changing from yellow to orange (day to sunset)
- **Street Lamps** - Automatically light up when sun intensity is low
  - Slight flickering effect for realism
  - Turn red and flicker when package timeout is approaching (<10 seconds)

### Boundaries
- 50×50 unit play area
- Invisible walls prevent leaving the play area

## UI Elements

### HUD (Always Visible)
- **Score** - Current points (top left)
- **Lives** - Remaining lives (top left)
- **Deliveries** - Progress counter (e.g., "3/5" deliveries completed)
- **Timer** - Package delivery countdown (only when carrying a package)
  - Green/yellow when >10 seconds
  - Red when <10 seconds

### Game Over Screen
- Appears when all lives are lost
- Shows "GAME OVER" in red
- Displays final score
- View tilts toward sky during game over

### Win Screen
- Appears when all packages are delivered successfully
- Shows "YOU WIN!" in green
- Displays final score
- Player continues jumping in celebration

## Level Layout
- Central starting area (clear of obstacles)
- 5 houses distributed around the perimeter
- 5 packages randomly placed throughout the level
- 25 trees for obstacles and atmosphere
- 20 rocks scattered around
- 10 fence sections
- 30 crop patches for atmosphere
- 8 street lights along pathways

## Technical Features

### Camera System
- **First-person** (default):
  - Camera at player's eye level
  - Direct view of what player sees
  - Lower when crouching
  
- **Third-person**:
  - Camera positioned 5 units behind player
  - 2 units above player height
  - Always looks at player

### Collision Detection
- Checks for collisions with trees, rocks, and fences
- 0.5-unit radius around player
- Prevents passing through obstacles
- Different collision radii for different obstacle types

### Physics
- Gravity system for jumping
- Jump force: 0.3 units/frame
- Gravity acceleration: 0.02 units/frame²
- Ground height: 1.5 units (player eye level)

### Animation (Simulated)
- Package bobbing animation when on ground
- Door swing animation on delivery (prepared but not fully animated)
- Arm movement animation on pickup/delivery (prepared but not fully animated)
- Street light flickering

## Tips for Playing

1. **Time Management**: Pick up packages quickly and head straight to target houses
2. **Navigation**: Learn the house positions to minimize travel time
3. **Obstacles**: Plan routes around dense obstacle areas
4. **Time Bonus**: Deliver quickly to maximize score
5. **Street Lights**: Use red street lights as warning that time is running out
6. **Camera**: Switch to third-person view (V key) for better spatial awareness
7. **Jump**: Use jumping to see over obstacles and scout house locations

## Development Notes

### Models Used
- House: `models/house/house.3DS`
- Tree: `models/tree/Tree1.3ds`
- Ground Texture: `textures/ground.bmp`
- Sky Texture: `textures/blu-sky-3.bmp`

### Additional Models Available (Not Currently Used)
- Farmhouse: `models/4vd2sk31doow-farmhouse_maya16`
- Cottage: `models/cs5xpgzxg9vk-Cottage`
- Rock models: `models/1elmla01hh-Rock1_BYTyroSmith`
- Fence model: `models/6od9waw1za0w-fence`
- Street lamp: `models/s3duldjjt9fk-StreetLampByTyroSmith`
- Carrot/crops: `models/Carrot_v01_l3.123c059c383a-f43b-48c0-b28a-bec318013e17`
- Wheat: `models/10458_Wheat_Field_v1_L3.123c5ecd0518-ae16-4fee-bf80-4177de196237`
- Player model: `models/98-hikerbasemesh` (mailman character - not currently rendered)

## Building the Project

### Requirements
- Visual Studio 2022 or later
- OpenGL
- GLUT library
- GLAUX library (included)
- GLEW library (included)

### Build Instructions
1. Open `OpenGLMeshLoader.sln` in Visual Studio
2. Select Debug or Release configuration
3. Build Solution (F7)
4. Run (F5 or Ctrl+F5)

### Project Structure
```
BlitzMail2/
├── OpenGLMeshLoader.cpp    (Main game implementation)
├── Model_3DS.h/cpp          (3DS model loader)
├── GLTexture.h/cpp          (Texture loader)
├── TextureBuilder.h         (Texture utilities)
├── models/                  (3D model files)
├── textures/                (Texture files)
└── *.lib, *.dll            (OpenGL libraries)
```

## Future Enhancements (Potential)

1. **Audio System**: Add sound effects for pickup, delivery, collision, jump
2. **Mailman Model**: Render actual player character model
3. **Enhanced Models**: Use more detailed models for obstacles
4. **Particle Effects**: Add visual effects for deliveries
5. **Multiple Levels**: Add suburban and city levels
6. **Difficulty Progression**: Shorter timers, more packages
7. **Power-ups**: Speed boost, time extension
8. **Leaderboard**: Track high scores
9. **Minimap**: Show house and package locations
10. **Tutorial Mode**: Guided introduction for new players
