# BlitzMail 3D - Rural Level Game

![OpenGL](https://img.shields.io/badge/OpenGL-Legacy-blue)
![C++](https://img.shields.io/badge/C++-11-blue)
![Platform](https://img.shields.io/badge/Platform-Windows-lightgrey)
![Status](https://img.shields.io/badge/Status-Complete-green)

A 3D mailman delivery game built with OpenGL and GLUT. Navigate a rural environment, pick up packages, and deliver them to houses before time runs out!

## 🎮 Game Overview

You play as a mailman delivering packages to rural houses. Each package has a 30-second delivery timer. Deliver packages quickly to maximize your score, but don't let the timer run out or you'll lose a life!

### Features
- 🏃 **Full 3D Movement** - WASD controls with jumping and crouching
- 📦 **Package Delivery System** - Pick up and deliver 5 packages
- 🏠 **Rural Environment** - Houses, trees, rocks, fences, and crops
- ☀️ **Dynamic Lighting** - Rotating sun with day-to-sunset transitions
- 💡 **Street Lamps** - Auto-activate at night with realistic flickering
- ⚠️ **Warning System** - Red lights when time is running out
- 🎯 **Scoring System** - Base points + time bonus for fast deliveries
- 💖 **Lives System** - 3 lives, lose one when timer expires
- 🎬 **Camera Modes** - Toggle between first-person and third-person views
- 🎪 **Win/Lose Screens** - Game over and victory displays with final scores

## 🎯 Quick Start

### Controls
| Key | Action |
|-----|--------|
| **W/A/S/D** | Move forward/left/backward/right |
| **Mouse** | Look around (cursor hidden) |
| **Space** | Jump |
| **C** | Toggle crouch |
| **V** | Toggle camera view (FPS/TPS) |
| **ESC** | Exit game |

### Objective
1. Walk near brown packages to pick them up
2. Deliver each package to a house within 30 seconds
3. Earn bonus points for faster deliveries
4. Complete all 5 deliveries without losing all 3 lives
5. Achieve the highest score possible!

## 🛠️ Building & Running

### Prerequisites
- **Visual Studio 2022** (or later)
- **Windows OS**
- **OpenGL** (included with Windows)
- **GLUT** (FreeGLUT library - included)

### Build Instructions
1. Clone this repository
2. Open `OpenGLMeshLoader.sln` in Visual Studio
3. Select **Debug** or **Release** configuration
4. Build the solution (**F7**)
5. Run the game (**F5** or **Ctrl+F5**)

### Project Structure
```
BlitzMail2/
├── OpenGLMeshLoader.cpp      # Main game implementation
├── Model_3DS.h/cpp            # 3DS model loader
├── GLTexture.h/cpp            # Texture management
├── TextureBuilder.h           # Texture utilities
├── models/                    # 3D model files
│   ├── house/                # House models
│   ├── tree/                 # Tree models
│   └── ...                   # Other models
├── textures/                  # Texture files
│   ├── ground.bmp            # Grass texture
│   └── blu-sky-3.bmp         # Sky texture
├── *.lib, *.dll              # OpenGL libraries
└── Documentation files
```

## 📚 Documentation

- **[QUICK_REFERENCE.md](QUICK_REFERENCE.md)** - Quick controls and gameplay guide
- **[GAME_DOCUMENTATION.md](GAME_DOCUMENTATION.md)** - Comprehensive game guide
- **[IMPLEMENTATION_SUMMARY.md](IMPLEMENTATION_SUMMARY.md)** - Technical implementation details

## 🎨 Gameplay Mechanics

### Package System
- **Collection**: Walk within 2 units of a package to pick it up
- **Timer**: 30 seconds to deliver each package
- **Warning**: Timer turns red and street lights flash when < 10 seconds
- **Delivery**: Walk within 3 units of target house to deliver

### Scoring
- **Base Points**: 100 per successful delivery
- **Time Bonus**: Remaining seconds × 10 (up to 300 points)
- **Maximum**: 400 points per delivery
- **Total Possible**: 2,000 points (5 perfect deliveries)

### Lives
- **Starting Lives**: 3 ❤️❤️❤️
- **Life Loss**: When package timer reaches zero
- **Game Over**: All lives lost
- **Visual Feedback**: Red screen flash when losing a life

### Environment
- **Play Area**: 50×50 units
- **Houses**: 5 delivery destinations
- **Packages**: 5 randomly placed collectibles
- **Obstacles**: 25 trees, 20 rocks, 10 fences
- **Decorations**: 30 crop patches, 8 street lights
- **Sky**: Full spherical skybox

## 🌟 Features in Detail

### Dynamic Lighting System
The game features a sophisticated lighting system:
- **Sun**: Rotates around the scene, transitioning from day to sunset
- **Color Shift**: Changes from bright yellow to warm orange
- **Street Lamps**: Automatically illuminate when sun intensity drops
- **Flickering**: Realistic light flickering using sine wave animation
- **Warning Mode**: Lamps turn red when package timeout is imminent

### Camera System
Two camera modes for different playstyles:
- **First Person** (default): Experience the world through the mailman's eyes
- **Third Person**: View from behind and above for better spatial awareness
- **Smooth Transitions**: Seamless switching between modes
- **Mouse Look**: Full 360° rotation with pitch limits (±89°)

### Physics & Collision
- **Gravity System**: Realistic falling and jumping
- **Collision Detection**: Prevents walking through obstacles
- **Jump Mechanics**: Space bar for jumping with proper physics
- **Boundaries**: Invisible walls keep player within play area

## 🎓 Tips & Strategies

1. **Explore First**: Before picking up packages, scout house locations
2. **Plan Routes**: Plan efficient paths between packages and houses
3. **Use Third Person**: Press V for better navigation and awareness
4. **Watch Street Lights**: They warn you when time is running low
5. **Avoid Obstacles**: Trees and rocks block your path - plan around them
6. **Jump for Vision**: Use jumping to see over obstacles
7. **Time Management**: Faster deliveries = higher scores

## 🔧 Technical Details

### Performance
- **Frame Rate**: ~60 FPS (16ms update cycle)
- **Rendering**: OpenGL fixed pipeline
- **Lighting**: Dynamic directional light + point lights
- **Collision**: Radius-based detection (optimized with lengthSquared)

### Code Quality
- ✅ Code review completed - all issues resolved
- ✅ Security scan passed - no vulnerabilities detected
- ✅ Proper buffer management (sprintf_s with sizes)
- ✅ Correct floating-point operations (fabs instead of abs)
- ✅ Performance optimizations applied

### Dependencies
- **GLUT** (freeglut3.dll) - Window management and input
- **OpenGL** (opengl32.dll) - Graphics rendering
- **GLU** (glu32.dll) - OpenGL utilities
- **GLAUX** (glaux.dll) - Texture loading
- **GLEW** (glew32.dll) - OpenGL extensions

## 🚀 Future Enhancements

Potential features for future versions:
- 🔊 **Audio System**: Sound effects and background music
- 🧑 **Player Model**: Render the mailman character
- 🌍 **More Levels**: Suburban and city environments
- 🏆 **Leaderboards**: High score tracking
- 🎯 **Power-ups**: Speed boosts, time extensions
- 🗺️ **Minimap**: Overhead view of objectives
- 🌦️ **Weather**: Rain, fog, and dynamic weather
- 🎮 **Gamepad Support**: Controller input

## 📝 License

This is a educational/demonstration project for learning OpenGL and 3D game development.

## 🤝 Contributing

This project was created as a complete implementation of a game specification. Feel free to fork and modify for your own learning purposes!

## 📧 Support

For issues or questions, please refer to:
- **Game Guide**: [GAME_DOCUMENTATION.md](GAME_DOCUMENTATION.md)
- **Technical Details**: [IMPLEMENTATION_SUMMARY.md](IMPLEMENTATION_SUMMARY.md)
- **Quick Reference**: [QUICK_REFERENCE.md](QUICK_REFERENCE.md)

---

**Made with ❤️ using OpenGL and C++**

*Deliver fast, score high, and don't run out of time!* 📬🏃💨
