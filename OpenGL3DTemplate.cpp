#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <SFML/Audio.hpp>
#include <glut.h>
#include <iostream>

#define GLUT_KEY_ESCAPE 27
#define DEG2RAD(a) (a * 0.0174532925)

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

	Camera(float eyeX = 8.0f, float eyeY = 8.0f, float eyeZ = 8.0f, float centerX = 0.0f, float centerY = 0.0f, float centerZ = 0.0f, float upX = 0.0f, float upY = 1.0f, float upZ = 0.0f) {
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

	void look() {
		gluLookAt(
			eye.x, eye.y, eye.z,
			center.x, center.y, center.z,
			up.x, up.y, up.z
		);
	}
};

Camera camera;
float playerX = 5;
float playerY = 5;
float playerZ = -1.9;
float playerRotation = 0.0f;
int countdownTime = 100;
bool reachedGoal = false;
float wallColorR = 1;
float wallColorG = 0;
float wallColorB = 0;
float timerY = 4.0f;
float wheelRotationAngle = 0.0f;
float treeRotationAngle = 0.0f;
float goalRotationAngle = 0.0f;
float trashCanTranslationAnimation = 0.0f;
bool trashGoUp = true;
float lampSwingAngle = 0.0f;
bool changeLampPostDirection = false;
float seesawSwingAngle = 0.0f;
bool changeSeesawDirection = false;
float waterFountainTranslationAnimation = -0.3f;
bool reachedMaxWaterHeight = false;
bool turnOnAnimation = true;
bool firstPerson = false;


//BONUS
sf::Sound gameMusicSound;
sf::SoundBuffer gameMusicBuffer;

sf::Sound gameEndSound;
sf::SoundBuffer gameEndSoundBuffer;

sf::Sound gameWinSound;
sf::SoundBuffer gameWinSoundBuffer;

sf::Sound goalSound;
sf::SoundBuffer goalSoundBuffer;

void drawWall(double thickness) {
	glPushMatrix();
	glTranslated(0.5, 0.5 * thickness, 0.5);
	glScaled(30, thickness, 30);
	glutSolidCube(1);
	glPopMatrix();
}

void drawTableLeg(double thick, double len) {
	glPushMatrix();
	glTranslated(0, len / 2, 0);
	glScaled(thick, len, thick);
	glutSolidCube(1.0);
	glPopMatrix();
}
void drawJackPart() {
	glPushMatrix();
	glScaled(0.2, 0.2, 1.0);
	glutSolidSphere(1, 15, 15);
	glPopMatrix();
	glPushMatrix();
	glTranslated(0, 0, 1.2);
	glutSolidSphere(0.2, 15, 15);
	glTranslated(0, 0, -2.4);
	glutSolidSphere(0.2, 15, 15);
	glPopMatrix();
}
void drawJack() {
	glPushMatrix();
	drawJackPart();
	glRotated(90.0, 0, 1, 0);
	drawJackPart();
	glRotated(90.0, 1, 0, 0);
	drawJackPart();
	glPopMatrix();
}
void drawTable(double topWid, double topThick, double legThick, double legLen) {
	glPushMatrix();
	glTranslated(0, legLen, 0);
	glScaled(topWid, topThick, topWid);
	glutSolidCube(1.0);
	glPopMatrix();

	double dist = 0.95 * topWid / 2.0 - legThick / 2.0;
	glPushMatrix();
	glTranslated(dist, 0, dist);
	drawTableLeg(legThick, legLen);
	glTranslated(0, 0, -2 * dist);
	drawTableLeg(legThick, legLen);
	glTranslated(-2 * dist, 0, 2 * dist);
	drawTableLeg(legThick, legLen);
	glTranslated(0, 0, -2 * dist);
	drawTableLeg(legThick, legLen);
	glPopMatrix();
}

