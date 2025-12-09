#include "TextureBuilder.h"
#include "Model_3DS.h"
#include "GLTexture.h"

#include <stdlib.h>        // ← MUST BE BEFORE glut.h
#include <cmath>
#include <vector>
#include <ctime>
#include <string>

#include <glut.h>          

#ifdef exit
#undef exit
#endif
int WIDTH = 1280;
int HEIGHT = 720;

GLuint tex_sky;
char title[] = "BlitzMail 3D - Rural Level";

// 3D Projection Options
GLdouble fovy = 45.0;
GLdouble aspectRatio = (GLdouble)WIDTH / (GLdouble)HEIGHT;
GLdouble zNear = 0.1;
GLdouble zFar = 500;

// Vector class
class Vector3
{
public:
	GLdouble x, y, z;
	Vector3() : x(0), y(0), z(0) {}
	Vector3(GLdouble _x, GLdouble _y, GLdouble _z) : x(_x), y(_y), z(_z) {}
	
	void operator +=(float value) {
		x += value; y += value; z += value;
	}
	
	Vector3 operator+(const Vector3& v) const {
		return Vector3(x + v.x, y + v.y, z + v.z);
	}
	
	Vector3 operator-(const Vector3& v) const {
		return Vector3(x - v.x, y - v.y, z - v.z);
	}
	
	Vector3 operator*(float scalar) const {
		return Vector3(x * scalar, y * scalar, z * scalar);
	}
	
	float length() const {
		return sqrt(x*x + y*y + z*z);
	}
	
	float lengthSquared() const {
		return x*x + y*y + z*z;
	}
	
	Vector3 normalize() const {
		float len = length();
		if(len > 0) return Vector3(x/len, y/len, z/len);
		return *this;
	}
};

// Game States
enum GameState {
	PLAYING,
	GAME_OVER,
	WIN
};

// Game variables
GameState gameState = PLAYING;
int playerScore = 0;
int playerLives = 3;
float packageTimer = 30.0f;
bool hasPackage = false;
int totalPackages = 5;
int deliveredPackages = 0;

// Player variables
Vector3 playerPos(0, 1.5f, 0);
Vector3 playerVelocity(0, 0, 0);
float playerYaw = 0.0f;
float playerPitch = 0.0f;
float playerSpeed = 0.15f;
float jumpForce = 0.3f;
bool isJumping = false;
bool isCrouching = false;
bool isFirstPerson = true;

// Camera
Vector3 cameraPos;
Vector3 cameraLookAt;
Vector3 cameraUp(0, 1, 0);

// Mouse control
int lastMouseX = WIDTH / 2;
int lastMouseY = HEIGHT / 2;
bool mouseInitialized = false;

// Keyboard state
bool keys[256] = {false};

// Animation timers
float armAnimTime = 0.0f;
float doorAnimTime = 0.0f;
float gameOverAnimTime = 0.0f;
float winAnimTime = 0.0f;
float vignetteIntensity = 0.0f;

// Sun/Lighting
float sunAngle = 0.0f;
float sunRotationSpeed = 0.005f;

// Models
Model_3DS model_house;
Model_3DS model_tree;
Model_3DS model_farmhouse;
Model_3DS model_cottage;
Model_3DS model_fence;
Model_3DS model_rock;
Model_3DS model_streetlamp;
Model_3DS model_tree2;

// Textures
GLTexture tex_ground;

// Game object structures
struct GameObject {
	Vector3 position;
	float rotation;
	float scale;
	bool active;
	
	GameObject() : position(0,0,0), rotation(0), scale(1), active(true) {}
	GameObject(float x, float y, float z, float r = 0, float s = 1) 
		: position(x,y,z), rotation(r), scale(s), active(true) {}
};

struct Package : public GameObject {
	int targetHouseIndex;
	bool collected;
	
	Package() : GameObject(), targetHouseIndex(0), collected(false) {}
	Package(float x, float y, float z) 
		: GameObject(x, y, z, 0, 0.5f), targetHouseIndex(0), collected(false) {}
};

struct House : public GameObject {
	bool hasDelivered;
	float doorAngle;
	
	House() : GameObject(), hasDelivered(false), doorAngle(0) {}
	House(float x, float y, float z, float r = 0) 
		: GameObject(x, y, z, r, 1.0f), hasDelivered(false), doorAngle(0) {}
};

struct StreetLight : public GameObject {
	float lightIntensity;
	float flickerTimer;
	
