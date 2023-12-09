#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <glut.h>
#include <sstream>
#include <irrKlang.h>
#include <random>
#include <cstdlib>
#include <iostream>
#include <string>
using namespace irrklang;
using namespace std;


#define GLUT_KEY_ESCAPE 27
#define DEG2RAD(a) (a * 0.0174532925)

ISoundEngine* engine = nullptr;
ISound* backgroundMusic;
ISoundSource* powerupMusic;
ISoundSource* explosionMusic;
ISoundSource* transitionMusic;
ISoundSource* winMusic;
ISoundSource* loseMusic;
bool playedEndMusic = false;

float spaceshipX = 0.0f; // to move the spaceship right and left along the x-axis
float spaceshipZ = 0.0f; // to move the spaceship forward and backward along the z-axis


float sceneOnePlanetX = 0.0f;
float sceneOnePlanetZ = 32.0f;

float sceneTwoPlanetX = 0.0f;
float sceneTwoPlanetZ = 32.0f;

int collectiblesCount = 5;
float SceneOneCollectibleX[] = { -1.0, -0.8, 1.3, 0.8, 1.5 }; // { 1.0, 1.6, 0.3, 1.2, -0.9, 0, 1.4, -0.8, 0.8, 0 }
float SceneOneCollectibleZ[] = { 10, 15, 21, 25, 27 }; // { 10, 12.3, 15, 17, 21, 21.5, 25, 26, 26.5, 27 }

float SceneTwoCollectibleX[] = { 0.8, -1.4, 0.5, 1.3, -1.5 }; // { 1.3, 1.8, -0.9, 1.3, 1.4, 0.75, -1.4, -0.8, 0.8, 0 }
float SceneTwoCollectibleZ[] = { 12, 17, 23, 27, 29 }; // { 12, 14.3, 17, 21, 23, 25.5, 27, 28.5, 29.75, 30.5 }

float collectibleSize = 0.4f;

int obstacleCount = 10;
float SceneOneObstacleX[] = { 1.0, 1.6, 0.3, 1.2, -0.9, 0, 1.4, -0.8, 0.8, 0 };
float SceneOneObstacleZ[] = { 10, 12.3, 15, 17, 21, 21.5, 25, 26, 26.5, 27 };

float SceneTwoObstacleX[] = { 1.3, 1.8, -0.9, 1.3, 1.4, 0.75, -1.4, -0.8, 0.8, 0 };
float SceneTwoObstacleZ[] = { 12, 14.3, 17, 21, 23, 25.5, 27, 28.5, 28.75, 29 };

float obstacleSize = 0.4f;

int score = 0;
int health = 100;

int remainingTime = 120; // 2 minutes

bool isGameOver = false;


bool firstPersonView = false; // To toggle between first-person and normal views
bool thirdPersonView = false; // To toggle between third-person and normal views

bool sceneOne = true; // To toggle between scene one and scene two
bool sceneTwo = false; // To toggle between scene one and scene two  

void init() {
	engine = createIrrKlangDevice();
	if (!engine) exit(1);
	backgroundMusic = engine->play2D("media/background.mp3", true, false, true);
	powerupMusic = engine->addSoundSourceFromFile("media/powerup.mp3");
	explosionMusic = engine->addSoundSourceFromFile("media/explosion.mp3");
	transitionMusic = engine->addSoundSourceFromFile("media/transition.mp3");
	winMusic = engine->addSoundSourceFromFile("media/win.mp3");
	loseMusic = engine->addSoundSourceFromFile("media/lose.mp3");
	if (!backgroundMusic || !transitionMusic|| !powerupMusic || !explosionMusic || !winMusic || !loseMusic) exit(1);
}

void cleanup() {
	if (engine) {
		engine->drop();
		engine = nullptr;
	}
}


class Vector3f {
public:
	float x, y, z;

	Vector3f(float _x = 0.0f, float _y = 0.0f, float _z = 0.0f) {
		x = _x;
		y = _y;
		z = _z;
	}

	Vector3f operator+(Vector3f& v) {
		return Vector3f(x + v.x, y + v.y, z + v.z);
	}

	Vector3f operator-(Vector3f& v) {
		return Vector3f(x - v.x, y - v.y, z - v.z);
	}