void drawTree() {
	// root of the tree
	glColor3f(0.545, 0.271, 0.075);
	GLUquadricObj* cylinder = gluNewQuadric();
	gluCylinder(cylinder, 0.3, 0.3, 3.0, 20, 20);
	gluDeleteQuadric(cylinder);

	// leaves part of the tree
	glColor3f(0.0, 1.0, 0.0);
	glutSolidSphere(1, 20, 20);

	// apples part of the tree
	glColor3f(1.0, 0.0, 0.0);
	glPushMatrix();
	glTranslatef(0.0, 1, 0.0);
	glutSolidSphere(0.1, 10, 10);
	glPopMatrix();

	glColor3f(1.0, 0.0, 0.0);
	glPushMatrix();
	glTranslatef(-0.3, 1, 0.0);
	glutSolidSphere(0.1, 10, 10);
	glPopMatrix();

	glColor3f(1.0, 0.0, 0.0);
	glPushMatrix();
	glTranslatef(0.3, 1, -0.2);
	glutSolidSphere(0.1, 10, 10);
	glPopMatrix();
}

void drawLampPost() {
	// a grey base
	glColor3f(0.5, 0.5, 0.5);  // Grey color
	glPushMatrix();
	glTranslatef(0, 0, 4);
	GLUquadricObj* base = gluNewQuadric();
	gluDisk(base, 0.0, 0.5, 20, 20);
	glPopMatrix();


	// the post in grey
	glColor3f(0.5, 0.5, 0.5);
	GLUquadricObj* cylinder = gluNewQuadric();
	gluCylinder(cylinder, 0.05, 0.05, 4, 20, 20);

	// the lightbulb in yellow
	glColor3f(255.0, 234.0, 0.0);
	glutSolidSphere(0.2, 20, 20);
}

void drawTrashCan() {
	// a grey base
	glColor3f(0.5, 0.5, 0.5);  // Grey color
	glPushMatrix();
	glTranslatef(0, 0, 1.5);
	GLUquadricObj* base = gluNewQuadric();
	gluCylinder(base, 0.5, 0.6, 0.5, 20, 20);
	glPopMatrix();


	// the body of the can in grey
	glColor3f(0.5, 0.5, 0.5);
	GLUquadricObj* cylinder = gluNewQuadric();
	gluCylinder(cylinder, 0.5, 0.5, 1.5, 20, 20);

	//the black lid on top 
	glColor3f(0, 0, 0);
	glPushMatrix();
	glTranslatef(0, 0, 0);
	GLUquadricObj* lid = gluNewQuadric();
	gluQuadricDrawStyle(lid, GLU_FILL);
	gluCylinder(lid, 0.6, 0.6, 0.1, 20, 20);
	glPopMatrix();

	// the black lid top
	glColor3f(0, 0, 0);
	glPushMatrix();
	glTranslatef(0, 0, 0);
	GLUquadricObj* lidTop = gluNewQuadric();
	gluDisk(lidTop, 0, 0.6, 20, 20);
	glPopMatrix();


}