	StreetLight() : GameObject(), lightIntensity(0), flickerTimer(0) {}
	StreetLight(float x, float y, float z) 
		: GameObject(x, y, z, 0, 1.0f), lightIntensity(0), flickerTimer(0) {}
};

// Game object collections
std::vector<House> houses;
std::vector<Package> packages;
std::vector<GameObject> trees;
std::vector<GameObject> rocks;
std::vector<GameObject> fences;
std::vector<GameObject> crops;
std::vector<StreetLight> streetLights;

int currentPackageIndex = 0;

//=======================================================================
// Function Forward Declarations
//=======================================================================
void RenderPackage(float x, float y, float z, float scale);
void RenderPlayer(float x, float y, float z, float rotation);

//=======================================================================
// Collision Detection
//=======================================================================
bool CheckCollision(Vector3 pos, float radius) {
	// Check collision with obstacles
	for(size_t i = 0; i < trees.size(); i++) {
		Vector3 diff = pos - trees[i].position;
		if(diff.length() < (radius + 1.5f)) return true;
	}
	for(size_t i = 0; i < rocks.size(); i++) {
		Vector3 diff = pos - rocks[i].position;
		if(diff.length() < (radius + 1.0f)) return true;
	}
	for(size_t i = 0; i < fences.size(); i++) {
		Vector3 diff = pos - fences[i].position;
		if(diff.length() < (radius + 1.5f)) return true;
	}
	return false;
}

//=======================================================================
// Package Pickup Check
//=======================================================================
void CheckPackagePickup() {
	if(hasPackage || gameState != PLAYING) return;
	
	for(size_t i = 0; i < packages.size(); i++) {
		if(!packages[i].collected) {
			Vector3 diff = playerPos - packages[i].position;
			if(diff.length() < 2.0f) {
				packages[i].collected = true;
				hasPackage = true;
				currentPackageIndex = i;
				packageTimer = 30.0f;
				armAnimTime = 0.0f;
				// Play pickup sound (simulated)
				break;
			}
		}
	}
}

//=======================================================================
// Package Delivery Check
//=======================================================================
void CheckPackageDelivery() {
	if(!hasPackage || gameState != PLAYING) return;
	
	if(currentPackageIndex >= 0 && currentPackageIndex < (int)packages.size()) {
		int targetHouse = packages[currentPackageIndex].targetHouseIndex;
		if(targetHouse >= 0 && targetHouse < (int)houses.size()) {
			Vector3 diff = playerPos - houses[targetHouse].position;
			if(diff.length() < 3.0f) {
				// Deliver package
				hasPackage = false;
				houses[targetHouse].hasDelivered = true;
				houses[targetHouse].doorAngle = 0.0f;
				doorAnimTime = 0.0f;
				deliveredPackages++;
				
				// Calculate score based on time remaining
				int timeBonus = (int)(packageTimer * 10);
				playerScore += 100 + timeBonus;
				
				armAnimTime = 0.0f;
				
				// Check win condition
				if(deliveredPackages >= totalPackages) {
					gameState = WIN;
					winAnimTime = 0.0f;
				}
			}
		}
	}
}

//=======================================================================
// Update Game State
//=======================================================================
void UpdateGameState(float deltaTime) {
	if(gameState == PLAYING) {
		// Update package timer
		if(hasPackage) {
			packageTimer -= deltaTime;
			if(packageTimer <= 0) {
				packageTimer = 0;
				hasPackage = false;
				playerLives--;
				vignetteIntensity = 1.0f;
				
				if(playerLives <= 0) {
					gameState = GAME_OVER;
					gameOverAnimTime = 0.0f;
				}
			}
		}
		
		// Update vignette fade
		if(vignetteIntensity > 0) {
			vignetteIntensity -= deltaTime * 2.0f;
			if(vignetteIntensity < 0) vignetteIntensity = 0;
		}
		
		// Update door animation
		if(doorAnimTime < 1.0f) {
			doorAnimTime += deltaTime;
		}
		
		// Update arm animation
		if(armAnimTime < 1.0f) {
			armAnimTime += deltaTime;
		}
	}
	else if(gameState == GAME_OVER) {
		gameOverAnimTime += deltaTime;
	}
	else if(gameState == WIN) {
		winAnimTime += deltaTime;
	}
}