	Vector3f operator*(float n) {
		return Vector3f(x * n, y * n, z * n);
	}

	Vector3f operator/(float n) {
		return Vector3f(x / n, y / n, z / n);
	}

	Vector3f unit() {
		return *this / sqrt(x * x + y * y + z * z);
	}

	Vector3f cross(Vector3f v) {
		return Vector3f(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
	}
};

class Camera {
public:
	Vector3f eye, center, up;

	Camera(float eyeX = 0.0f, float eyeY = 2.0f, float eyeZ = -3.0f,
		float centerX = 0.0f, float centerY = 0.0f, float centerZ = 0.0f,
		float upX = 0.0f, float upY = 1.0f, float upZ = 0.0f)
		: eye(eyeX, eyeY, eyeZ), center(centerX, centerY, centerZ), up(upX, upY, upZ) {}

	void moveX(float d) {
		Vector3f right = up.cross(center - eye).unit();
		eye = eye + right * d;
		center = center + right * d;
	}

	void moveY(float d) {
		eye = eye + up.unit() * d;
		center = center + up.unit() * d;
	}

	void moveZ(float d) {
		Vector3f view = (center - eye).unit();
		eye = eye + view * d;
		center = center + view * d;
	}

	void rotateX(float a) {
		Vector3f view = (center - eye).unit();
		Vector3f right = up.cross(view).unit();
		view = view * cos(DEG2RAD(a)) + up * sin(DEG2RAD(a));
		up = view.cross(right);
		center = eye + view;
	}

	void rotateY(float a) {
		Vector3f view = (center - eye).unit();
		Vector3f right = up.cross(view).unit();
		view = view * cos(DEG2RAD(a)) + right * sin(DEG2RAD(a));
		right = view.cross(up);
		center = eye + view;
	}