void drawWaterFountain() {
	// first white base
	glColor3f(1.0, 1.0, 1.0);
	glPushMatrix();
	glTranslatef(0, 0, 1.5);
	GLUquadricObj* base = gluNewQuadric();
	gluCylinder(base, 3, 3, 0.5, 20, 20);
	glPopMatrix();

	//first water disk layer
	glColor3f(0.5, 0.7, 1.0);
	glPushMatrix();
	glTranslatef(0, 0, 1.5);
	GLUquadricObj* water = gluNewQuadric();
	gluDisk(water, 0.0, 3, 20, 20);
	glPopMatrix();

	// second white base
	glColor3f(1.0, 1.0, 1.0);
	glPushMatrix();
	glTranslatef(0, 0, 1);
	GLUquadricObj* base2 = gluNewQuadric();
	gluCylinder(base2, 2, 2, 0.5, 20, 20);
	glPopMatrix();

	//second water disk layer
	glColor3f(0.5, 0.7, 1.0);
	glPushMatrix();
	glTranslatef(0, 0, 1);
	GLUquadricObj* water2 = gluNewQuadric();
	gluDisk(water2, 0.0, 2, 20, 20);
	glPopMatrix();

	// third white base
	glColor3f(1.0, 1.0, 1.0);
	glPushMatrix();
	glTranslatef(0, 0, 0.3);
	GLUquadricObj* base3 = gluNewQuadric();
	gluCylinder(base3, 1, 1, 0.5, 20, 20);
	glPopMatrix();

	//third water disk layer
	glColor3f(0.5, 0.7, 1.0);
	glPushMatrix();
	glTranslatef(0, 0, 0.3);
	GLUquadricObj* water3 = gluNewQuadric();
	gluDisk(water3, 0.0, 1, 20, 20);
	glPopMatrix();



	// fourth white base
	glColor3f(1.0, 1.0, 1.0);
	glPushMatrix();
	glTranslatef(0, 0, 0.4);
	GLUquadricObj* base4 = gluNewQuadric();
	gluCylinder(base4, 0.3, 0.3, 0.9, 20, 20);
	glPopMatrix();

	//fourth water disk layer
	glColor3f(0.5, 0.7, 1.0);
	glPushMatrix();
	glTranslatef(0, 0, 0.4);
	GLUquadricObj* water4 = gluNewQuadric();
	gluDisk(water4, 0.0, 0.3, 20, 20);
	glPopMatrix();

	// fifth white base
	glColor3f(1.0, 1.0, 1.0);
	glPushMatrix();
	glTranslatef(0, 0, -0.3);
	GLUquadricObj* base5 = gluNewQuadric();
	gluCylinder(base5, 0.3, 0.3, 0.9, 20, 20);
	glPopMatrix();

	//fifth water disk layer
	glColor3f(0.5, 0.7, 1.0);
	glPushMatrix();
	glTranslatef(0, 0, waterFountainTranslationAnimation);
	GLUquadricObj* water5 = gluNewQuadric();
	gluDisk(water5, 0.0, 0.3, 20, 20);
	glPopMatrix();

	//cylinder for animation purposes
	glColor3f(0.5, 0.7, 1.0);
	glPushMatrix();
	glTranslatef(0, 0, waterFountainTranslationAnimation);
	GLUquadricObj* base6 = gluNewQuadric();
	gluCylinder(base6, 0.29, 0.29, 5, 20, 20);
	glPopMatrix();



}

void drawSeeSaw() {

	glColor3f(1.0, 0, 0.0);
	glutSolidSphere(0.5, 20, 20);

	glColor3f(0.545, 0.271, 0.075);
	glPushMatrix();
	glScaled(5.0, 0.05, 1.0);
	glTranslated(0, 10, 0);
	glutSolidCube(1);
	glPopMatrix();

	glColor3f(0, 0, 0);
	glPushMatrix();
	glRotated(-90, 1.0, 0.0, 0.0);
	glTranslated(2.5, 0, 1);
	glScaled(0.1, 1, 1.0);
	glutSolidCube(0.8);
	glPopMatrix();

	glColor3f(0, 0, 0);
	glPushMatrix();
	glRotated(-90, 1.0, 0.0, 0.0);
	glTranslated(2.1, 0, 0.6);
	glScaled(1, 1, 0.1);
	glutSolidCube(0.8);
	glPopMatrix();

	glColor3f(0, 0, 0);
	glPushMatrix();
	glRotated(-90, 1.0, 0.0, 0.0);
	glTranslated(-2.5, 0, 1);
	glScaled(0.1, 1, 1.0);
	glutSolidCube(0.8);
	glPopMatrix();

	glColor3f(0, 0, 0);
	glPushMatrix();
	glRotated(-90, 1.0, 0.0, 0.0);
	glTranslated(-2.1, 0, 0.6);
	glScaled(1, 1, 0.1);
	glutSolidCube(0.8);
	glPopMatrix();

}

