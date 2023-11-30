#include <stdio.h>
#include <irrKlang.h>
#include <math.h>
#include <random>
#include <glut.h>
#include <cstdlib>
#include <iostream>
#include <string>
using namespace irrklang;
using namespace std;

#define GLUT_KEY_ESCAPE 27
#define DEG2RAD(a) (a * 0.0174532925)

ISoundEngine* engine = nullptr;
ISound* backgroundMusic;
ISoundSource* collectableMusic;
ISoundSource* animateMusic;
ISoundSource* goalMusic;
ISoundSource* loseMusic;

void init() {
	engine = createIrrKlangDevice();
	if (!engine) exit(1);
	backgroundMusic = engine->play2D("media/monkeys.mp3", true, false, true);
	collectableMusic = engine->addSoundSourceFromFile("media/collectable.wav");
	animateMusic = engine->addSoundSourceFromFile("media/animation.mp3");
	goalMusic = engine->addSoundSourceFromFile("media/goal.wav");
	loseMusic = engine->addSoundSourceFromFile("media/lose.wav");
	if (!backgroundMusic || !collectableMusic || !goalMusic || !loseMusic) exit(1);
}

void cleanup() {
	if (engine) {
		engine->drop();
		engine = nullptr;
	}
}

void Anim() {
	glutPostRedisplay();
}