//=======================================================================
// Lighting Configuration Function
//=======================================================================
void UpdateLighting()
{
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	// Calculate sun position
	float sunX = cos(sunAngle) * 100.0f;
	float sunY = sin(sunAngle) * 100.0f;
	float sunZ = 0.0f;
	
	// Sun intensity based on angle
	float sunIntensity = (sin(sunAngle) + 1.0f) * 0.5f;
	if(sunIntensity < 0.1f) sunIntensity = 0.1f;
	
	// Sun color changes from yellow to orange
	float sunRed = 0.9f + 0.1f * (1.0f - sunIntensity);
	float sunGreen = 0.7f * sunIntensity;
	float sunBlue = 0.3f * sunIntensity;
	
	GLfloat light_ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
	GLfloat light_diffuse[] = { sunRed * sunIntensity, sunGreen * sunIntensity, sunBlue * sunIntensity, 1.0f };
	GLfloat light_specular[] = { 0.5f, 0.5f, 0.5f, 1.0f };
	GLfloat light_position[] = { sunX, sunY, sunZ, 0.0f };
	
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	
	// Update street lights
	bool streetLightsOn = sunIntensity < 0.4f;
	for(size_t i = 0; i < streetLights.size(); i++) {
		streetLights[i].flickerTimer += 0.016f;
		if(streetLightsOn) {
			float flicker = sin(streetLights[i].flickerTimer * 20.0f) * 0.1f + 0.9f;
			streetLights[i].lightIntensity = flicker;
		} else {
			streetLights[i].lightIntensity = 0.0f;
		}
	}
}

//=======================================================================
// Material Configuration Function
//======================================================================
void InitMaterial()
{
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
	
	GLfloat specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
	
	GLfloat shininess[] = { 96.0f };
	glMaterialfv(GL_FRONT, GL_SHININESS, shininess);
}

//=======================================================================
// Update Camera
//=======================================================================
void UpdateCamera()
{
	if(isFirstPerson) {
		// First person camera
		float lookX = sin(playerYaw * 3.14159f / 180.0f);
		float lookZ = cos(playerYaw * 3.14159f / 180.0f);
		float lookY = sin(playerPitch * 3.14159f / 180.0f);
		
		cameraPos = playerPos;
		cameraPos.y += isCrouching ? 0.5f : 1.5f;
		
		cameraLookAt = cameraPos;
		cameraLookAt.x += lookX;
		cameraLookAt.y += lookY;
		cameraLookAt.z += lookZ;
	}
	else {
		// Third person camera
		float distance = 5.0f;
		float height = 2.0f;
		
		float camX = playerPos.x - sin(playerYaw * 3.14159f / 180.0f) * distance;
		float camZ = playerPos.z - cos(playerYaw * 3.14159f / 180.0f) * distance;
		float camY = playerPos.y + height;
		
		cameraPos = Vector3(camX, camY, camZ);
		cameraLookAt = playerPos;
		cameraLookAt.y += 1.5f;
	}
}

//=======================================================================
// Update Player
//=======================================================================
void UpdatePlayer(float deltaTime)
{
	if(gameState != PLAYING) return;
	
	// Movement
	Vector3 forward(sin(playerYaw * 3.14159f / 180.0f), 0, cos(playerYaw * 3.14159f / 180.0f));
	Vector3 right(sin((playerYaw + 90) * 3.14159f / 180.0f), 0, cos((playerYaw + 90) * 3.14159f / 180.0f));
	
	Vector3 movement(0, 0, 0);
	
	if(keys['w'] || keys['W']) movement = movement + forward * playerSpeed;
	if(keys['s'] || keys['S']) movement = movement + forward * (-playerSpeed);
	if(keys['a'] || keys['A']) movement = movement + right * (-playerSpeed);
	if(keys['d'] || keys['D']) movement = movement + right * playerSpeed;
	
	// Try to move
	Vector3 newPos = playerPos + movement;
	if(!CheckCollision(newPos, 0.5f)) {
		playerPos = newPos;
	}
	
	// Gravity
	if(!isJumping && playerPos.y > 1.5f) {
		playerVelocity.y -= 0.02f;
	}
	else if(isJumping) {
		playerVelocity.y -= 0.02f;
		if(playerPos.y <= 1.5f) {
			playerPos.y = 1.5f;
			playerVelocity.y = 0;
			isJumping = false;
		}
	}
	
	playerPos.y += playerVelocity.y;
	if(playerPos.y < 1.5f) {
		playerPos.y = 1.5f;
		playerVelocity.y = 0;
	}
	
	// Check boundaries
	float boundary = 90.0f;
	if(playerPos.x < -boundary) playerPos.x = -boundary;
	if(playerPos.x > boundary) playerPos.x = boundary;
	if(playerPos.z < -boundary) playerPos.z = -boundary;
	if(playerPos.z > boundary) playerPos.z = boundary;
	
	// Check interactions
	CheckPackagePickup();
	CheckPackageDelivery();
}