	void look() {
		gluLookAt(
			eye.x, eye.y, eye.z,
			center.x, center.y, center.z,
			up.x, up.y, up.z
		);
	}

};

Camera camera;

void drawSpaceship() {

	glColor3f(0.0f, 0.0f, 1.0f); // Blue color

	glPushMatrix();


	glTranslated(spaceshipX, 0.0, spaceshipZ);
	glScaled(0.5, 0.5, 0.5);

	// Draw a cube
	glBegin(GL_QUADS);

	// Front face
	glVertex3f(-0.5f, -0.5f, 0.5f);
	glVertex3f(0.5f, -0.5f, 0.5f);
	glVertex3f(0.5f, 0.5f, 0.5f);
	glVertex3f(-0.5f, 0.5f, 0.5f);

	// Back face
	glVertex3f(-0.5f, -0.5f, -0.5f);
	glVertex3f(0.5f, -0.5f, -0.5f);
	glVertex3f(0.5f, 0.5f, -0.5f);
	glVertex3f(-0.5f, 0.5f, -0.5f);

	// Top face
	glVertex3f(-0.5f, 0.5f, 0.5f);
	glVertex3f(0.5f, 0.5f, 0.5f);
	glVertex3f(0.5f, 0.5f, -0.5f);
	glVertex3f(-0.5f, 0.5f, -0.5f);

	// Bottom face
	glVertex3f(-0.5f, -0.5f, 0.5f);
	glVertex3f(0.5f, -0.5f, 0.5f);
	glVertex3f(0.5f, -0.5f, -0.5f);
	glVertex3f(-0.5f, -0.5f, -0.5f);

	// Left face
	glVertex3f(-0.5f, -0.5f, 0.5f);
	glVertex3f(-0.5f, 0.5f, 0.5f);
	glVertex3f(-0.5f, 0.5f, -0.5f);
	glVertex3f(-0.5f, -0.5f, -0.5f);

	// Right face
	glVertex3f(0.5f, -0.5f, 0.5f);
	glVertex3f(0.5f, 0.5f, 0.5f);
	glVertex3f(0.5f, 0.5f, -0.5f);
	glVertex3f(0.5f, -0.5f, -0.5f);

	glPopMatrix();

	glEnd();
}

void drawPlanet(float x, float z) {
	glPushMatrix();
	glColor3f(1.0f, 0.5f, 0.0f); // Orange color

	glTranslated(x, 0.0, z);
	glutSolidSphere(3.0, 50, 50);

	glPopMatrix();
}

void drawCollectible(float x, float z) {

	glPushMatrix();
	glColor3f(0.0f, 1.0f, 0.0f); // Green color

	glTranslated(x, 0.0, z);
	glScaled(collectibleSize, collectibleSize, collectibleSize);

	glBegin(GL_QUADS);
	// Front face
	glVertex3f(-0.5f, -0.5f, 0.5f);
	glVertex3f(0.5f, -0.5f, 0.5f);
	glVertex3f(0.5f, 0.5f, 0.5f);
	glVertex3f(-0.5f, 0.5f, 0.5f);

	// Back face
	glVertex3f(-0.5f, -0.5f, -0.5f);
	glVertex3f(0.5f, -0.5f, -0.5f);
	glVertex3f(0.5f, 0.5f, -0.5f);
	glVertex3f(-0.5f, 0.5f, -0.5f);

	// Top face
	glVertex3f(-0.5f, 0.5f, 0.5f);
	glVertex3f(0.5f, 0.5f, 0.5f);
	glVertex3f(0.5f, 0.5f, -0.5f);
	glVertex3f(-0.5f, 0.5f, -0.5f);

	// Bottom face
	glVertex3f(-0.5f, -0.5f, 0.5f);
	glVertex3f(0.5f, -0.5f, 0.5f);
	glVertex3f(0.5f, -0.5f, -0.5f);
	glVertex3f(-0.5f, -0.5f, -0.5f);

	// Left face
	glVertex3f(-0.5f, -0.5f, 0.5f);
	glVertex3f(-0.5f, 0.5f, 0.5f);
	glVertex3f(-0.5f, 0.5f, -0.5f);
	glVertex3f(-0.5f, -0.5f, -0.5f);

	// Right face
	glVertex3f(0.5f, -0.5f, 0.5f);
	glVertex3f(0.5f, 0.5f, 0.5f);
	glVertex3f(0.5f, 0.5f, -0.5f);
	glVertex3f(0.5f, -0.5f, -0.5f);

	glEnd();

	glPopMatrix();

}

void drawCollectibles() {

	if (sceneOne) {
		for (int i = 0; i < collectiblesCount; i++) {
			drawCollectible(SceneOneCollectibleX[i], SceneOneCollectibleZ[i]);
		}
	}
	else if (sceneTwo) {
		for (int i = 0; i < collectiblesCount; i++) {
			drawCollectible(SceneTwoCollectibleX[i], SceneTwoCollectibleZ[i]);
		}
	}
}

bool checkCollectibleCollision(float shipX, float shipZ, float collectibleX, float collectibleZ) {

	float shipSize = 0.5;

	if (fabs(shipX - collectibleX) < (shipSize + collectibleSize) / 2.0 &&
		fabs(shipZ - collectibleZ) < (shipSize + collectibleSize) / 2.0) {
		return true;
	}

	return false;
}

void drawObstacle(float x, float z) {

	glPushMatrix();
	glColor3f(1.0f, 0.0f, 0.0f); // Red color

	glTranslated(x, 0.0, z);
	glScaled(obstacleSize, obstacleSize, obstacleSize);

	glBegin(GL_QUADS);
	// Front face
	glVertex3f(-0.5f, -0.5f, 0.5f);
	glVertex3f(0.5f, -0.5f, 0.5f);
	glVertex3f(0.5f, 0.5f, 0.5f);
	glVertex3f(-0.5f, 0.5f, 0.5f);

	// Back face
	glVertex3f(-0.5f, -0.5f, -0.5f);
	glVertex3f(0.5f, -0.5f, -0.5f);
	glVertex3f(0.5f, 0.5f, -0.5f);
	glVertex3f(-0.5f, 0.5f, -0.5f);

	// Top face
	glVertex3f(-0.5f, 0.5f, 0.5f);
	glVertex3f(0.5f, 0.5f, 0.5f);
	glVertex3f(0.5f, 0.5f, -0.5f);
	glVertex3f(-0.5f, 0.5f, -0.5f);

	// Bottom face
	glVertex3f(-0.5f, -0.5f, 0.5f);
	glVertex3f(0.5f, -0.5f, 0.5f);
	glVertex3f(0.5f, -0.5f, -0.5f);
	glVertex3f(-0.5f, -0.5f, -0.5f);

	// Left face
	glVertex3f(-0.5f, -0.5f, 0.5f);
	glVertex3f(-0.5f, 0.5f, 0.5f);
	glVertex3f(-0.5f, 0.5f, -0.5f);
	glVertex3f(-0.5f, -0.5f, -0.5f);

	// Right face
	glVertex3f(0.5f, -0.5f, 0.5f);
	glVertex3f(0.5f, 0.5f, 0.5f);
	glVertex3f(0.5f, 0.5f, -0.5f);
	glVertex3f(0.5f, -0.5f, -0.5f);

	glEnd();

	glPopMatrix();

}

void drawObstacles() {
	if (sceneOne) {
		for (int i = 0; i < obstacleCount; i++) {
			drawObstacle(SceneOneObstacleX[i], SceneOneObstacleZ[i]);
		}
	}
	else if (sceneTwo) {
		for (int i = 0; i < obstacleCount; i++) {
			drawObstacle(SceneTwoObstacleX[i], SceneTwoObstacleZ[i]);
		}
	}
}

bool checkObstacleCollision(float shipX, float shipZ, float obstacleX, float obstacleZ) {

	float shipSize = 0.5;

	if (fabs(shipX - obstacleX) < (shipSize + obstacleSize) / 2.0 &&
		fabs(shipZ - obstacleZ) < (shipSize + obstacleSize) / 2.0) {
		return true;
	}

	return false;
}

bool checkPlanetCollision(float shipX, float shipZ, float planetX, float planetZ) {
	float shipSize = 0.5;
	float planetSize = 3.0;

	if (sceneOne) {
		if (fabs(shipX - planetX) < (shipSize + planetSize) / 2.0 &&
			fabs(shipZ - planetZ) < (shipSize + planetSize) / 2.0) {
			return true;
		}
	}
	else if (sceneTwo) {
		if (fabs(shipX - planetX) < (shipSize + planetSize) / 2.0 &&
			fabs(shipZ - planetZ) < (shipSize + planetSize) / 2.0) {
			return true;
		}
	}

	return false;
}

void setupLights() {
	GLfloat ambient[] = { 0.7f, 0.7f, 0.7, 1.0f };
	GLfloat diffuse[] = { 0.6f, 0.6f, 0.6, 1.0f };
	GLfloat specular[] = { 1.0f, 1.0f, 1.0, 1.0f };
	GLfloat shininess[] = { 50 };
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, shininess);