void drawRect(int x1, int y1, int x2, int y2) {
	glPushMatrix();
	glBegin(GL_POLYGON);
	glVertex2f(x1, y1);
	glVertex2f(x2, y1);
	glVertex2f(x2, y2);
	glVertex2f(x1, y2);
	glEnd();
	glPopMatrix();
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

	Camera(float eyeX = 2.24816, float eyeY = 2.6667, float eyeZ = 2.24816, float centerX = 1.6868, float centerY = 2.05864, float centerZ = 1.6868, float upX = 0.0f, float upY = 1.0f, float upZ = 0.0f) {
		eye = Vector3f(eyeX, eyeY, eyeZ);
		center = Vector3f(centerX, centerY, centerZ);
		up = Vector3f(upX, upY, upZ);
	}

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

	void viewTop() {
		eye.x = 0.000982938; eye.y = 9.57839; eye.z = -0.219619;
		center.x = 0.00101701; center.y = 8.57836; center.z = -0.226333;
	}

	void viewFront() {
		eye.x = -0.0210365; eye.y = 0.803742; eye.z = 4.08213;
		center.x = -0.0159924; center.y = 0.688444; center.z = 3.08881;
	}

	void viewSide() {
		eye.x = 4.07816; eye.y = 0.685027; eye.z = 0.0150442;
		center.x = 3.07898; center.y = 0.644756; center.z = 0.0123142;
	}

	void view3D() {
		eye.x = 2.24816; eye.y = 2.6667; eye.z = 2.24816;
		center.x = 1.6868; center.y = 2.05864; center.z = 1.6868;
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

void update(int value) {
	glutPostRedisplay();
	glutTimerFunc(1000, update, 0);
}

void drawCuboid(double width, double height, double depth) {
	glPushMatrix();
	glScalef(width, height, depth);
	glutSolidCube(1.0);
	glPopMatrix();
}

class Character {
public:
	Character() : x(0.0), z(0.0), rotation(0.0) {}

	void draw() {
		glPushMatrix();
		glColor3f(1.0, 0.84, 0.69);
		glTranslatef(0.0 + x, 1, 0.0 + z);
		glutSolidSphere(0.2, 100, 100);
		glPopMatrix();
		glPushMatrix();
		glColor3f(0.0, 0, 0.39);
		glTranslatef(0.0 + x, 0.6, 0.0 + z);
		glRotatef(rotation, 0.0, 1.0, 0.0);
		glScalef(0.4, 0.6, 0.2);
		glutSolidCube(1.0);
		glPopMatrix();

		if (rotation == 0) {
			// Draw left arm as a cuboid
			glPushMatrix();
			glColor3f(1.0, 0.84, 0.69);  // Skin color for the left arm
			glTranslatef(-0.3 + x, 0.8, 0.0 + z);  // Left arm position
			glRotatef(45, 0.0, 0.0, 1.0);  // Rotate the left arm
			glRotatef(rotation, 0.0, 1.0, 0.0);
			drawCuboid(0.1, 0.5, 0.1);  // Adjust the dimensions of the left arm
			glPopMatrix();

			// Draw right arm as a cuboid
			glPushMatrix();
			glColor3f(1.0, 0.84, 0.69);  // Skin color for the right arm
			glTranslatef(0.3 + x, 0.8, 0.0 + z);  // Right arm position
			glRotatef(-45, 0.0, 0.0, 1.0);  // Rotate the right arm
			glRotatef(rotation, 0.0, 1.0, 0.0);
			drawCuboid(0.1, 0.5, 0.1);  // Adjust the dimensions of the right arm
			glPopMatrix();

			// Draw left leg as a cuboid
			glPushMatrix();
			glColor3f(0.0, 0.0, 0.8);  // Blue color for the left leg
			glTranslatef(-0.15 + x, 0.2, 0.0 + z);  // Left leg position
			glRotatef(rotation, 0.0, 1.0, 0.0);
			drawCuboid(0.15, 0.5, 0.1);  // Adjust the dimensions of the left leg
			glPopMatrix();

			// Draw right leg as a cuboid
			glPushMatrix();
			glColor3f(0.0, 0.0, 0.8);  // Blue color for the right leg
			glTranslatef(0.15 + x, 0.2, 0.0 + z);  // Right leg position
			glRotatef(rotation, 0.0, 1.0, 0.0);
			drawCuboid(0.15, 0.5, 0.1);  // Adjust the dimensions of the right leg
			glPopMatrix();
		}
		else {
			// Draw left arm as a cuboid
			glPushMatrix();
			glColor3f(1.0, 0.84, 0.69);  // Skin color for the left arm
			glTranslatef(0.0 + x, 0.8, -0.3 + z);  // Left arm position
			glRotatef(135, 1.0, 0.0, 0.0);  // Rotate the left arm
			glRotatef(rotation, 0.0, 1.0, 0.0);
			drawCuboid(0.1, 0.5, 0.1);  // Adjust the dimensions of the left arm
			glPopMatrix();

			// Draw right arm as a cuboid
			glPushMatrix();
			glColor3f(1.0, 0.84, 0.69);  // Skin color for the right arm
			glTranslatef(0.0 + x, 0.8, 0.3 + z);  // Left arm position
			glRotatef(-135, 1.0, 0.0, 0.0);  // Rotate the right arm
			glRotatef(rotation, 0.0, 1.0, 0.0);
			drawCuboid(0.1, 0.5, 0.1);  // Adjust the dimensions of the right arm
			glPopMatrix();

			// Draw left leg as a cuboid
			glPushMatrix();
			glColor3f(0.0, 0.0, 0.8);  // Blue color for the left leg
			glTranslatef(x, 0.2, -0.15 + z);  // Left leg position
			glRotatef(rotation, 0.0, 1.0, 0.0);
			drawCuboid(0.15, 0.5, 0.1);  // Adjust the dimensions of the left leg
			glPopMatrix();

			// Draw right leg as a cuboid
			glPushMatrix();
			glColor3f(0.0, 0.0, 0.8);  // Blue color for the right leg
			glTranslatef(x, 0.2, 0.15 + z);  // Right leg position
			glRotatef(rotation, 0.0, 1.0, 0.0);
			drawCuboid(0.15, 0.5, 0.1);  // Adjust the dimensions of the right leg
			glPopMatrix();
		}
	}

	void move(float dx, float dz) {
		if (x < 0) {
			x = max(float(-4.4), x + dx);
		}
		else {
			x = min(float(4.4), x + dx);
		} if (z < 0) {
			z = max(float(-4.4), z + dz);
		}
		else {
			z = min(float(4.4), z + dz);
		}
		cout << "x " << x << "\n";
		cout << "z " << z << "\n";
	}

	void rotate(int angle) {
		rotation = angle;
	}

	// Other member functions as needed

private:
	float x;  // X-coordinate of the character's position
	float z;  // Y-coordinate of the character's position
	float rotation;
};

Character character;

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

	gluPerspective(60.0, (double)1200 / (double)600, 0.1, 100.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	camera.look();
}

void Display() {
	/*
	cout << "EYE X " << camera.eye.x << "\n";
	cout << "EYE Y " << camera.eye.y << "\n";
	cout << "EYE Z " << camera.eye.z << "\n";
	cout << "CENTER X " << camera.center.x << "\n";
	cout << "CENTER Y " << camera.center.y << "\n";
	cout << "CENTER Z " << camera.center.z << "\n";
	*/
	//if (!lose && !win) {
		setupCamera();
		setupLights();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		character.draw();

		glutSwapBuffers();
		glFlush();
	//}
	/*
	else {
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0, 1200, 0, 600, -1, 1);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glDisable(GL_DEPTH_TEST);
		glPushMatrix();
		glColor3f(1, 1, 1);
		glBegin(GL_QUADS);
		glVertex2i(0, 0);
		glVertex2i(1200, 0);
		glVertex2i(1200, 600);
		glVertex2i(0, 600);
		glEnd();
		glPopMatrix();

		int x = 500;
		int y = 360;

		std::string text;
		if (win) {
			text = "CONGRATULATIONS, YOU WON!";
		}
		if (lose) {
			text = "GAME OVER, TIME IS UP!";
		}
		glColor3f(0, 0, 0);
		glRasterPos2i(x, y);
		for (const char& c : text) {
			glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, c);
		}
		glFlush();
	}

	if (win && !played) {
		backgroundMusic->stop();
		engine->play2D(goalMusic, false);
		played = true;
	}
	if (lose && !played) {
		backgroundMusic->stop();
		engine->play2D(loseMusic, false);
		played = true;
	}
	*/
	glFlush();
}

void Keyboard(unsigned char key, int x, int y) {
	float d = 1;
	//if (!win && !lose) {
		switch (key) {
		case 'a':
			character.move(-0.1, 0);
			camera.moveX(d);
			break;
		case 'd':
			character.move(0.1, 0);
			camera.moveX(-d);
			break;
	//}
	}

	glutPostRedisplay();
}

void Timer(int value) {
	glutPostRedisplay();
	glutTimerFunc(1000, Timer, 0);
}

void reshape(int w, int h) {
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, static_cast<double>(w) / h, 1.0, 10.0);
}

void main(int argc, char** argv) {
	glutInit(&argc, argv);

	glutInitWindowSize(1200, 600);
	glutInitWindowPosition(150, 150);

	init();

	glutCreateWindow("Galaxy Warriors");
	glutDisplayFunc(Display);
	glutReshapeFunc(reshape);


	glutIdleFunc(Anim);
	glutKeyboardFunc(Keyboard);

	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
	glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
	glutTimerFunc(0, Timer, 0);

	glEnable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f, 1200.0f / 600.0f, 0.1f, 300.0f);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);
	glEnable(GL_COLOR_MATERIAL);

	glShadeModel(GL_SMOOTH);

	atexit(cleanup);
	glutMainLoop();
}