void drawPlayer() {

	// torso
	glColor3f(0, 0, 0);
	GLUquadricObj* torso = gluNewQuadric();
	gluCylinder(torso, 0.05, 0.05, 1, 20, 20);

	// arms
	glPushMatrix();
	glColor3f(0.9608, 0.8706, 0.7019);
	glRotated(-45, 1.0, 0.0, 0.0);
	glRotated(-30, 0.0, 1.0, 0.0);
	glTranslated(0.1, -0.1, 0.05);
	GLUquadricObj* arm1 = gluNewQuadric();
	gluCylinder(arm1, 0.05, 0.05, 0.8, 20, 20);
	glPopMatrix();

	glPushMatrix();
	glColor3f(0.9608, 0.8706, 0.7019);
	glRotated(45, 1.0, 0.0, 0.0);
	glRotated(-20, 0.0, 1.0, 0.0);
	glTranslated(-0.1, 0.1, 0.05);
	GLUquadricObj* arm2 = gluNewQuadric();
	gluCylinder(arm2, 0.05, 0.05, 0.8, 20, 20);
	glPopMatrix();

	//legs
	glPushMatrix();
	glColor3f(0, 0, 0.7019);
	glRotated(-45, 1.0, 0.0, 0.0);
	glTranslated(0, -0.6, 0.55);
	GLUquadricObj* leg1 = gluNewQuadric();
	gluCylinder(leg1, 0.05, 0.05, 0.8, 20, 20);
	glPopMatrix();

	glPushMatrix();
	glColor3f(0, 0, 0.7019);
	glRotated(45, 1.0, 0.0, 0.0);
	glTranslated(0, 0.6, 0.55);
	GLUquadricObj* leg2 = gluNewQuadric();
	gluCylinder(leg2, 0.05, 0.05, 0.8, 20, 20);
	glPopMatrix();

	// head
	glColor3f(0.9608, 0.8706, 0.7019);
	glutSolidSphere(0.2, 20, 20);
}

void drawFence(double thickness, double scale) {
	glPushMatrix();
	glTranslated(0.5, 0.5, 0.5);
	glScaled(5.0 * scale, 1 * thickness, 10.0 * scale);
	glutSolidCube(1);
	glPopMatrix();
}

void drawGoal() {
	//black base
	glColor3f(0, 0, 0);
	glPushMatrix();
	glTranslatef(0, 0, 1.5);
	GLUquadricObj* base = gluNewQuadric();
	gluCylinder(base, 3, 3, 0.5, 20, 20);
	glPopMatrix();

	//first blue layer
	glColor3f(0, 0, 1.0);
	glPushMatrix();
	glTranslatef(0, 0, 1.5);
	GLUquadricObj* water = gluNewQuadric();
	gluDisk(water, 0.0, 3, 20, 20);
	glPopMatrix();


	//second yellow layer
	glColor3f(1.0, 1.0, 0.0);
	glPushMatrix();
	glTranslatef(0, 0, 1.45);
	GLUquadricObj* water2 = gluNewQuadric();
	gluDisk(water2, 0.0, 2, 20, 20);
	glPopMatrix();


	//third red layer
	glColor3f(1, 0, 0);
	glPushMatrix();
	glTranslatef(0, 0, 1.38);
	GLUquadricObj* water3 = gluNewQuadric();
	gluDisk(water3, 0.0, 1, 20, 20);
	glPopMatrix();

}

void drawWheel() {
	//wheel base
	glColor3f(1.0, 0.75, 0.8);
	glPushMatrix();
	glTranslatef(0, 0, 1.5);
	GLUquadricObj* base = gluNewQuadric();
	gluCylinder(base, 5, 5, 1, 50, 50);
	glPopMatrix();

	//bars inside the wheel
	glColor3f(0.5, 0.5, 0.5);
	glPushMatrix();
	GLUquadricObj* bar1 = gluNewQuadric();
	glRotated(90, 1.0, 0.0, 0.0);
	glTranslatef(0, 2, -5);
	gluCylinder(bar1, 0.05, 0.05, 10, 20, 20);
	glPopMatrix();

	glColor3f(0.5, 0.5, 0.5);
	glPushMatrix();
	GLUquadricObj* bar2 = gluNewQuadric();
	glRotated(90, 0.0, 1.0, 0.0);
	glTranslatef(-2, 0, -5);
	gluCylinder(bar2, 0.05, 0.05, 10, 20, 20);
	glPopMatrix();

	glColor3f(0.5, 0.5, 0.5);
	glPushMatrix();
	GLUquadricObj* bar3 = gluNewQuadric();
	glRotated(90, 1.0, 0.0, 0.0);
	glRotated(45, 0.0, 1.0, 0.0);
	glTranslatef(0, 2, -5);
	gluCylinder(bar3, 0.05, 0.05, 10, 20, 20);
	glPopMatrix();

	glColor3f(0.5, 0.5, 0.5);
	glPushMatrix();
	GLUquadricObj* bar4 = gluNewQuadric();
	glRotated(90, 1.0, 0.0, 0.0);
	glRotated(-45, 0.0, 1.0, 0.0);
	glTranslatef(0, 2, -5);
	gluCylinder(bar4, 0.05, 0.05, 10, 20, 20);
	glPopMatrix();

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

void print(int x, int y, int z, char* string)
{
	int len, i;
	glRasterPos3f(x, y, z);
	len = (int)strlen(string);
	for (i = 0; i < len; i++) {
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, string[i]);
	}
}