	GLfloat lightIntensity[] = { 0.7f, 0.7f, 1, 1.0f };
	GLfloat lightPosition[] = { -7.0f, 6.0f, 3.0f, 0.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, lightIntensity);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightIntensity);
}

void setupCamera() {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60, 640 / 480, 0.001, 100);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	camera.look();
}

void spachShipMovement(int value) {
	float d = 0.03;


	if (isGameOver == false && health != 0 && remainingTime != 0) {
		spaceshipZ += d;

		if (sceneOne) {
			for (int i = 0; i < obstacleCount; i++) {
				if (checkObstacleCollision(spaceshipX, spaceshipZ, SceneOneObstacleX[i], SceneOneObstacleZ[i])) {
					// Collision detected, remove the obstacle
					engine->play2D(explosionMusic, false);
					SceneOneObstacleX[i] = SceneOneObstacleZ[i] = 1000.0f;  // Move the obstacle out of the visible area
					health -= 10;
				}
			}

			for (int i = 0; i < collectiblesCount; i++) {
				if (checkCollectibleCollision(spaceshipX, spaceshipZ, SceneOneCollectibleX[i], SceneOneCollectibleZ[i])) {
					// Collision detected, remove the collectible
					engine->play2D(powerupMusic, false);
					SceneOneCollectibleX[i] = SceneOneCollectibleZ[i] = 1000.0f;  // Move the collectible out of the visible area
					score += 10;
				}
			}

			// Check for collision with the planet in sceneOne
			if (checkPlanetCollision(spaceshipX, spaceshipZ, sceneOnePlanetX, sceneOnePlanetZ)) {
				engine->play2D(transitionMusic, false);
				sceneOnePlanetX = sceneOnePlanetZ = 1000.0f;  // Move the planet out of the visible area
				sceneOne = false;
				sceneTwo = true;
				spaceshipZ = 0.0f;  // Reset the spaceship position for scene two
			}

		}
		else if (sceneTwo) {
			for (int i = 0; i < obstacleCount; i++) {
				if (checkObstacleCollision(spaceshipX, spaceshipZ, SceneTwoObstacleX[i], SceneTwoObstacleZ[i])) {
					engine->play2D(explosionMusic, false);
					SceneTwoObstacleX[i] = SceneTwoObstacleZ[i] = 1000.0f;  // Move the obstacle out of the visible area
					health -= 10;
				}
			}

			for (int i = 0; i < collectiblesCount; i++) {
				if (checkCollectibleCollision(spaceshipX, spaceshipZ, SceneTwoCollectibleX[i], SceneTwoCollectibleZ[i])) {
					engine->play2D(powerupMusic, false);
					SceneTwoCollectibleX[i] = SceneTwoCollectibleZ[i] = 1000.0f;  // Move the collectible out of the visible area
					score += 10;
				}
			}

			// Check for collision with the planet in sceneTwo
			if (checkPlanetCollision(spaceshipX, spaceshipZ, sceneTwoPlanetX, sceneTwoPlanetZ)) {
				//planetX = planetZ = 1000.0f;  // Move the planet out of the visible area
				isGameOver = true;
				backgroundMusic->stop();
				engine->play2D(winMusic, false);
				playedEndMusic = true;
			}

		}


		// Update camera position to follow the spaceship
		if (firstPersonView) {
			camera.eye.z = spaceshipZ + 1.0; // Adjust the camera's distance from the spaceship
			camera.center.z = spaceshipZ + 3.0;
			camera.eye.x = spaceshipX;
			camera.center.x = spaceshipX;
			camera.eye.y = 0.0;
			camera.center.y = 0.0;
		}
		else {
			camera.eye.z = spaceshipZ - 3.0; // Adjust the camera's distance from the spaceship
			camera.center.z = spaceshipZ;
			camera.center.x = spaceshipX;
		}



		glutPostRedisplay();


		glutTimerFunc(16, spachShipMovement, 0); // 16 milliseconds (about 60 frames per second)

	}
}