//=======================================================================
// Render Ground Function
//=======================================================================
void RenderGround()
{
	glDisable(GL_LIGHTING);
	glColor3f(0.7f, 0.7f, 0.7f);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, tex_ground.texture[0]);

	glPushMatrix();
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);
	glTexCoord2f(0, 0);
	glVertex3f(-100, 0, -100);
	glTexCoord2f(20, 0);
	glVertex3f(100, 0, -100);
	glTexCoord2f(20, 20);
	glVertex3f(100, 0, 100);
	glTexCoord2f(0, 20);
	glVertex3f(-100, 0, 100);
	glEnd();
	glPopMatrix();

	glEnable(GL_LIGHTING);
	glColor3f(1, 1, 1);
}

//=======================================================================
// Render Sky
//=======================================================================
void RenderSky()
{
	glPushMatrix();
	glDisable(GL_LIGHTING);
	
	GLUquadricObj* qobj = gluNewQuadric();
	glTranslated(playerPos.x, playerPos.y, playerPos.z);
	glRotated(90, 1, 0, 1);
	glBindTexture(GL_TEXTURE_2D, tex_sky);
	gluQuadricTexture(qobj, true);
	gluQuadricNormals(qobj, GL_SMOOTH);
	gluSphere(qobj, 200, 100, 100);
	gluDeleteQuadric(qobj);
	
	glEnable(GL_LIGHTING);
	glPopMatrix();
}

//=======================================================================
// Render Package
//=======================================================================
void RenderPackage(float x, float y, float z, float scale)
{
	glPushMatrix();
	glTranslatef(x, y, z);
	glScalef(scale, scale, scale);
	
	// Package cube is 1.0 unit, tape positioned slightly beyond surface to avoid z-fighting
	const float TAPE_OFFSET = 0.51f;
	
	// Simple box for package with cardboard color
	glColor3f(0.8f, 0.6f, 0.4f);
	glutSolidCube(1.0f);
	
	// Add tape stripes for better appearance
	glColor3f(0.9f, 0.85f, 0.7f);
	glPushMatrix();
	glTranslatef(0, 0, TAPE_OFFSET);
	glScalef(0.2f, 1.0f, 0.02f);
	glutSolidCube(1.0f);
	glPopMatrix();
	
	glPushMatrix();
	glTranslatef(0, TAPE_OFFSET, 0);
	glScalef(1.0f, 0.02f, 0.2f);
	glutSolidCube(1.0f);
	glPopMatrix();
	
	glPopMatrix();
	glColor3f(1.0f, 1.0f, 1.0f);
}

//=======================================================================
// Render Player Character (Mailman)
//=======================================================================
void RenderPlayer(float x, float y, float z, float rotation)
{
	glPushMatrix();
	glTranslatef(x, y, z);
	glRotatef(rotation, 0, 1, 0);
	
	// Body (torso)
	glColor3f(0.2f, 0.3f, 0.6f); // Blue uniform
	glPushMatrix();
	glTranslatef(0, 0, 0);
	glScalef(0.4f, 0.6f, 0.3f);
	glutSolidCube(1.0f);
	glPopMatrix();
	
	// Head
	glColor3f(0.9f, 0.7f, 0.6f); // Skin tone
	glPushMatrix();
	glTranslatef(0, 0.5f, 0);
	glutSolidSphere(0.2f, 16, 16);
	glPopMatrix();
	
	// Cap
	glColor3f(0.2f, 0.3f, 0.6f);
	glPushMatrix();
	glTranslatef(0, 0.65f, 0);
	glScalef(1.0f, 0.3f, 1.0f);
	glutSolidSphere(0.2f, 16, 16);
	glPopMatrix();
	
	// Left arm
	glColor3f(0.2f, 0.3f, 0.6f);
	glPushMatrix();
	glTranslatef(-0.3f, -0.1f, 0);
	glScalef(0.12f, 0.5f, 0.12f);
	glutSolidCube(1.0f);
	glPopMatrix();
	
	// Right arm
	glPushMatrix();
	glTranslatef(0.3f, -0.1f, 0);
	glScalef(0.12f, 0.5f, 0.12f);
	glutSolidCube(1.0f);
	glPopMatrix();
	
	// Left leg
	glColor3f(0.3f, 0.3f, 0.3f); // Dark pants
	glPushMatrix();
	glTranslatef(-0.12f, -0.6f, 0);
	glScalef(0.14f, 0.6f, 0.14f);
	glutSolidCube(1.0f);
	glPopMatrix();
	
	// Right leg
	glPushMatrix();
	glTranslatef(0.12f, -0.6f, 0);
	glScalef(0.14f, 0.6f, 0.14f);
	glutSolidCube(1.0f);
	glPopMatrix();
	
	// Mail bag on shoulder
	glColor3f(0.6f, 0.5f, 0.3f); // Brown bag
	glPushMatrix();
	glTranslatef(0.25f, 0.1f, -0.15f);
	glRotatef(20, 0, 0, 1);
	glScalef(0.25f, 0.3f, 0.15f);
	glutSolidCube(1.0f);
	glPopMatrix();
	
	glPopMatrix();
	glColor3f(1.0f, 1.0f, 1.0f);
}