void Display() {
	setupCamera();
	setupLights();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//ground
	glColor3f(0.5, 0.8, 0.2);
	drawWall(0.5);

	if (reachedGoal) {
		glColor3f(0, 0, 0);
		char* p0s[20];
		sprintf((char*)p0s, "YOU WON!");
		print(0, 9, 0, (char*)p0s);
	}
	else if (countdownTime <= 0) {
		glColor3f(0, 0, 0);
		char* p0s[20];
		sprintf((char*)p0s, "GAME OVER!");
		print(0, 9, 0, (char*)p0s);
	}
	else {

		glColor3f(0, 0, 0);
		char* p1s[20];
		sprintf((char*)p1s, "Time: %d ", countdownTime);
		print(0, timerY, 0, (char*)p1s);
		glPopMatrix();
		//fence sides
		glColor3f(wallColorR, wallColorG, wallColorB);
		glPushMatrix();
		glRotated(-90, 0.0, 0.0, 1.0);
		glScaled(1, 1, 0.03);
		glTranslated(-2.5, -15, 510);
		drawFence(0.05, 1);
		glPopMatrix();

		glPushMatrix();
		glRotated(-90, 0.0, 0.0, 1.0);
		glScaled(1, 1, 0.03);
		glTranslated(-2.5, -15, -480);
		drawFence(0.05, 1);
		glPopMatrix();

		glPushMatrix();
		glRotated(-90, 0.0, 0.0, 1.0);
		glScaled(1, 1, 0.03);
		glTranslated(-2.5, 15, 510);
		drawFence(0.05, 1);
		glPopMatrix();

		glPushMatrix();
		glRotated(-90, 0.0, 0.0, 1.0);
		glScaled(1, 1, 0.03);
		glTranslated(-2.5, 15, -480);
		drawFence(0.05, 1);
		glPopMatrix();


		//horizontal fence bars
		glPushMatrix();
		glTranslated(-4, 2, 0);
		glRotated(90, 0, 0, 1.0);
		glTranslated(0, -0.01, 0);

		glTranslated(0, 10, -29.5);
		glScaled(1, 1, 60);
		drawFence(0.05, 0.05);

		glTranslated(1, 0, 0);
		drawFence(0.05, 0.05);

		glTranslated(1, 0, 0);
		drawFence(0.05, 0.05);

		glTranslated(-3, 0, 0);
		drawFence(0.05, 0.05);

		glTranslated(-1, 0, 0);
		drawFence(0.05, 0.05);
		glPopMatrix();

		glPushMatrix();
		glTranslated(6, 2, 0);
		glRotated(90, 0, 0, 1.0);
		glTranslated(0, -0.01, 0);

		glTranslated(0, -10, -29.5);
		glScaled(1, 1, 60);
		drawFence(0.05, 0.05);

		glTranslated(1, 0, 0);
		drawFence(0.05, 0.05);

		glTranslated(1, 0, 0);
		drawFence(0.05, 0.05);

		glTranslated(-3, 0, 0);
		drawFence(0.05, 0.05);

		glTranslated(-1, 0, 0);
		drawFence(0.05, 0.05);
		glPopMatrix();


		glPushMatrix();
		glTranslated(6, 2, 0);
		glRotated(90, 0, 0, 1.0);
		glRotated(90, 1, 0, 0.0);
		glTranslated(0, -0.01, 0);

		glTranslated(0, -15, -35.5);
		glScaled(1, 1, 60.0);
		drawFence(0.05, 0.05);

		glTranslated(1, 0, 0);
		drawFence(0.05, 0.05);

		glTranslated(1, 0, 0);
		drawFence(0.05, 0.05);

		glTranslated(-3, 0, 0);
		drawFence(0.05, 0.05);

		glTranslated(-1, 0, 0);
		drawFence(0.05, 0.05);
		glPopMatrix();

		//trees
		glPushMatrix();
		glRotated(90, 1, 0, 0);
		glTranslated(-5, -11, -3.5);
		glRotated(treeRotationAngle, 0, 0, 1);
		drawTree();
		glPopMatrix();

		glPushMatrix();
		glRotated(90, 1, 0, 0);
		glTranslated(-7, -11, -3.5);
		glRotated(treeRotationAngle, 0, 0, 1);
		drawTree();
		glPopMatrix();

		glPushMatrix();
		glRotated(90, 1, 0, 0);
		glTranslated(-3, -11, -3.5);
		glRotated(treeRotationAngle, 0, 0, 1);
		drawTree();
		glPopMatrix();

		//lamp posts
		glPushMatrix();
		glRotated(90, 1, 0, 0);
		glTranslated(-12, -12, -4.6);
		glRotated(lampSwingAngle, 1, 0, 0);
		drawLampPost();
		glPopMatrix();

		glPushMatrix();
		glRotated(90, 1, 0, 0);
		glTranslated(12, 12, -4.6);
		glRotated(lampSwingAngle, 1, 0, 0);
		drawLampPost();
		glPopMatrix();

		glPushMatrix();
		glRotated(90, 1, 0, 0);
		glTranslated(-12, 12, -4.6);
		glRotated(lampSwingAngle, 1, 0, 0);
		drawLampPost();
		glPopMatrix();

		//trash cans
		glPushMatrix();
		glRotated(90, 1, 0, 0);
		glTranslated(-11, -12, -2.5);
		glTranslated(0, 0, trashCanTranslationAnimation);
		drawTrashCan();
		glPopMatrix();

		glPushMatrix();
		glRotated(90, 1, 0, 0);
		glTranslated(11, 12, -2.5);
		glTranslated(0, 0, trashCanTranslationAnimation);
		drawTrashCan();
		glPopMatrix();

		glPushMatrix();
		glRotated(90, 1, 0, 0);
		glTranslated(-12, 11, -2.5);
		glTranslated(0, 0, trashCanTranslationAnimation);
		drawTrashCan();
		glPopMatrix();


		glPushMatrix();
		glRotated(90, 1, 0, 0);
		glTranslated(0, 0, -2.5);
		drawWaterFountain();
		glPopMatrix();

		//seesaw
		glPushMatrix();
		glTranslated(-5, 1, -2.5);
		glRotated(lampSwingAngle, 0, 0, 1);
		drawSeeSaw();
		glPopMatrix();

		//player
		glPushMatrix();
		glRotated(90, 1, 0, 0);
		glTranslated(playerX, playerY, playerZ);
		glRotatef(playerRotation, 0.0, 0.0, 1.0);
		drawPlayer();
		glPopMatrix();

		glPushMatrix();
		glRotated(90, 1, 0, 0);
		glTranslated(12, -11, -2.5);
		glRotated(goalRotationAngle, 1, 0, 0);
		drawGoal();
		glPopMatrix();

		//what holds the wheel
		glColor3f(1.0, 0.75, 0.8);
		glPushMatrix();
		glRotated(90, 0, 1, 0);
		glTranslated(0, 8, -13);
		GLUquadricObj* bar5 = gluNewQuadric();
		glTranslatef(0, 0, 1);
		gluCylinder(bar5, 0.05, 0.05, 2, 20, 20);
		glPopMatrix();

		//holding poles for the wheel
		glColor3f(1.0, 0.75, 0.8);
		glPushMatrix();
		GLUquadricObj* pole1 = gluNewQuadric();
		glRotated(90, 0, 1, 0);
		glTranslated(0, 8, -13);
		glRotated(90, 1.0, 0.0, 0.0);
		glTranslatef(0, 1, 0);
		gluCylinder(pole1, 0.05, 0.05, 10, 20, 20);
		glPopMatrix();

		glColor3f(1.0, 0.75, 0.8);
		glPushMatrix();
		GLUquadricObj* pole2 = gluNewQuadric();
		glRotated(90, 0, 1, 0);
		glTranslated(0, 8, -13);
		glRotated(90, 1.0, 0.0, 0.0);
		glTranslatef(0, 3, 0);
		gluCylinder(pole2, 0.05, 0.05, 10, 20, 20);
		glPopMatrix();

		glPushMatrix();
		glRotated(90, 0, 1, 0);
		glTranslated(0, 8, -13);
		glRotated(wheelRotationAngle, 0, 0, 1);
		drawWheel();
		glPopMatrix();
	}


	glFlush();
}
void checkGoalCollision() {
	if ((playerX >= 9 && playerX <= 14.5 && playerY >= -15 && playerY <= -7.6)) {
		reachedGoal = true;
		gameMusicSound.stop();
		gameWinSound.play();
		goalSound.play();
	}
}