void drawScore(int score) {
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, glutGet(GLUT_WINDOW_WIDTH), 0, glutGet(GLUT_WINDOW_HEIGHT));

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glColor3f(1.0f, 1.0f, 1.0f); // White color

	std::stringstream ss;
	ss << "Score: " << score;
	std::string scoreString = ss.str();

	glRasterPos2i(10, glutGet(GLUT_WINDOW_HEIGHT) - 20);

	for (char const& c : scoreString) {
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
	}

	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}

void drawHealth(int health) {
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, glutGet(GLUT_WINDOW_WIDTH), 0, glutGet(GLUT_WINDOW_HEIGHT));

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glColor3f(1.0f, 1.0f, 1.0f); // White color

	std::stringstream ss;
	ss << "Health: " << health << "%";
	std::string healthString = ss.str();

	glRasterPos2i(glutGet(GLUT_WINDOW_WIDTH) - 105, glutGet(GLUT_WINDOW_HEIGHT) - 20);

	for (char const& c : healthString) {
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
	}

	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}

void drawTimer(int time) {
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, glutGet(GLUT_WINDOW_WIDTH), 0, glutGet(GLUT_WINDOW_HEIGHT));
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glColor3f(1.0f, 1.0f, 1.0f); // White color
	glRasterPos2f(glutGet(GLUT_WINDOW_WIDTH) / 2 - 30, glutGet(GLUT_WINDOW_HEIGHT) - 20);

	std::stringstream ss;
	ss << "Time: " << time;
	std::string str = ss.str();
	for (int i = 0; i < str.length(); i++) {
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, str[i]);
	}

	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}

void updateGameTimer(int value) {

	remainingTime--;

	if (remainingTime <= 0) {
		if (!playedEndMusic) 
		{
			isGameOver = true;
			backgroundMusic->stop();
			engine->play2D(loseMusic, false);
			playedEndMusic = true;
		}
	}
	glutPostRedisplay();

	// Set up the next timer call (e.g., 1000 milliseconds for 1 second)
	glutTimerFunc(1000, updateGameTimer, 0);
}