//=======================================================================
// Render UI
//=======================================================================
void RenderUI()
{
	// Switch to 2D orthographic projection
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, WIDTH, 0, HEIGHT);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	
	// Render vignette
	if(vignetteIntensity > 0) {
		glColor4f(1.0f, 0.0f, 0.0f, vignetteIntensity * 0.5f);
		glBegin(GL_QUADS);
		glVertex2f(0, 0);
		glVertex2f(WIDTH, 0);
		glVertex2f(WIDTH, HEIGHT);
		glVertex2f(0, HEIGHT);
		glEnd();
	}
	
	// Score
	glColor3f(1.0f, 1.0f, 1.0f);
	glRasterPos2f(20, HEIGHT - 30);
	char scoreText[50];
	sprintf_s(scoreText, sizeof(scoreText), "Score: %d", playerScore);
	for(int i = 0; scoreText[i] != '\0'; i++) {
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, scoreText[i]);
	}
	
	// Lives
	glRasterPos2f(20, HEIGHT - 60);
	char livesText[50];
	sprintf_s(livesText, sizeof(livesText), "Lives: %d", playerLives);
	for(int i = 0; livesText[i] != '\0'; i++) {
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, livesText[i]);
	}
	
	// Package timer
	if(hasPackage) {
		glColor3f(1.0f, packageTimer < 10.0f ? 0.0f : 1.0f, 0.0f);
		glRasterPos2f(20, HEIGHT - 90);
		char timerText[50];
		sprintf_s(timerText, sizeof(timerText), "Time: %.1f", packageTimer);
		for(int i = 0; timerText[i] != '\0'; i++) {
			glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, timerText[i]);
		}
	}
	
	// Deliveries
	glColor3f(1.0f, 1.0f, 1.0f);
	glRasterPos2f(20, HEIGHT - 120);
	char deliveriesText[50];
	sprintf_s(deliveriesText, sizeof(deliveriesText), "Deliveries: %d/%d", deliveredPackages, totalPackages);
	for(int i = 0; deliveriesText[i] != '\0'; i++) {
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, deliveriesText[i]);
	}
	
	// Game Over Screen
	if(gameState == GAME_OVER) {
		glColor4f(0.0f, 0.0f, 0.0f, 0.7f);
		glBegin(GL_QUADS);
		glVertex2f(0, 0);
		glVertex2f(WIDTH, 0);
		glVertex2f(WIDTH, HEIGHT);
		glVertex2f(0, HEIGHT);
		glEnd();
		
		glColor3f(1.0f, 0.0f, 0.0f);
		glRasterPos2f(WIDTH/2 - 80, HEIGHT/2 + 30);
		char* gameOverText = "GAME OVER";
		for(int i = 0; gameOverText[i] != '\0'; i++) {
			glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, gameOverText[i]);
		}
		
		glColor3f(1.0f, 1.0f, 1.0f);
		glRasterPos2f(WIDTH/2 - 100, HEIGHT/2 - 30);
		char scoreText2[50];
		sprintf_s(scoreText2, sizeof(scoreText2), "Final Score: %d", playerScore);
		for(int i = 0; scoreText2[i] != '\0'; i++) {
			glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, scoreText2[i]);
		}
	}
	
	// Win Screen
	if(gameState == WIN) {
		glColor4f(0.0f, 0.0f, 0.0f, 0.7f);
		glBegin(GL_QUADS);
		glVertex2f(0, 0);
		glVertex2f(WIDTH, 0);
		glVertex2f(WIDTH, HEIGHT);
		glVertex2f(0, HEIGHT);
		glEnd();
		
		glColor3f(0.0f, 1.0f, 0.0f);
		glRasterPos2f(WIDTH/2 - 60, HEIGHT/2 + 30);
		char* winText = "YOU WIN!";
		for(int i = 0; winText[i] != '\0'; i++) {
			glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, winText[i]);
		}
		
		glColor3f(1.0f, 1.0f, 1.0f);
		glRasterPos2f(WIDTH/2 - 100, HEIGHT/2 - 30);
		char scoreText3[50];
		sprintf_s(scoreText3, sizeof(scoreText3), "Final Score: %d", playerScore);
		for(int i = 0; scoreText3[i] != '\0'; i++) {
			glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, scoreText3[i]);
		}
	}
	
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}