void Keyboard(unsigned char key, int x, int y) {
	float d = 0.1;

	switch (key) {
	case 'w':
		camera.moveY(d);
		break;
	case 's':
		camera.moveY(-d);
		break;
	case 'a':
		camera.moveX(d);
		break;
	case 'd':
		camera.moveX(-d);
		break;
	case 'q':
		camera.moveZ(d);
		break;
	case 'e':
		camera.moveZ(-d);
		break;
	case '1':  //NORMAL VIEW
		camera.eye = Vector3f(8.0f, 8.0f, 8.0f);
		camera.center = Vector3f(0.0f, 0.0f, 0.0f);
		camera.up = Vector3f(0.0f, 1.0f, 0.0f);
		timerY = 4;
		firstPerson = false;
		break;
	case '2': //TOP VIEW
		camera.eye = Vector3f(0.1f, 30.0f, 0.0f);
		camera.center = Vector3f(0.0f, 0.0f, 0.0f);
		camera.up = Vector3f(0.0f, 1.0f, 0.0f);
		timerY = 4;
		firstPerson = false;
		break;
	case '3': //SIDE VIEW
		camera.eye = Vector3f(-30.0f, 0.0f, 0.0f);
		camera.center = Vector3f(0.0f, 0.0f, 0.0f);
		camera.up = Vector3f(0.0f, 1.0f, 0.0f);
		timerY = 7;
		firstPerson = false;
		break;
	case '4': //FRONT VIEW
		camera.eye = Vector3f(0.0f, 0.0f, 30.0f);
		camera.center = Vector3f(0.0f, 0.0f, 0.0f);
		camera.up = Vector3f(0.0f, 1.0f, 0.0f);
		timerY = 7;
		firstPerson = false;
		break;
		//PLAYER MOVEMENT
	case 'u': //move up
		if (playerX - 0.2 > -14) {
			playerX -= 0.2;
			playerRotation = 0;
		}
		if (firstPerson) {
			camera.eye = Vector3f(playerY, playerZ, playerX);
			camera.center = Vector3f(playerZ-100.f, playerY, playerX);
		}

		break;
	case 'j': //move down
		if (playerX + 0.2 < 15) {
			playerX += 0.2;
			playerRotation = 180;
		}
		camera.eye = Vector3f(playerX, playerY, playerZ);
		break;
	case 'h': //move left
		if (playerY + 0.2 < 15) {
			playerY += 0.2;
			playerRotation = -90;
		}
		camera.eye = Vector3f(playerX, playerY, playerZ);
		break;
	case 'k': //move right
		if (playerY - 0.2 > -14) {
			playerY -= 0.2;
			playerRotation = 90;
		}
		camera.eye = Vector3f(playerX, playerY, playerZ);
		break;
	case '5': //ANIMATION BUTTON
		if (turnOnAnimation) {
			turnOnAnimation = false;
		}
		else {
			turnOnAnimation = true;
		}
		break;
	case 'f': //first person view
		camera.eye = Vector3f(playerZ, playerY, playerX);
		camera.center = Vector3f(playerZ-100.f , playerY , playerX);
		camera.up = Vector3f(0.0f, 1.0f, 0.0f);
		firstPerson = true;
		break;

	case GLUT_KEY_ESCAPE:
		exit(EXIT_SUCCESS);
	}
	checkGoalCollision();

	glutPostRedisplay();
}
void Special(int key, int x, int y) {
	float a = 1.0;

	switch (key) {
	case GLUT_KEY_UP:
		camera.rotateX(a);
		break;
	case GLUT_KEY_DOWN:
		camera.rotateX(-a);
		break;
	case GLUT_KEY_LEFT:
		camera.rotateY(a);
		break;
	case GLUT_KEY_RIGHT:
		camera.rotateY(-a);
		break;

	}

	glutPostRedisplay();
}