void drawSceneTransition() {
	// Clear the depth buffer to allow the text to appear on top
	glClear(GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, glutGet(GLUT_WINDOW_WIDTH), 0, glutGet(GLUT_WINDOW_HEIGHT));

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	// Set color for the transition message
	glColor3f(1.0, 1.0, 1.0); // White color

	const char* transitionMsg = (sceneTwo ? "Level : 2" : "Level : 1");
	glRasterPos2f(glutGet(GLUT_WINDOW_WIDTH) / 2.0 - 309, glutGet(GLUT_WINDOW_HEIGHT) / 2.0 + 180);

	// Draw the transition message
	while (*transitionMsg) {
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *transitionMsg++);
	}

	// Restore matrices
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}


void Display() {
	setupCamera();
	setupLights();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



	if (sceneOne) {

		if (isGameOver || health <= 0) {
			// Display "Game Over" message
			glMatrixMode(GL_PROJECTION);
			glPushMatrix();
			glLoadIdentity();
			gluOrtho2D(0, 640, 0, 480); // Adjust based on your window dimensions

			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			glLoadIdentity();

			// Set color for "Game Over" message
			glColor3f(1.0, 0.0, 0.0);

			const char* gameOverMsg = "Game Over";
			glRasterPos2f(640 / 2.0 - 50, 480.0 / 2); // Adjust the position as needed

			while (*gameOverMsg) {
				glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *gameOverMsg++);
			}

			// Set color for score message
			glColor3f(1.0, 1.0, 1.0); // White color

			const char* scoreMsg = "Your Score: ";
			glRasterPos2f(640 / 2.0 - 50, 480.0 / 2 - 20); // Adjust the position as needed

			while (*scoreMsg) {
				glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *scoreMsg++);
			}

			std::string scoreString = std::to_string(score);

			// Draw the actual score value
			for (const char& c : scoreString) {
				glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
			}
			if (!playedEndMusic) {
				backgroundMusic->stop();
				engine->play2D(loseMusic, false);
				playedEndMusic = true;
			}
			// Restore matrices
			glPopMatrix();
			glMatrixMode(GL_PROJECTION);
			glPopMatrix();
			glMatrixMode(GL_MODELVIEW);
		}

		else {

			glPushMatrix();
			drawScore(score);
			glPopMatrix();

			glPushMatrix();
			drawHealth(health);
			glPopMatrix();

			glPushMatrix();
			drawTimer(remainingTime);
			glPopMatrix();

			glPushMatrix();
			drawSpaceship();
			glPopMatrix();


			glPushMatrix();
			drawPlanet(sceneOnePlanetX, sceneOnePlanetZ);
			glPopMatrix();

			glPushMatrix();
			drawObstacles();
			glPopMatrix();

			glPushMatrix();
			drawCollectibles();
			glPopMatrix();

			glPushMatrix();
			drawSceneTransition();
			glPopMatrix();


		}


	}
	else if (sceneTwo) {


		if (checkPlanetCollision && isGameOver && health != 0 && remainingTime != 0 && score != 0) {

			// Display "Congratulations, You Won!" message
			glMatrixMode(GL_PROJECTION);
			glPushMatrix();
			glLoadIdentity();
			gluOrtho2D(0, 640, 0, 480); // Adjust based on your window dimensions

			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			glLoadIdentity();

			// Set color for "Congratulations, You Won!" message
			glColor3f(0.0, 1, 0.0);

			const char* winMsg = "Congratulations, You Won!";
			glRasterPos2f(640 / 2.0 - 120, 480.0 / 2); // Adjust the position as needed

			while (*winMsg) {
				glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *winMsg++);
			}

			// Display the score
			glColor3f(1.0, 1.0, 1.0); // Set color to white

			std::stringstream ss;
			ss << "Your Score: " << score;
			std::string scoreString = ss.str();

			glRasterPos2f(640 / 2.0 - 50, 480.0 / 2 - 20);

			for (char const& c : scoreString) {
				glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
			}

			// Restore matrices
			glPopMatrix();
			glMatrixMode(GL_PROJECTION);
			glPopMatrix();
			glMatrixMode(GL_MODELVIEW);
		}

		else if (isGameOver || health <= 0) {

			// Display "Game Over" message
			glMatrixMode(GL_PROJECTION);
			glPushMatrix();
			glLoadIdentity();
			gluOrtho2D(0, 640, 0, 480); // Adjust based on your window dimensions

			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			glLoadIdentity();

			// Set color for "Game Over" message
			glColor3f(1.0, 0.0, 0.0);

			const char* gameOverMsg = "Game Over";
			glRasterPos2f(640 / 2.0 - 50, 480.0 / 2); // Adjust the position as needed

			while (*gameOverMsg) {
				glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *gameOverMsg++);
			}

			// Set color for score message
			glColor3f(1.0, 1.0, 1.0); // White color

			const char* scoreMsg = "Your Score: ";
			glRasterPos2f(640 / 2.0 - 50, 480.0 / 2 - 20); // Adjust the position as needed

			while (*scoreMsg) {
				glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *scoreMsg++);
			}


			std::string scoreString = std::to_string(score);

			// Draw the actual score value
			for (const char& c : scoreString) {
				glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
			}

			if (!playedEndMusic) {
				backgroundMusic->stop();
				engine->play2D(loseMusic, false);

				playedEndMusic = true;
			}

			// Restore matrices
			glPopMatrix();
			glMatrixMode(GL_PROJECTION);
			glPopMatrix();
			glMatrixMode(GL_MODELVIEW);

		}

		else {

			glPushMatrix();
			drawScore(score);
			glPopMatrix();

			glPushMatrix();
			drawHealth(health);
			glPopMatrix();

			glPushMatrix();
			drawTimer(remainingTime);
			glPopMatrix();

			glPushMatrix();
			drawSpaceship();
			glPopMatrix();

			glPushMatrix();
			drawPlanet(sceneTwoPlanetX, sceneTwoPlanetZ);
			glPopMatrix();

			glPushMatrix();
			drawObstacles();
			glPopMatrix();

			glPushMatrix();
			drawCollectibles();
			glPopMatrix();

			glPushMatrix();
			drawSceneTransition();
			glPopMatrix();

		}

	}

	glFlush();
}