//=======================================================================
// Display Function
//=======================================================================
void myDisplay(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Update lighting
	UpdateLighting();

	// Draw Sky
	RenderSky();
	
	// Draw Ground
	RenderGround();

	// Render houses
	for(size_t i = 0; i < houses.size(); i++) {
		glPushMatrix();
		glTranslatef(houses[i].position.x, houses[i].position.y, houses[i].position.z);
		glRotatef(houses[i].rotation, 0, 1, 0);
		glScalef(houses[i].scale, houses[i].scale, houses[i].scale);
		model_house.Draw();
		glPopMatrix();
	}

	// Render trees
	for(size_t i = 0; i < trees.size(); i++) {
		glPushMatrix();
		glTranslatef(trees[i].position.x, trees[i].position.y, trees[i].position.z);
		glScalef(trees[i].scale * 0.7f, trees[i].scale * 0.7f, trees[i].scale * 0.7f);
		model_tree.Draw();
		glPopMatrix();
	}
	
	// Render rocks
	for(size_t i = 0; i < rocks.size(); i++) {
		glPushMatrix();
		glTranslatef(rocks[i].position.x, rocks[i].position.y, rocks[i].position.z);
		glScalef(rocks[i].scale * 0.5f, rocks[i].scale * 0.5f, rocks[i].scale * 0.5f);
		glColor3f(0.5f, 0.5f, 0.5f);
		glutSolidSphere(1.0f, 16, 16);
		glColor3f(1.0f, 1.0f, 1.0f);
		glPopMatrix();
	}
	
	// Render fences
	for(size_t i = 0; i < fences.size(); i++) {
		glPushMatrix();
		glTranslatef(fences[i].position.x, fences[i].position.y, fences[i].position.z);
		glRotatef(fences[i].rotation, 0, 1, 0);
		glScalef(fences[i].scale * 0.3f, fences[i].scale * 0.3f, fences[i].scale * 0.3f);
		glColor3f(0.6f, 0.4f, 0.2f);
		// Simple fence representation
		for(int j = 0; j < 5; j++) {
			glPushMatrix();
			glTranslatef(j * 0.5f - 1.0f, 0.5f, 0);
			glScalef(0.1f, 1.0f, 0.1f);
			glutSolidCube(1.0f);
			glPopMatrix();
		}
		glColor3f(1.0f, 1.0f, 1.0f);
		glPopMatrix();
	}
	
	// Render crops
	for(size_t i = 0; i < crops.size(); i++) {
		glPushMatrix();
		glTranslatef(crops[i].position.x, crops[i].position.y, crops[i].position.z);
		glScalef(crops[i].scale * 0.2f, crops[i].scale * 0.5f, crops[i].scale * 0.2f);
		glColor3f(0.8f, 0.7f, 0.2f);
		glutSolidCone(0.5f, 1.0f, 8, 8);
		glColor3f(1.0f, 1.0f, 1.0f);
		glPopMatrix();
	}
	
	// Render street lights
	for(size_t i = 0; i < streetLights.size(); i++) {
		glPushMatrix();
		glTranslatef(streetLights[i].position.x, streetLights[i].position.y, streetLights[i].position.z);
		
		// Pole
		glColor3f(0.3f, 0.3f, 0.3f);
		glPushMatrix();
		glTranslatef(0, 2.5f, 0);
		glScalef(0.1f, 2.5f, 0.1f);
		glutSolidCube(1.0f);
		glPopMatrix();
		
		// Light
		if(streetLights[i].lightIntensity > 0) {
			float intensity = streetLights[i].lightIntensity;
			if(packageTimer < 10.0f && hasPackage) {
				glColor3f(intensity, 0.1f * intensity, 0.1f * intensity);
			} else {
				glColor3f(intensity * 0.9f, intensity * 0.9f, intensity * 0.7f);
			}
		} else {
			glColor3f(0.2f, 0.2f, 0.2f);
		}
		glPushMatrix();
		glTranslatef(0, 5.0f, 0);
		glutSolidSphere(0.3f, 16, 16);
		glPopMatrix();
		
		glColor3f(1.0f, 1.0f, 1.0f);
		glPopMatrix();
	}

	// Render packages
	for(size_t i = 0; i < packages.size(); i++) {
		if(!packages[i].collected) {
			float bobOffset = sin(glutGet(GLUT_ELAPSED_TIME) * 0.003f + i) * 0.1f;
			RenderPackage(packages[i].position.x, packages[i].position.y + bobOffset, 
						  packages[i].position.z, packages[i].scale);
		}
	}
	
	// Render player character (only in third-person view)
	if(!isFirstPerson) {
		RenderPlayer(playerPos.x, playerPos.y, playerPos.z, playerYaw);
	}
	
	// Render carried package
	if(hasPackage) {
		glPushMatrix();
		glTranslatef(cameraLookAt.x, cameraLookAt.y - 0.5f, cameraLookAt.z);
		RenderPackage(0, 0, 0, 0.3f);
		glPopMatrix();
	}

	// Render UI
	RenderUI();

	glutSwapBuffers();
}