void timer(int val)
{
	if (countdownTime != 0) {
		countdownTime--;
		if (wallColorR == 1) {
			wallColorR = 0;
			wallColorG = 1;
			wallColorB = 0;
		}
		else if (wallColorG == 1) {
			wallColorR = 0;
			wallColorG = 0;
			wallColorB = 1;
		}
		else {
			wallColorR = 1;
			wallColorG = 0;
			wallColorB = 0;
		}
	}

	if (countdownTime == 0) {
		gameMusicSound.stop();
		gameEndSound.play();
		glutTimerFunc(15000, timer, 0);
	}
	else {
		glutTimerFunc(1000, timer, 0);
	}
	glutPostRedisplay();

}

void Animation() {
	if (turnOnAnimation) {
		wheelRotationAngle += 0.1f;
		treeRotationAngle += 0.1f;
		if (trashGoUp) {
			trashCanTranslationAnimation -= 0.001f;
			if (trashCanTranslationAnimation < -1.0f) {
				trashGoUp = false;
			}
		}
		else {
			trashCanTranslationAnimation += 0.001f;
			if (trashCanTranslationAnimation > 0.1) {
				trashGoUp = true;
			}
		}
		goalRotationAngle += 0.1f;
		if (!changeLampPostDirection) {
			lampSwingAngle += 0.1f;
			if (lampSwingAngle > 20.0f) {
				changeLampPostDirection = true;
			}
		}
		else {
			lampSwingAngle -= 0.1f;
			if (lampSwingAngle < -20.0f) {
				changeLampPostDirection = false;
			}
		}

		if (!changeSeesawDirection) {
			seesawSwingAngle += 0.1f;
			if (seesawSwingAngle > 20.0f) {
				changeSeesawDirection = true;
			}
		}
		else {
			seesawSwingAngle -= 0.1f;
			if (seesawSwingAngle < -20.0f) {
				changeSeesawDirection = false;
			}
		}

		if (!reachedMaxWaterHeight) {
			waterFountainTranslationAnimation -= 0.003f;
			if (waterFountainTranslationAnimation < -5.0f) {
				reachedMaxWaterHeight = true;
			}
		}
		else {
			waterFountainTranslationAnimation += 0.01f;
			if (waterFountainTranslationAnimation > -0.3f) {
				reachedMaxWaterHeight = false;
			}
		}
	}



	glutPostRedisplay();
}