void Keyboard(unsigned char key, int x, int y) {
	float d = 0.1;

	switch (key) {
		case GLUT_KEY_ESCAPE:
			exit(EXIT_SUCCESS);
		}

	glutPostRedisplay();
}
void Special(int key, int x, int y) {
	float a = 1.0;
	float d = 0.3;
	int windowWidth = glutGet(GLUT_WINDOW_WIDTH);
	float maxX = static_cast<float>(windowWidth) / 2.0f;

	switch (key) {
	
	case GLUT_KEY_LEFT:
		if (spaceshipX + d < 1.6) {
			spaceshipX += d;
		}
		//printf("spaceshipX: %f\n", spaceshipX);
		break;
	case GLUT_KEY_RIGHT:
		if (spaceshipX - d > -1.6) {
			spaceshipX -= d;
		}
		break;
	}

	glutPostRedisplay();
}

void mouseClick(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) { // First-person view
		firstPersonView = true;
		camera.eye.z = spaceshipZ + 1.0; // Adjust the camera's distance from the spaceship
		camera.center.z = spaceshipZ + 3.0;
		camera.eye.x = spaceshipX;
		camera.center.x = spaceshipX;
		camera.eye.y = 0.0;
		camera.center.y = 0.0;

	}

	else if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) { // Third-person view
		firstPersonView = false;
		camera.eye.x = 0.0f;
		camera.eye.y = 2.0f;
		camera.eye.z = spaceshipZ - 3.0; // Adjust the camera's distance from the spaceship
		camera.center.x = 0.0f;
		camera.center.y = 0.0f;
		camera.center.z = spaceshipZ;
	}
}

void main(int argc, char** argv) {
	glutInit(&argc, argv);

	glutInitWindowSize(640, 480);
	glutInitWindowPosition(50, 50);

	init();

	glutCreateWindow("Guardians Of The Galaxy");
	glutDisplayFunc(Display);
	glutKeyboardFunc(Keyboard);
	glutSpecialFunc(Special);

	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);


	glutTimerFunc(16, spachShipMovement, 0);

	// mouse click function
	glutMouseFunc(mouseClick);

	// Set up the initial timer call for game timer
	glutTimerFunc(1000, updateGameTimer, 0);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);
	glEnable(GL_COLOR_MATERIAL);

	glShadeModel(GL_SMOOTH);

	glutMainLoop();
}