//=======================================================================
// Keyboard Function
//=======================================================================
void myKeyboard(unsigned char button, int x, int y)
{
	keys[button] = true;
	
	switch (button)
	{
	case ' ': // Jump
		if(!isJumping && playerPos.y <= 1.5f && gameState == PLAYING) {
			isJumping = true;
			playerVelocity.y = jumpForce;
		}
		break;
	case 'c':
	case 'C': // Crouch
		isCrouching = !isCrouching;
		break;
	case 'v':
	case 'V': // Toggle camera
		isFirstPerson = !isFirstPerson;
		break;
	case 27: // ESC
		exit(0);
		break;
	default:
		break;
	}

	glutPostRedisplay();
}

//=======================================================================
// Keyboard Up Function
//=======================================================================
void myKeyboardUp(unsigned char button, int x, int y)
{
	keys[button] = false;
	glutPostRedisplay();
}

//=======================================================================
// Mouse Motion Function
//=======================================================================
void myMouseMotion(int x, int y)
{
	if(!mouseInitialized) {
		lastMouseX = x;
		lastMouseY = y;
		mouseInitialized = true;
		return;
	}
	
	int deltaX = x - lastMouseX;
	int deltaY = y - lastMouseY;
	
	playerYaw += deltaX * 0.2f;
	playerPitch -= deltaY * 0.2f;
	
	// Clamp pitch
	if(playerPitch > 89.0f) playerPitch = 89.0f;
	if(playerPitch < -89.0f) playerPitch = -89.0f;
	
	lastMouseX = x;
	lastMouseY = y;
	
	// Re-center mouse
	if(x < 100 || x > WIDTH - 100 || y < 100 || y > HEIGHT - 100) {
		lastMouseX = WIDTH / 2;
		lastMouseY = HEIGHT / 2;
		glutWarpPointer(lastMouseX, lastMouseY);
	}
	
	glutPostRedisplay();
}

//=======================================================================
// Mouse Function
//=======================================================================
void myMouse(int button, int state, int x, int y)
{
	if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN && gameState == PLAYING) {
		// Drop package functionality (not used in current implementation)
	}
}

//=======================================================================
// Timer Function
//=======================================================================
void myTimer(int value)
{
	float deltaTime = 0.016f; // ~60 FPS
	
	// Update sun rotation
	sunAngle += sunRotationSpeed;
	if(sunAngle > 2 * 3.14159f) sunAngle -= 2 * 3.14159f;
	
	// Update player
	UpdatePlayer(deltaTime);
	
	// Update camera
	UpdateCamera();
	
	// Update game state
	UpdateGameState(deltaTime);
	
	glutPostRedisplay();
	glutTimerFunc(16, myTimer, 0);
}

//=======================================================================
// Reshape Function
//=======================================================================
void myReshape(int w, int h)
{
	if (h == 0) h = 1;

	WIDTH = w;
	HEIGHT = h;

	glViewport(0, 0, w, h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fovy, (GLdouble)WIDTH / (GLdouble)HEIGHT, zNear, zFar);

	glMatrixMode(GL_MODELVIEW);
}