void main(int argc, char** argv) {
	glutInit(&argc, argv);

	glutInitWindowSize(900, 700);
	glutInitWindowPosition(50, 50);

	glutCreateWindow("3D Game");
	glutDisplayFunc(Display);
	glutKeyboardFunc(Keyboard);
	glutSpecialFunc(Special);
	glutTimerFunc(0, timer, 0);
	glutIdleFunc(Animation);

	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
	glClearColor(1.0f, 1.0f, 1.0f, 0.0f);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);
	glEnable(GL_COLOR_MATERIAL);

	glShadeModel(GL_SMOOTH);

	gameMusicBuffer.loadFromFile("game-music.wav");
	gameMusicSound.setBuffer(gameMusicBuffer);
	gameMusicSound.setVolume(100.0f);
	gameMusicSound.play();

	gameEndSoundBuffer.loadFromFile("game-end.wav");
	gameEndSound.setBuffer(gameEndSoundBuffer);
	gameEndSound.setVolume(100.0f);

	gameWinSoundBuffer.loadFromFile("game-win.wav");
	gameWinSound.setBuffer(gameWinSoundBuffer);
	gameWinSound.setVolume(100.0f);

	goalSoundBuffer.loadFromFile("goal.wav");
	goalSound.setBuffer(goalSoundBuffer);
	goalSound.setVolume(100.0f);

	glutMainLoop();
}