//=======================================================================
// Initialize Level Layout
//=======================================================================
void InitializeLevel()
{
	srand((unsigned int)time(NULL));
	
	// Create houses (delivery targets)
	houses.push_back(House(10, 1.0f, 15, 0));
	houses.push_back(House(-15, 1.0f, 20, 90));
	houses.push_back(House(20, 1.0f, -10, 180));
	houses.push_back(House(-20, 1.0f, -15, 270));
	houses.push_back(House(0, 1.0f, 25, 45));
	
	// Create packages
	for(int i = 0; i < totalPackages; i++) {
		float x = (rand() % 100) - 50;
		float z = (rand() % 100) - 50;
		packages.push_back(Package(x, 1.0f, z));
		packages[i].targetHouseIndex = i % houses.size();
	}
	
	// Create trees
	for(int i = 0; i < 25; i++) {
		float x = (rand() % 160) - 80;
		float z = (rand() % 160) - 80;
		// Avoid spawning too close to center
		if(fabs(x) > 8 || fabs(z) > 8) {
			trees.push_back(GameObject(x, 0, z, 0, 1.0f + (rand() % 50) / 100.0f));
		}
	}
	
	// Create rocks
	for(int i = 0; i < 20; i++) {
		float x = (rand() % 160) - 80;
		float z = (rand() % 160) - 80;
		if(fabs(x) > 5 || fabs(z) > 5) {
			rocks.push_back(GameObject(x, 0.5f, z, 0, 0.8f + (rand() % 40) / 100.0f));
		}
	}
	
	// Create fences
	for(int i = 0; i < 10; i++) {
		float x = (rand() % 140) - 70;
		float z = (rand() % 140) - 70;
		if(fabs(x) > 10 || fabs(z) > 10) {
			fences.push_back(GameObject(x, 0, z, rand() % 360, 1.0f));
		}
	}
	
	// Create crops
	for(int i = 0; i < 30; i++) {
		float x = (rand() % 120) - 60;
		float z = (rand() % 120) - 60;
		crops.push_back(GameObject(x, 0, z, 0, 1.0f));
	}
	
	// Create street lights
	streetLights.push_back(StreetLight(15, 0, 10));
	streetLights.push_back(StreetLight(-15, 0, 15));
	streetLights.push_back(StreetLight(25, 0, -5));
	streetLights.push_back(StreetLight(-25, 0, -10));
	streetLights.push_back(StreetLight(5, 0, 30));
	streetLights.push_back(StreetLight(-5, 0, -30));
	streetLights.push_back(StreetLight(30, 0, 20));
	streetLights.push_back(StreetLight(-30, 0, 20));
}

//=======================================================================
// Assets Loading Function
//=======================================================================
void LoadAssets()
{
	// Loading Model files
	model_house.Load("Models/house/house.3DS");
	model_tree.Load("Models/tree/Tree1.3ds");

	// Loading texture files
	tex_ground.Load("Textures/ground.bmp");
	loadBMP(&tex_sky, "Textures/blu-sky-3.bmp", true);
}

//=======================================================================
// OpenGL Configuration Function
//=======================================================================
void myInit(void)
{
	glClearColor(0.5f, 0.7f, 1.0f, 0.0f);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fovy, aspectRatio, zNear, zFar);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	InitMaterial();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_NORMALIZE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_2D);
	
	// Initialize camera
	UpdateCamera();
	gluLookAt(cameraPos.x, cameraPos.y, cameraPos.z, 
			  cameraLookAt.x, cameraLookAt.y, cameraLookAt.z, 
			  cameraUp.x, cameraUp.y, cameraUp.z);
}

//=======================================================================
// Idle Function - Updates camera view matrix
//=======================================================================
void myIdle()
{
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(cameraPos.x, cameraPos.y, cameraPos.z, 
			  cameraLookAt.x, cameraLookAt.y, cameraLookAt.z, 
			  cameraUp.x, cameraUp.y, cameraUp.z);
}

//=======================================================================
// Main Function
//=======================================================================
void main(int argc, char** argv)
{
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

	glutInitWindowSize(WIDTH, HEIGHT);

	glutInitWindowPosition(100, 100);

	glutCreateWindow(title);

	glutDisplayFunc(myDisplay);

	glutKeyboardFunc(myKeyboard);
	
	glutKeyboardUpFunc(myKeyboardUp);

	glutPassiveMotionFunc(myMouseMotion);
	
	glutMotionFunc(myMouseMotion);

	glutMouseFunc(myMouse);

	glutReshapeFunc(myReshape);
	
	glutTimerFunc(0, myTimer, 0);
	
	glutIdleFunc(myIdle);

	myInit();

	LoadAssets();
	
	InitializeLevel();
	
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);
	glEnable(GL_COLOR_MATERIAL);

	glShadeModel(GL_SMOOTH);
	
	// Hide cursor
	glutSetCursor(GLUT_CURSOR_NONE);

	glutMainLoop();
}