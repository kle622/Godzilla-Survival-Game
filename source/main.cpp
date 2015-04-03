//  
//example code to draw ground and godzilla - ZJ Wood
//

#include <stdio.h>
#include <stdlib.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cassert>
#include <cmath>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include "GLSL.h"
#include "tiny_obj_loader.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp" //perspective, trans etc
#include "glm/gtc/type_ptr.hpp" //value_ptr
#include "RenderingHelper.h"

#define M_PI 3.14159

GLFWwindow* window;
using namespace std;
using namespace glm;

RenderingHelper ModelTrans;

vector<tinyobj::shape_t> godzilla;
vector<tinyobj::shape_t> cube;
vector<tinyobj::shape_t> gun;
vector<tinyobj::shape_t> star;
vector<tinyobj::shape_t> bullet;
vector<tinyobj::shape_t> flag;
vector<tinyobj::shape_t> mine;
vector<tinyobj::material_t> materials;

int mouseButton = 0;
int mouseAction = 0;
int mouseMods = 0;

int g_SM = 1;
int g_width;
int g_height;
int numGodzillas = 20;
int numTowers = 22;
int numMinesLeft = 0;
float bulletSpread = 0.03;
float bulletSpeed = 0.25;
float maxBulletDistance = 100;
float cameraSpeed = 0.015;
float backPedalSpeed = 0.008;
float godzillaSpeed = 0.01;
float godzillaSpeedIncrease = .002;
float godzillaSpread = 4.0;
float spawnCircleRadius = 5;
float minGodzillaSpawn = 25;
int godzillaRadius = 38;
float towerSpread = 4;
float towerRadius = 38;
float starRadius = 10;
bool playerDead = false;
glm::vec3 g_light(30, 30, 0);

GLuint ShadeProg;

GLuint posBufObjGodzilla = 0;
GLuint norBufObjGodzilla = 0;
GLuint indBufObjGodzilla = 0;

GLuint posBufObjCube = 0;
GLuint norBufObjCube = 0;
GLuint indBufObjCube = 0;

GLuint posBufObjGun = 0;
GLuint norBufObjGun = 0;
GLuint indBufObjGun = 0;

GLuint posBufObjStar = 0;
GLuint norBufObjStar = 0;
GLuint indBufObjStar = 0;

GLuint posBufObjBullet = 0;
GLuint norBufObjBullet = 0;
GLuint indBufObjBullet = 0;

GLuint posBufObjFlag = 0;
GLuint norBufObjFlag = 0;
GLuint indBufObjFlag = 0;

GLuint posBufObjMine = 0;
GLuint norBufObjMine = 0;
GLuint indBufObjMine = 0;

GLuint posBufObjG = 0;
GLuint norBufObjG = 0;
//Handles to the shader data
GLint h_aPosition;
GLint h_aNormal;
GLint h_uModelMatrix;
GLint h_uViewMatrix;
GLint h_uProjMatrix;
GLint h_uLightPos;
GLint h_uMatAmb, h_uMatDif, h_uMatSpec, h_uMatShine;
GLint h_uShadeM;
GLint h_uCam;

glm::vec3 position(0, 0.5, 0);
glm::vec3 direction(0, 0, 0);
glm::vec3 rightV(0, 0, 0);

vector<vec3> godzillaPos;

vector<vec3> minePos;

vec3 gunPos;

vector<vec3> starPos;

vector<vec3> bulletPos;
vector<vec3> bulletDirection;
vector<float> bulletRotX;
vector<float> bulletRotY;

vector<vec3> towerPos;
vector<float> towerRot;
vector<float> towerMat;

float getRand(double M, double N)
{
	float random = (float)(M + (rand() / (RAND_MAX / (N - M))));
	return roundf(random * 1000) / 1000;
}

void addMine() {
	vec3 pos = vec3(position.x + direction.x*3.0f, -1, position.z + direction.z*3.0f);
	minePos.push_back(pos);
}

void addBullet() {
	double theta = 0;
	double phi = 0;

	glfwGetCursorPos(window, &theta, &phi);
	float horizontal = .005 * float(g_width / 2 - theta);
	float vertical = .005 * float(g_height / 2 - phi);

	if (vertical > 0.7) {
		vertical = 0.7;
	}
	else if (vertical < -0.5) {
		vertical = -0.5;
	}

	bulletPos.push_back(position - vec3(0.0, 0.3, 0.0));
	bulletDirection.push_back(vec3(direction.x + getRand(-bulletSpread, bulletSpread), 
		direction.y + getRand(-0.05, 0.05), direction.z + getRand(-bulletSpread, bulletSpread)));
	bulletRotX.push_back(-(180 * vertical / M_PI));
	bulletRotY.push_back(180 * horizontal / M_PI);
}

void addTowers(int num) {
	for (int i = 0; i < num; i++) {
		float angle = getRand(-M_PI, M_PI);
		towerPos.push_back(vec3(getRand(10, towerRadius) * cos(angle), .5, 
			getRand(10, towerRadius) * sin(angle)));
		towerRot.push_back(getRand(0, 360));
		towerMat.push_back(floor(getRand(0, 5.9)));
	}
}

bool boundingBoxCollide(vec3 stationary, vec3 moving, 
	float xBound, float yBound, float zBound) {
	vec3 point1 = vec3(stationary.x - xBound, stationary.y - yBound, stationary.z - zBound);
	vec3 point2 = vec3(stationary.x + xBound, stationary.y + yBound, stationary.z + zBound);

	if (point1.x <= moving.x && moving.x <= point2.x &&
		point1.y <= moving.y && moving.y <= point2.y &&
		point1.z <= moving.z && moving.z <= point2.z) {
		return true;
	}
	return false;
}

void godzillaBulletCollide() {
	for (int i = 0; i < bulletPos.size(); i++) {
		for (int j = 0; j < godzillaPos.size(); j++) {
			if (boundingBoxCollide(godzillaPos[j], bulletPos[i], 1.0f, 2.0f, 1.0f)) {
				godzillaPos.erase(godzillaPos.begin() + j);
				bulletPos.erase(bulletPos.begin() + i);
				bulletDirection.erase(bulletDirection.begin() + i);
				bulletRotX.erase(bulletRotX.begin() + i);
				bulletRotY.erase(bulletRotY.begin() + i);
				break;
			}
		}
	}
}

void godzillaMineCollide() {
	for (int i = 0; i < minePos.size(); i++) {
		for (int j = 0; j < godzillaPos.size(); j++) {
			if (boundingBoxCollide(godzillaPos[j], minePos[i], 1.0f, 5.0f, 1.0f)) {
				godzillaPos.erase(godzillaPos.begin() + j);
				minePos.erase(minePos.begin() + i);
				break;
			}
		}
	}
}

void towerBulletCollide() {
	for (int i = 0; i < bulletPos.size(); i++) {
		for (int j = 0; j < towerPos.size(); j++) {
			if (boundingBoxCollide(towerPos[j], bulletPos[i], 2.0f, 4.0f, 2.0f)) {
				bulletPos.erase(bulletPos.begin() + i);
				bulletDirection.erase(bulletDirection.begin() + i);
				bulletRotX.erase(bulletRotX.begin() + i);
				bulletRotY.erase(bulletRotY.begin() + i);
				break;
			}
		}
	}
}

void addGodzillas(int num) {
	for (int i = 0; i < num; i++) {
		float angle = getRand(-M_PI, M_PI);
		godzillaPos.push_back(vec3(godzillaRadius * cos(angle), .5, godzillaRadius * sin(angle)));
	}
}

void addStar() {
	float angle = getRand(-M_PI, M_PI);
	starPos.push_back(vec3(getRand(0, starRadius) * cos(angle), 20,
		getRand(0, starRadius) * sin(angle)));
}

void towerCollide(vec3 oldPosition) {
	for (int i = 0; i < towerPos.size(); i++) {
		if (length(position - towerPos[i]) <= 3.0) {
			position = oldPosition;
			break;
		}
	}
}

void godzillaCollide() {
	for (int i = 0; i < godzillaPos.size(); i++) {
		if (length(position - godzillaPos[i]) <= 2.5) {
			playerDead = true;
			break;
		}
	}
}

int printOglError(const char *file, int line) {
	/* Returns 1 if an OpenGL error occurred, 0 otherwise. */
	GLenum glErr;
	int    retCode = 0;

	glErr = glGetError();
	while (glErr != GL_NO_ERROR)
	{
		retCode = 1;
		glErr = glGetError();
	}
	return retCode;
}

inline void safe_glUniformMatrix4fv(const GLint handle, const GLfloat data[]) {
	if (handle >= 0)
		glUniformMatrix4fv(handle, 1, GL_FALSE, data);
}

void SetMaterial(int i) {

	glUseProgram(ShadeProg);
	switch (i) {
	case 0: //shiny blue plastic
		glUniform3f(h_uMatAmb, 0.02, 0.04, 0.2);
		glUniform3f(h_uMatDif, 0.0, 0.16, 0.9);
		glUniform3f(h_uMatSpec, 0.14, 0.2, 0.8);
		glUniform1f(h_uMatShine, 120.0);
		break;
	case 1: // flat grey
		glUniform3f(h_uMatAmb, 0.13, 0.13, 0.14);
		glUniform3f(h_uMatDif, 0.3, 0.3, 0.4);
		glUniform3f(h_uMatSpec, 0.3, 0.3, 0.4);
		glUniform1f(h_uMatShine, 4.0);
		break;
	case 2: //gold
		glUniform3f(h_uMatAmb, 0.09, 0.07, 0.08);
		glUniform3f(h_uMatDif, 0.91, 0.782, 0.82);
		glUniform3f(h_uMatSpec, 1.0, 0.913, 0.8);
		glUniform1f(h_uMatShine, 20.0);
		break;
	case 4: // black
		glUniform3f(h_uMatAmb, 0.08, 0.08, 0.08);
		glUniform3f(h_uMatDif, 0.08, 0.08, 0.08);
		glUniform3f(h_uMatSpec, 0.08, 0.08, 0.08);
		glUniform1f(h_uMatShine, 10.0);
		break;
	case 5: // Green
		glUniform3f(h_uMatAmb, 0.00, 0.095, 0.081);
		glUniform3f(h_uMatDif, 0.00, 0.55, 0.41);
		glUniform3f(h_uMatSpec, 0.00, 0.055, 0.041);
		glUniform1f(h_uMatShine, 10.0);
		break;
	case 6: //stars
		glUniform3f(h_uMatAmb, 0.1, 0.1, 0.0);
		glUniform3f(h_uMatDif, 1.0, 1.0, 0.2);
		glUniform3f(h_uMatSpec, 1.0, 1.0, 0.2);
		glUniform1f(h_uMatShine, 200.0);
	case 7: //gun
		glUniform3f(h_uMatAmb, 0.2, 0.1, 0.1);
		glUniform3f(h_uMatDif, .8, 0.1, 0.1);
		glUniform3f(h_uMatSpec, 1, 0.1, 0.1);
		glUniform1f(h_uMatShine, 200.0);
		break;
	case 8: //white
		glUniform3f(h_uMatAmb, 0.5, 0.5, 0.5);
		glUniform3f(h_uMatDif, .7, 0.7, 0.7);
		glUniform3f(h_uMatSpec, 7, 0.7, 0.7);
		glUniform1f(h_uMatShine, 120.0);
		break;
	}
}

/* projection matrix */
void SetProjectionMatrix() {
	glm::mat4 Projection = glm::perspective(60.0f, (float)g_width / g_height, 0.1f, 400.f);
	safe_glUniformMatrix4fv(h_uProjMatrix, glm::value_ptr(Projection));
}

/* camera controls - do not change */
void SetView() {
	glm::mat4 Cam = glm::lookAt(
		position, 
		position + direction,
		vec3(0, 1, 0));
	safe_glUniformMatrix4fv(h_uViewMatrix, glm::value_ptr(Cam));
}

/* model transforms */
void SetModel(vec3 trans, float rot, float sc) {
	glm::mat4 Trans = glm::translate(glm::mat4(1.0f), trans);
	glm::mat4 RotateY = glm::rotate(glm::mat4(1.0f), rot, glm::vec3(0.0f, 1, 0));
	glm::mat4 Sc = glm::scale(glm::mat4(1.0f), vec3(sc));
	glm::mat4 com = Trans*RotateY*Sc;
	safe_glUniformMatrix4fv(h_uModelMatrix, glm::value_ptr(com));
}

void SetModel2(vec3 trans, float rotX, float rotY, float sc) {
	glm::mat4 Trans = glm::translate(glm::mat4(1.0f), trans);
	glm::mat4 RotateX = glm::rotate(glm::mat4(1.0f), rotX, glm::vec3(1, 0, 0));
	glm::mat4 RotateY = glm::rotate(glm::mat4(1.0f), rotY, glm::vec3(0, 1, 0));
	glm::mat4 Sc = glm::scale(glm::mat4(1.0f), vec3(sc));
	glm::mat4 com = Trans*RotateY*RotateX*Sc;
	safe_glUniformMatrix4fv(h_uModelMatrix, glm::value_ptr(com));
}

//Given a vector of shapes which has already been read from an obj file
// resize all vertices to the range [-1, 1]
void resize_obj(std::vector<tinyobj::shape_t> &shapes){
	float minX, minY, minZ;
	float maxX, maxY, maxZ;
	float scaleX, scaleY, scaleZ;
	float shiftX, shiftY, shiftZ;
	float epsilon = 0.001;

	minX = minY = minZ = 1.1754E+38F;
	maxX = maxY = maxZ = -1.1754E+38F;

	//Go through all vertices to determine min and max of each dimension
	for (size_t i = 0; i < shapes.size(); i++) {
		for (size_t v = 0; v < shapes[i].mesh.positions.size() / 3; v++) {
			if (shapes[i].mesh.positions[3 * v + 0] < minX) minX = shapes[i].mesh.positions[3 * v + 0];
			if (shapes[i].mesh.positions[3 * v + 0] > maxX) maxX = shapes[i].mesh.positions[3 * v + 0];

			if (shapes[i].mesh.positions[3 * v + 1] < minY) minY = shapes[i].mesh.positions[3 * v + 1];
			if (shapes[i].mesh.positions[3 * v + 1] > maxY) maxY = shapes[i].mesh.positions[3 * v + 1];

			if (shapes[i].mesh.positions[3 * v + 2] < minZ) minZ = shapes[i].mesh.positions[3 * v + 2];
			if (shapes[i].mesh.positions[3 * v + 2] > maxZ) maxZ = shapes[i].mesh.positions[3 * v + 2];
		}
	}
	//From min and max compute necessary scale and shift for each dimension
	float maxExtent, xExtent, yExtent, zExtent;
	xExtent = maxX - minX;
	yExtent = maxY - minY;
	zExtent = maxZ - minZ;
	if (xExtent >= yExtent && xExtent >= zExtent) {
		maxExtent = xExtent;
	}
	if (yExtent >= xExtent && yExtent >= zExtent) {
		maxExtent = yExtent;
	}
	if (zExtent >= xExtent && zExtent >= yExtent) {
		maxExtent = zExtent;
	}
	scaleX = 2.0 / maxExtent;
	shiftX = minX + (xExtent / 2.0);
	scaleY = 2.0 / maxExtent;
	shiftY = minY + (yExtent / 2.0);
	scaleZ = 2.0 / maxExtent;
	shiftZ = minZ + (zExtent) / 2.0;

	//Go through all verticies shift and scale them
	for (size_t i = 0; i < shapes.size(); i++) {
		for (size_t v = 0; v < shapes[i].mesh.positions.size() / 3; v++) {
			shapes[i].mesh.positions[3 * v + 0] = (shapes[i].mesh.positions[3 * v + 0] - shiftX) * scaleX;
			assert(shapes[i].mesh.positions[3 * v + 0] >= -1.0 - epsilon);
			assert(shapes[i].mesh.positions[3 * v + 0] <= 1.0 + epsilon);
			shapes[i].mesh.positions[3 * v + 1] = (shapes[i].mesh.positions[3 * v + 1] - shiftY) * scaleY;
			assert(shapes[i].mesh.positions[3 * v + 1] >= -1.0 - epsilon);
			assert(shapes[i].mesh.positions[3 * v + 1] <= 1.0 + epsilon);
			shapes[i].mesh.positions[3 * v + 2] = (shapes[i].mesh.positions[3 * v + 2] - shiftZ) * scaleZ;
			assert(shapes[i].mesh.positions[3 * v + 2] >= -1.0 - epsilon);
			assert(shapes[i].mesh.positions[3 * v + 2] <= 1.0 + epsilon);
		}
	}
}

void loadShapes(const string &objFile, std::vector<tinyobj::shape_t>& shapes)
{
	string err = tinyobj::LoadObj(shapes, materials, objFile.c_str());
	if (!err.empty()) {
		cerr << err << endl;
	}
	resize_obj(shapes);
}


void initGodzilla(std::vector<tinyobj::shape_t>& shape) {

	// Send the position array to the GPU
	const vector<float> &posBuf = shape[0].mesh.positions;
	glGenBuffers(1, &posBufObjGodzilla);
	glBindBuffer(GL_ARRAY_BUFFER, posBufObjGodzilla);
	glBufferData(GL_ARRAY_BUFFER, posBuf.size()*sizeof(float), &posBuf[0], GL_STATIC_DRAW);

	// Send the normal array to the GPU
	vector<float> norBuf;
	glm::vec3 v1, v2, v3;
	glm::vec3 edge1, edge2, norm;
	int idx1, idx2, idx3;
	//for every vertex initialize the vertex normal to 0
	for (int j = 0; j < shape[0].mesh.positions.size() / 3; j++) {
		norBuf.push_back(0);
		norBuf.push_back(0);
		norBuf.push_back(0);
	}
	//process the mesh and compute the normals - for every face
	//add its normal to its associated vertex
	for (int i = 0; i < shape[0].mesh.indices.size() / 3; i++) {
		idx1 = shape[0].mesh.indices[3 * i + 0];
		idx2 = shape[0].mesh.indices[3 * i + 1];
		idx3 = shape[0].mesh.indices[3 * i + 2];
		v1 = glm::vec3(shape[0].mesh.positions[3 * idx1 + 0], shape[0].mesh.positions[3 * idx1 + 1], shape[0].mesh.positions[3 * idx1 + 2]);
		v2 = glm::vec3(shape[0].mesh.positions[3 * idx2 + 0], shape[0].mesh.positions[3 * idx2 + 1], shape[0].mesh.positions[3 * idx2 + 2]);
		v3 = glm::vec3(shape[0].mesh.positions[3 * idx3 + 0], shape[0].mesh.positions[3 * idx3 + 1], shape[0].mesh.positions[3 * idx3 + 2]);
		edge1 = v2 - v1;
		edge2 = v3 - v1;
		norm = glm::cross(edge1, edge2);
		norBuf[3 * idx1 + 0] += (norm.x);
		norBuf[3 * idx1 + 1] += (norm.y);
		norBuf[3 * idx1 + 2] += (norm.z);
		norBuf[3 * idx2 + 0] += (norm.x);
		norBuf[3 * idx2 + 1] += (norm.y);
		norBuf[3 * idx2 + 2] += (norm.z);
		norBuf[3 * idx3 + 0] += (norm.x);
		norBuf[3 * idx3 + 1] += (norm.y);
		norBuf[3 * idx3 + 2] += (norm.z);
	}
	glGenBuffers(1, &norBufObjGodzilla);
	glBindBuffer(GL_ARRAY_BUFFER, norBufObjGodzilla);
	glBufferData(GL_ARRAY_BUFFER, norBuf.size()*sizeof(float), &norBuf[0], GL_STATIC_DRAW);

	// Send the index array to the GPU
	const vector<unsigned int> &indBuf = shape[0].mesh.indices;
	glGenBuffers(1, &indBufObjGodzilla);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufObjGodzilla);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indBuf.size()*sizeof(unsigned int), &indBuf[0], GL_STATIC_DRAW);

	// Unbind the arrays
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	GLSL::checkVersion();
	assert(glGetError() == GL_NO_ERROR);
}

void initMine(std::vector<tinyobj::shape_t>& shape) {

	// Send the position array to the GPU
	const vector<float> &posBuf = shape[0].mesh.positions;
	glGenBuffers(1, &posBufObjMine);
	glBindBuffer(GL_ARRAY_BUFFER, posBufObjMine);
	glBufferData(GL_ARRAY_BUFFER, posBuf.size()*sizeof(float), &posBuf[0], GL_STATIC_DRAW);

	// Send the normal array to the GPU
	vector<float> norBuf;
	glm::vec3 v1, v2, v3;
	glm::vec3 edge1, edge2, norm;
	int idx1, idx2, idx3;
	//for every vertex initialize the vertex normal to 0
	for (int j = 0; j < shape[0].mesh.positions.size() / 3; j++) {
		norBuf.push_back(0);
		norBuf.push_back(0);
		norBuf.push_back(0);
	}
	//process the mesh and compute the normals - for every face
	//add its normal to its associated vertex
	for (int i = 0; i < shape[0].mesh.indices.size() / 3; i++) {
		idx1 = shape[0].mesh.indices[3 * i + 0];
		idx2 = shape[0].mesh.indices[3 * i + 1];
		idx3 = shape[0].mesh.indices[3 * i + 2];
		v1 = glm::vec3(shape[0].mesh.positions[3 * idx1 + 0], shape[0].mesh.positions[3 * idx1 + 1], shape[0].mesh.positions[3 * idx1 + 2]);
		v2 = glm::vec3(shape[0].mesh.positions[3 * idx2 + 0], shape[0].mesh.positions[3 * idx2 + 1], shape[0].mesh.positions[3 * idx2 + 2]);
		v3 = glm::vec3(shape[0].mesh.positions[3 * idx3 + 0], shape[0].mesh.positions[3 * idx3 + 1], shape[0].mesh.positions[3 * idx3 + 2]);
		edge1 = v2 - v1;
		edge2 = v3 - v1;
		norm = glm::cross(edge1, edge2);
		norBuf[3 * idx1 + 0] += (norm.x);
		norBuf[3 * idx1 + 1] += (norm.y);
		norBuf[3 * idx1 + 2] += (norm.z);
		norBuf[3 * idx2 + 0] += (norm.x);
		norBuf[3 * idx2 + 1] += (norm.y);
		norBuf[3 * idx2 + 2] += (norm.z);
		norBuf[3 * idx3 + 0] += (norm.x);
		norBuf[3 * idx3 + 1] += (norm.y);
		norBuf[3 * idx3 + 2] += (norm.z);
	}
	glGenBuffers(1, &norBufObjMine);
	glBindBuffer(GL_ARRAY_BUFFER, norBufObjMine);
	glBufferData(GL_ARRAY_BUFFER, norBuf.size()*sizeof(float), &norBuf[0], GL_STATIC_DRAW);

	// Send the index array to the GPU
	const vector<unsigned int> &indBuf = shape[0].mesh.indices;
	glGenBuffers(1, &indBufObjMine);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufObjMine);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indBuf.size()*sizeof(unsigned int), &indBuf[0], GL_STATIC_DRAW);

	// Unbind the arrays
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	GLSL::checkVersion();
	assert(glGetError() == GL_NO_ERROR);
}

void initFlag(std::vector<tinyobj::shape_t>& shape) {

	// Send the position array to the GPU
	const vector<float> &posBuf = shape[0].mesh.positions;
	glGenBuffers(1, &posBufObjFlag);
	glBindBuffer(GL_ARRAY_BUFFER, posBufObjFlag);
	glBufferData(GL_ARRAY_BUFFER, posBuf.size()*sizeof(float), &posBuf[0], GL_STATIC_DRAW);

	// Send the normal array to the GPU
	vector<float> norBuf;
	glm::vec3 v1, v2, v3;
	glm::vec3 edge1, edge2, norm;
	int idx1, idx2, idx3;
	//for every vertex initialize the vertex normal to 0
	for (int j = 0; j < shape[0].mesh.positions.size() / 3; j++) {
		norBuf.push_back(0);
		norBuf.push_back(0);
		norBuf.push_back(0);
	}
	//process the mesh and compute the normals - for every face
	//add its normal to its associated vertex
	for (int i = 0; i < shape[0].mesh.indices.size() / 3; i++) {
		idx1 = shape[0].mesh.indices[3 * i + 0];
		idx2 = shape[0].mesh.indices[3 * i + 1];
		idx3 = shape[0].mesh.indices[3 * i + 2];
		v1 = glm::vec3(shape[0].mesh.positions[3 * idx1 + 0], shape[0].mesh.positions[3 * idx1 + 1], shape[0].mesh.positions[3 * idx1 + 2]);
		v2 = glm::vec3(shape[0].mesh.positions[3 * idx2 + 0], shape[0].mesh.positions[3 * idx2 + 1], shape[0].mesh.positions[3 * idx2 + 2]);
		v3 = glm::vec3(shape[0].mesh.positions[3 * idx3 + 0], shape[0].mesh.positions[3 * idx3 + 1], shape[0].mesh.positions[3 * idx3 + 2]);
		edge1 = v2 - v1;
		edge2 = v3 - v1;
		norm = glm::cross(edge1, edge2);
		norBuf[3 * idx1 + 0] += (norm.x);
		norBuf[3 * idx1 + 1] += (norm.y);
		norBuf[3 * idx1 + 2] += (norm.z);
		norBuf[3 * idx2 + 0] += (norm.x);
		norBuf[3 * idx2 + 1] += (norm.y);
		norBuf[3 * idx2 + 2] += (norm.z);
		norBuf[3 * idx3 + 0] += (norm.x);
		norBuf[3 * idx3 + 1] += (norm.y);
		norBuf[3 * idx3 + 2] += (norm.z);
	}
	glGenBuffers(1, &norBufObjFlag);
	glBindBuffer(GL_ARRAY_BUFFER, norBufObjFlag);
	glBufferData(GL_ARRAY_BUFFER, norBuf.size()*sizeof(float), &norBuf[0], GL_STATIC_DRAW);

	// Send the index array to the GPU
	const vector<unsigned int> &indBuf = shape[0].mesh.indices;
	glGenBuffers(1, &indBufObjFlag);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufObjFlag);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indBuf.size()*sizeof(unsigned int), &indBuf[0], GL_STATIC_DRAW);

	// Unbind the arrays
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	GLSL::checkVersion();
	assert(glGetError() == GL_NO_ERROR);
}

void initBullet(std::vector<tinyobj::shape_t>& shape) {

	// Send the position array to the GPU
	const vector<float> &posBuf = shape[0].mesh.positions;
	glGenBuffers(1, &posBufObjBullet);
	glBindBuffer(GL_ARRAY_BUFFER, posBufObjBullet);
	glBufferData(GL_ARRAY_BUFFER, posBuf.size()*sizeof(float), &posBuf[0], GL_STATIC_DRAW);

	// Send the normal array to the GPU
	vector<float> norBuf;
	glm::vec3 v1, v2, v3;
	glm::vec3 edge1, edge2, norm;
	int idx1, idx2, idx3;
	//for every vertex initialize the vertex normal to 0
	for (int j = 0; j < shape[0].mesh.positions.size() / 3; j++) {
		norBuf.push_back(0);
		norBuf.push_back(0);
		norBuf.push_back(0);
	}
	//process the mesh and compute the normals - for every face
	//add its normal to its associated vertex
	for (int i = 0; i < shape[0].mesh.indices.size() / 3; i++) {
		idx1 = shape[0].mesh.indices[3 * i + 0];
		idx2 = shape[0].mesh.indices[3 * i + 1];
		idx3 = shape[0].mesh.indices[3 * i + 2];
		v1 = glm::vec3(shape[0].mesh.positions[3 * idx1 + 0], shape[0].mesh.positions[3 * idx1 + 1], shape[0].mesh.positions[3 * idx1 + 2]);
		v2 = glm::vec3(shape[0].mesh.positions[3 * idx2 + 0], shape[0].mesh.positions[3 * idx2 + 1], shape[0].mesh.positions[3 * idx2 + 2]);
		v3 = glm::vec3(shape[0].mesh.positions[3 * idx3 + 0], shape[0].mesh.positions[3 * idx3 + 1], shape[0].mesh.positions[3 * idx3 + 2]);
		edge1 = v2 - v1;
		edge2 = v3 - v1;
		norm = glm::cross(edge1, edge2);
		norBuf[3 * idx1 + 0] += (norm.x);
		norBuf[3 * idx1 + 1] += (norm.y);
		norBuf[3 * idx1 + 2] += (norm.z);
		norBuf[3 * idx2 + 0] += (norm.x);
		norBuf[3 * idx2 + 1] += (norm.y);
		norBuf[3 * idx2 + 2] += (norm.z);
		norBuf[3 * idx3 + 0] += (norm.x);
		norBuf[3 * idx3 + 1] += (norm.y);
		norBuf[3 * idx3 + 2] += (norm.z);
	}
	glGenBuffers(1, &norBufObjBullet);
	glBindBuffer(GL_ARRAY_BUFFER, norBufObjBullet);
	glBufferData(GL_ARRAY_BUFFER, norBuf.size()*sizeof(float), &norBuf[0], GL_STATIC_DRAW);

	// Send the index array to the GPU
	const vector<unsigned int> &indBuf = shape[0].mesh.indices;
	glGenBuffers(1, &indBufObjBullet);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufObjBullet);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indBuf.size()*sizeof(unsigned int), &indBuf[0], GL_STATIC_DRAW);

	// Unbind the arrays
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	GLSL::checkVersion();
	assert(glGetError() == GL_NO_ERROR);
}

void initStar(std::vector<tinyobj::shape_t>& shape) {
	// Send the position array to the GPU
	const vector<float> &posBuf = shape[0].mesh.positions;
	glGenBuffers(1, &posBufObjStar);
	glBindBuffer(GL_ARRAY_BUFFER, posBufObjStar);
	glBufferData(GL_ARRAY_BUFFER, posBuf.size()*sizeof(float), &posBuf[0], GL_STATIC_DRAW);

	// Send the normal array to the GPU
	vector<float> norBuf;
	glm::vec3 v1, v2, v3;
	glm::vec3 edge1, edge2, norm;
	int idx1, idx2, idx3;
	//for every vertex initialize the vertex normal to 0
	for (int j = 0; j < shape[0].mesh.positions.size() / 3; j++) {
		norBuf.push_back(0);
		norBuf.push_back(0);
		norBuf.push_back(0);
	}
	//process the mesh and compute the normals - for every face
	//add its normal to its associated vertex
	for (int i = 0; i < shape[0].mesh.indices.size() / 3; i++) {
		idx1 = shape[0].mesh.indices[3 * i + 0];
		idx2 = shape[0].mesh.indices[3 * i + 1];
		idx3 = shape[0].mesh.indices[3 * i + 2];
		v1 = glm::vec3(shape[0].mesh.positions[3 * idx1 + 0], shape[0].mesh.positions[3 * idx1 + 1], shape[0].mesh.positions[3 * idx1 + 2]);
		v2 = glm::vec3(shape[0].mesh.positions[3 * idx2 + 0], shape[0].mesh.positions[3 * idx2 + 1], shape[0].mesh.positions[3 * idx2 + 2]);
		v3 = glm::vec3(shape[0].mesh.positions[3 * idx3 + 0], shape[0].mesh.positions[3 * idx3 + 1], shape[0].mesh.positions[3 * idx3 + 2]);
		edge1 = v2 - v1;
		edge2 = v3 - v1;
		norm = glm::cross(edge1, edge2);
		norBuf[3 * idx1 + 0] += (norm.x);
		norBuf[3 * idx1 + 1] += (norm.y);
		norBuf[3 * idx1 + 2] += (norm.z);
		norBuf[3 * idx2 + 0] += (norm.x);
		norBuf[3 * idx2 + 1] += (norm.y);
		norBuf[3 * idx2 + 2] += (norm.z);
		norBuf[3 * idx3 + 0] += (norm.x);
		norBuf[3 * idx3 + 1] += (norm.y);
		norBuf[3 * idx3 + 2] += (norm.z);
	}
	glGenBuffers(1, &norBufObjStar);
	glBindBuffer(GL_ARRAY_BUFFER, norBufObjStar);
	glBufferData(GL_ARRAY_BUFFER, norBuf.size()*sizeof(float), &norBuf[0], GL_STATIC_DRAW);

	// Send the index array to the GPU
	const vector<unsigned int> &indBuf = shape[0].mesh.indices;
	glGenBuffers(1, &indBufObjStar);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufObjStar);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indBuf.size()*sizeof(unsigned int), &indBuf[0], GL_STATIC_DRAW);

	// Unbind the arrays
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	GLSL::checkVersion();
	assert(glGetError() == GL_NO_ERROR);
}

void initGun(std::vector<tinyobj::shape_t>& shape) {
	// Send the position array to the GPU
	const vector<float> &posBuf = shape[0].mesh.positions;
	glGenBuffers(1, &posBufObjGun);
	glBindBuffer(GL_ARRAY_BUFFER, posBufObjGun);
	glBufferData(GL_ARRAY_BUFFER, posBuf.size()*sizeof(float), &posBuf[0], GL_STATIC_DRAW);

	// Send the normal array to the GPU
	vector<float> norBuf;
	glm::vec3 v1, v2, v3;
	glm::vec3 edge1, edge2, norm;
	int idx1, idx2, idx3;
	//for every vertex initialize the vertex normal to 0
	for (int j = 0; j < shape[0].mesh.positions.size() / 3; j++) {
		norBuf.push_back(0);
		norBuf.push_back(0);
		norBuf.push_back(0);
	}
	//process the mesh and compute the normals - for every face
	//add its normal to its associated vertex
	for (int i = 0; i < shape[0].mesh.indices.size() / 3; i++) {
		idx1 = shape[0].mesh.indices[3 * i + 0];
		idx2 = shape[0].mesh.indices[3 * i + 1];
		idx3 = shape[0].mesh.indices[3 * i + 2];
		v1 = glm::vec3(shape[0].mesh.positions[3 * idx1 + 0], shape[0].mesh.positions[3 * idx1 + 1], shape[0].mesh.positions[3 * idx1 + 2]);
		v2 = glm::vec3(shape[0].mesh.positions[3 * idx2 + 0], shape[0].mesh.positions[3 * idx2 + 1], shape[0].mesh.positions[3 * idx2 + 2]);
		v3 = glm::vec3(shape[0].mesh.positions[3 * idx3 + 0], shape[0].mesh.positions[3 * idx3 + 1], shape[0].mesh.positions[3 * idx3 + 2]);
		edge1 = v2 - v1;
		edge2 = v3 - v1;
		norm = glm::cross(edge1, edge2);
		norBuf[3 * idx1 + 0] += (norm.x);
		norBuf[3 * idx1 + 1] += (norm.y);
		norBuf[3 * idx1 + 2] += (norm.z);
		norBuf[3 * idx2 + 0] += (norm.x);
		norBuf[3 * idx2 + 1] += (norm.y);
		norBuf[3 * idx2 + 2] += (norm.z);
		norBuf[3 * idx3 + 0] += (norm.x);
		norBuf[3 * idx3 + 1] += (norm.y);
		norBuf[3 * idx3 + 2] += (norm.z);
	}
	glGenBuffers(1, &norBufObjGun);
	glBindBuffer(GL_ARRAY_BUFFER, norBufObjGun);
	glBufferData(GL_ARRAY_BUFFER, norBuf.size()*sizeof(float), &norBuf[0], GL_STATIC_DRAW);

	// Send the index array to the GPU
	const vector<unsigned int> &indBuf = shape[0].mesh.indices;
	glGenBuffers(1, &indBufObjGun);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufObjGun);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indBuf.size()*sizeof(unsigned int), &indBuf[0], GL_STATIC_DRAW);

	// Unbind the arrays
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	GLSL::checkVersion();
	assert(glGetError() == GL_NO_ERROR);
}

void initCube(std::vector<tinyobj::shape_t>& shape) {
	// Send the position array to the GPU
	const vector<float> &posBuf = shape[0].mesh.positions;
	glGenBuffers(1, &posBufObjCube);
	glBindBuffer(GL_ARRAY_BUFFER, posBufObjCube);
	glBufferData(GL_ARRAY_BUFFER, posBuf.size()*sizeof(float), &posBuf[0], GL_STATIC_DRAW);

	// Send the normal array to the GPU
	vector<float> norBuf;
	glm::vec3 v1, v2, v3;
	glm::vec3 edge1, edge2, norm;
	int idx1, idx2, idx3;
	//for every vertex initialize the vertex normal to 0
	for (int j = 0; j < shape[0].mesh.positions.size() / 3; j++) {
		norBuf.push_back(0);
		norBuf.push_back(0);
		norBuf.push_back(0);
	}
	//process the mesh and compute the normals - for every face
	//add its normal to its associated vertex
	for (int i = 0; i < shape[0].mesh.indices.size() / 3; i++) {
		idx1 = shape[0].mesh.indices[3 * i + 0];
		idx2 = shape[0].mesh.indices[3 * i + 1];
		idx3 = shape[0].mesh.indices[3 * i + 2];
		v1 = glm::vec3(shape[0].mesh.positions[3 * idx1 + 0], shape[0].mesh.positions[3 * idx1 + 1], shape[0].mesh.positions[3 * idx1 + 2]);
		v2 = glm::vec3(shape[0].mesh.positions[3 * idx2 + 0], shape[0].mesh.positions[3 * idx2 + 1], shape[0].mesh.positions[3 * idx2 + 2]);
		v3 = glm::vec3(shape[0].mesh.positions[3 * idx3 + 0], shape[0].mesh.positions[3 * idx3 + 1], shape[0].mesh.positions[3 * idx3 + 2]);
		edge1 = v2 - v1;
		edge2 = v3 - v1;
		norm = glm::cross(edge1, edge2);
		norBuf[3 * idx1 + 0] += (norm.x);
		norBuf[3 * idx1 + 1] += (norm.y);
		norBuf[3 * idx1 + 2] += (norm.z);
		norBuf[3 * idx2 + 0] += (norm.x);
		norBuf[3 * idx2 + 1] += (norm.y);
		norBuf[3 * idx2 + 2] += (norm.z);
		norBuf[3 * idx3 + 0] += (norm.x);
		norBuf[3 * idx3 + 1] += (norm.y);
		norBuf[3 * idx3 + 2] += (norm.z);
	}
	glGenBuffers(1, &norBufObjCube);
	glBindBuffer(GL_ARRAY_BUFFER, norBufObjCube);
	glBufferData(GL_ARRAY_BUFFER, norBuf.size()*sizeof(float), &norBuf[0], GL_STATIC_DRAW);

	// Send the index array to the GPU
	const vector<unsigned int> &indBuf = shape[0].mesh.indices;
	glGenBuffers(1, &indBufObjCube);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufObjCube);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indBuf.size()*sizeof(unsigned int), &indBuf[0], GL_STATIC_DRAW);

	// Unbind the arrays
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	GLSL::checkVersion();
	assert(glGetError() == GL_NO_ERROR);
}

void initGround() {

	float G_edge = 40;
	GLfloat g_backgnd_data[] = {
		-G_edge, -1.0f, -G_edge,
		-G_edge, -1.0f, G_edge,
		G_edge, -1.0f, -G_edge,
		-G_edge, -1.0f, G_edge,
		G_edge, -1.0f, -G_edge,
		G_edge, -1.0f, G_edge,
	};


	GLfloat nor_Buf_G[] = {
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
	};

	glGenBuffers(1, &posBufObjG);
	glBindBuffer(GL_ARRAY_BUFFER, posBufObjG);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_backgnd_data), g_backgnd_data, GL_STATIC_DRAW);

	glGenBuffers(1, &norBufObjG);
	glBindBuffer(GL_ARRAY_BUFFER, norBufObjG);
	glBufferData(GL_ARRAY_BUFFER, sizeof(nor_Buf_G), nor_Buf_G, GL_STATIC_DRAW);

}

void initGL()
{
	// Set the background color
	glClearColor(0.6f, 0.6f, 0.8f, 1.0f);
	// Enable Z-buffer test
	glEnable(GL_DEPTH_TEST);
	glPointSize(18);

	initGodzilla(godzilla);
	initBullet(bullet);
	initFlag(flag);
	initMine(mine);
	initCube(cube);
	initGun(gun);
	initStar(star);
	initGround();
}

bool installShaders(const string &vShaderName, const string &fShaderName)
{
	GLint rc;

	// Create shader handles
	GLuint VS = glCreateShader(GL_VERTEX_SHADER);
	GLuint FS = glCreateShader(GL_FRAGMENT_SHADER);

	// Read shader sources
	const char *vshader = GLSL::textFileRead(vShaderName.c_str());
	const char *fshader = GLSL::textFileRead(fShaderName.c_str());
	glShaderSource(VS, 1, &vshader, NULL);
	glShaderSource(FS, 1, &fshader, NULL);

	// Compile vertex shader
	glCompileShader(VS);
	std::cout << "just compiled the v shader" << std::endl;
	printOglError(__FILE__, __LINE__);
	GLSL::printError();
	glGetShaderiv(VS, GL_COMPILE_STATUS, &rc);
	GLSL::printShaderInfoLog(VS);
	if (!rc) {
		printf("Error compiling vertex shader %s\n", vShaderName.c_str());
		return false;
	}

	// Compile fragment shader
	glCompileShader(FS);
	std::cout << "just compiled the f shader" << std::endl;
	GLSL::printError();
	glGetShaderiv(FS, GL_COMPILE_STATUS, &rc);
	GLSL::printShaderInfoLog(FS);
	if (!rc) {
		printf("Error compiling fragment shader %s\n", fShaderName.c_str());
		return false;
	}

	// Create the program and link
	ShadeProg = glCreateProgram();
	glAttachShader(ShadeProg, VS);
	glAttachShader(ShadeProg, FS);
	glLinkProgram(ShadeProg);
	std::cout << "just linked the shaders" << std::endl;

	GLSL::printError();
	glGetProgramiv(ShadeProg, GL_LINK_STATUS, &rc);
	GLSL::printProgramInfoLog(ShadeProg);
	if (!rc) {
		printf("Error linking shaders %s and %s\n", vShaderName.c_str(), fShaderName.c_str());
		return false;
	}

	/* get handles to attribute data */
	h_aPosition = GLSL::getAttribLocation(ShadeProg, "aPosition");
	h_aNormal = GLSL::getAttribLocation(ShadeProg, "aNormal");
	h_uProjMatrix = GLSL::getUniformLocation(ShadeProg, "uProjMatrix");
	h_uViewMatrix = GLSL::getUniformLocation(ShadeProg, "uViewMatrix");
	h_uModelMatrix = GLSL::getUniformLocation(ShadeProg, "uModelMatrix");
	h_uLightPos = GLSL::getUniformLocation(ShadeProg, "uLightPos");
	h_uMatAmb = GLSL::getUniformLocation(ShadeProg, "UaColor");
	h_uMatDif = GLSL::getUniformLocation(ShadeProg, "UdColor");
	h_uMatSpec = GLSL::getUniformLocation(ShadeProg, "UsColor");
	h_uMatShine = GLSL::getUniformLocation(ShadeProg, "Ushine");
	h_uShadeM = GLSL::getUniformLocation(ShadeProg, "uShadeModel");
	h_uCam = GLSL::getUniformLocation(ShadeProg, "cam_pos");

	assert(glGetError() == GL_NO_ERROR);
	return true;
}

void drawStar(vec3 trans, float scale, int mat) {
	//draw a star
	SetMaterial(mat);

	SetModel(trans, 0, scale);

	// Enable and bind position array for drawing
	GLSL::enableVertexAttribArray(h_aPosition);
	glBindBuffer(GL_ARRAY_BUFFER, posBufObjStar);
	glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Enable and bind normal array for drawing
	GLSL::enableVertexAttribArray(h_aNormal);
	glBindBuffer(GL_ARRAY_BUFFER, norBufObjStar);
	glVertexAttribPointer(h_aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Bind index array for drawing
	int nIndices = (int)star[0].mesh.indices.size();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufObjStar);

	//draw the godzilla	
	glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);
}

void drawGodzilla(vec3 trans, float rotate, float scale) {
	//draw a godzilla
	SetMaterial(5);
	SetModel(trans, rotate, scale);

	// Enable and bind position array for drawing
	GLSL::enableVertexAttribArray(h_aPosition);
	glBindBuffer(GL_ARRAY_BUFFER, posBufObjGodzilla);
	glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Enable and bind normal array for drawing
	GLSL::enableVertexAttribArray(h_aNormal);
	glBindBuffer(GL_ARRAY_BUFFER, norBufObjGodzilla);
	glVertexAttribPointer(h_aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Bind index array for drawing
	int nIndices = (int)godzilla[0].mesh.indices.size();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufObjGodzilla);

	//draw the godzilla	
	glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);
}

void drawBullet(vec3 trans, float rotateX, float rotateY) {
	//draw a bullet
	SetMaterial(7);
	SetModel2(trans, rotateX, rotateY, .10);

	// Enable and bind position array for drawing
	GLSL::enableVertexAttribArray(h_aPosition);
	glBindBuffer(GL_ARRAY_BUFFER, posBufObjBullet);
	glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Enable and bind normal array for drawing
	GLSL::enableVertexAttribArray(h_aNormal);
	glBindBuffer(GL_ARRAY_BUFFER, norBufObjBullet);
	glVertexAttribPointer(h_aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Bind index array for drawing
	int nIndices = (int)bullet[0].mesh.indices.size();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufObjBullet);

	//draw the bullet
	glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);
}

void drawMine(vec3 trans, float scale) {
	SetMaterial(7);
	SetModel(trans, 0, scale);

	// Enable and bind position array for drawing
	GLSL::enableVertexAttribArray(h_aPosition);
	glBindBuffer(GL_ARRAY_BUFFER, posBufObjMine);
	glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Enable and bind normal array for drawing
	GLSL::enableVertexAttribArray(h_aNormal);
	glBindBuffer(GL_ARRAY_BUFFER, norBufObjMine);
	glVertexAttribPointer(h_aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Bind index array for drawing
	int nIndices = (int)mine[0].mesh.indices.size();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufObjMine);

	//draw the mine
	glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);
}

void drawFlag(vec3 trans, float rotate, float scale) {
	if (godzillaPos.empty())
		SetMaterial(7);
	else
		SetMaterial(0);
	if (playerDead)
		SetMaterial(8);
	SetModel(trans, rotate, scale);

	// Enable and bind position array for drawing
	GLSL::enableVertexAttribArray(h_aPosition);
	glBindBuffer(GL_ARRAY_BUFFER, posBufObjFlag);
	glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Enable and bind normal array for drawing
	GLSL::enableVertexAttribArray(h_aNormal);
	glBindBuffer(GL_ARRAY_BUFFER, norBufObjFlag);
	glVertexAttribPointer(h_aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Bind index array for drawing
	int nIndices = (int)flag[0].mesh.indices.size();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufObjFlag);

	//draw the flag
	glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);
}

void drawGun(vec3 trans, float rotateX, float rotateY, float scale) {
	//draw a gun
	SetMaterial(7);

	SetModel2(trans, rotateX, rotateY, scale);

	// Enable and bind position array for drawing
	GLSL::enableVertexAttribArray(h_aPosition);
	glBindBuffer(GL_ARRAY_BUFFER, posBufObjGun);
	glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Enable and bind normal array for drawing
	GLSL::enableVertexAttribArray(h_aNormal);
	glBindBuffer(GL_ARRAY_BUFFER, norBufObjGun);
	glVertexAttribPointer(h_aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Bind index array for drawing
	int nIndices = (int)gun[0].mesh.indices.size();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufObjGun);

	//draw the gun
	glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);
}

void drawWalls() {
	SetMaterial(4);
	SetModel(vec3(0, 0, 0), 0, 40);

	// Enable and bind position array for drawing
	GLSL::enableVertexAttribArray(h_aPosition);
	glBindBuffer(GL_ARRAY_BUFFER, posBufObjCube);
	glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Enable and bind normal array for drawing
	GLSL::enableVertexAttribArray(h_aNormal);
	glBindBuffer(GL_ARRAY_BUFFER, norBufObjCube);
	glVertexAttribPointer(h_aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Bind index array for drawing
	int nIndices = (int)cube[0].mesh.indices.size();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufObjCube);

	//draw the cube
	glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);
}


void drawTower(vec3 trans, float rotate, float scale, int material) {
	//draw a cube
	SetMaterial(0);
	SetModel(trans, rotate, scale);

	// Enable and bind position array for drawing
	GLSL::enableVertexAttribArray(h_aPosition);
	glBindBuffer(GL_ARRAY_BUFFER, posBufObjCube);
	glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Enable and bind normal array for drawing
	GLSL::enableVertexAttribArray(h_aNormal);
	glBindBuffer(GL_ARRAY_BUFFER, norBufObjCube);
	glVertexAttribPointer(h_aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Bind index array for drawing
	int nIndices = (int)cube[0].mesh.indices.size();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufObjCube);

	//draw the cube
	glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);

	SetModel(glm::vec3(trans.x, 2.5, trans.z), rotate, scale/1.5);

	// Enable and bind position array for drawing
	GLSL::enableVertexAttribArray(h_aPosition);
	glBindBuffer(GL_ARRAY_BUFFER, posBufObjCube);
	glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Enable and bind normal array for drawing
	GLSL::enableVertexAttribArray(h_aNormal);
	glBindBuffer(GL_ARRAY_BUFFER, norBufObjCube);
	glVertexAttribPointer(h_aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Bind index array for drawing
	nIndices = (int)cube[0].mesh.indices.size();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufObjCube);

	//draw the cube
	glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);

	SetModel(glm::vec3(trans.x, 4, trans.z), rotate, scale / 2.5);

	// Enable and bind position array for drawing
	GLSL::enableVertexAttribArray(h_aPosition);
	glBindBuffer(GL_ARRAY_BUFFER, posBufObjCube);
	glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Enable and bind normal array for drawing
	GLSL::enableVertexAttribArray(h_aNormal);
	glBindBuffer(GL_ARRAY_BUFFER, norBufObjCube);
	glVertexAttribPointer(h_aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Bind index array for drawing
	nIndices = (int)cube[0].mesh.indices.size();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufObjCube);

	//draw the cube
	glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);
}

void drawGL()
{
	// Clear the screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Use our GLSL program
	glUseProgram(ShadeProg);
	glUniform3f(h_uLightPos, g_light.x, g_light.y, g_light.z);
	glUniform1i(h_uShadeM, g_SM);
	glUniform3f(h_uCam, position.x, position.y, position.z);

	SetProjectionMatrix();

	double theta, phi;
	glfwGetCursorPos(window, &theta, &phi);
	float horizontal = .005 * float(g_width / 2 - theta);
	float vertical = .005 * float(g_height / 2 - phi);

	if (vertical > 0.7 && !playerDead) {
		vertical = 0.7;
	}
	else if (vertical < -0.45 && !playerDead) {
		vertical = -0.45;
	}
	else if (vertical > 1.54 && playerDead) {
		vertical = 1.54;
	}
	else if (vertical < -1.54 && playerDead) {
		vertical = -1.54;
	}

	float x = cos(vertical) * sin(horizontal);
	float y = sin(vertical);
	float z = cos(vertical) * cos(horizontal);
	direction = glm::vec3(x, y, z);
	rightV = glm::vec3(sin(horizontal - 3.14 / 2.0), 0, cos(horizontal - 3.14 / 2.0));
	SetView();

	if (playerDead && position.y < 35) {
		position += vec3(0, 1, 0) * cameraSpeed;
	}

	if (playerDead) {
		drawFlag(vec3(0.1, -2.5, 0.1), 0, 6.0);
	}
	else {
		drawFlag(vec3(0.1, 3.3, 0.1), 0, 6.0);
	}

	for (int i = 0; i < godzillaPos.size(); i++) {
		vec3 godzillaNormal(0, 0, -1);
		vec3 direction = normalize(position - godzillaPos[i]);
		vec3 adjust(0, 0.0, 0);
		godzillaPos[i].x += direction.x * godzillaSpeed;
		godzillaPos[i].z += direction.z * godzillaSpeed;

		if (godzillaPos[i].y >= 0.5) {
			adjust.y -= 0.5;
		}

		for (int j = 0; j < godzillaPos.size(); j++) {
			if (i != j) {
				if (length(godzillaPos[i] - godzillaPos[j]) <= godzillaSpread) {
					vec3 adjustDirection = normalize(godzillaPos[i] - godzillaPos[j]);
					adjust.x += adjustDirection.x;
					adjust.z += adjustDirection.z;
				}
			}
		}
		for (int j = 0; j < towerPos.size(); j++) {
			if (length(godzillaPos[i] - towerPos[j]) <= towerSpread) {
				vec3 adjustDirection = normalize(godzillaPos[i] - towerPos[j]);
				adjust.x += adjustDirection.x;
				adjust.z += adjustDirection.z;
				adjust.y += abs(adjust.x + adjust.z) * 2;
			}
		}
		adjust.x *= getRand(0.50, 1.0);
		adjust.z *= getRand(0.50, 1.0);

		godzillaPos[i] += adjust * godzillaSpeed;

		float dot = direction.x * godzillaNormal.x + direction.z * godzillaNormal.z;
		float det = direction.x * godzillaNormal.z - direction.z * godzillaNormal.x;
		float radians = atan2(det, dot);
		int angle = (int)(180 * radians / M_PI);

		drawGodzilla(godzillaPos[i], angle, 1.5);
	}

	for (int i = 0; i < towerPos.size(); i++) {
		drawTower(towerPos[i], towerRot[i], 2, towerMat[i]);
	}

	for (int i = 0; i < starPos.size(); i++) {
		drawStar(starPos[i], 0.3, getRand(0, 7.1));
	}

	for (int i = 0; i < bulletPos.size(); i++) {
		bulletPos[i] += normalize(bulletDirection[i]) * bulletSpeed;
		drawBullet(bulletPos[i], bulletRotX[i], bulletRotY[i]);
		if (length(bulletPos[i]) > maxBulletDistance) {
			bulletPos.erase(bulletPos.begin() + i);
			bulletDirection.erase(bulletDirection.begin() + i);
			bulletRotX.erase(bulletRotX.begin() + i);
			bulletRotY.erase(bulletRotY.begin() + i);
		}
	}

	for (int i = 0; i < minePos.size(); i++) {
		drawMine(minePos[i], 0.5);
	}

	drawWalls();
	drawStar(g_light + vec3(0, 1, 0), 3, 6);

	if (!playerDead) {
		drawGun(position - vec3(0.01, 0.3, 0.01), -(180 * vertical / M_PI), (180 * horizontal / M_PI), .7);
	}

	//draw the ground
	SetMaterial(4);
	SetModel(vec3(0), 0, 1);
	glEnableVertexAttribArray(h_aPosition);
	glBindBuffer(GL_ARRAY_BUFFER, posBufObjG);
	glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	GLSL::enableVertexAttribArray(h_aNormal);
	glBindBuffer(GL_ARRAY_BUFFER, norBufObjG);
	glVertexAttribPointer(h_aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	GLSL::disableVertexAttribArray(h_aPosition);
	GLSL::disableVertexAttribArray(h_aNormal);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
	assert(glGetError() == GL_NO_ERROR);
}

void window_size_callback(GLFWwindow* window, int w, int h){
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	g_width = w;
	g_height = h;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS &&
		(position.x <= spawnCircleRadius && position.x >= -spawnCircleRadius) &&
		(position.z <= spawnCircleRadius && position.z >= -spawnCircleRadius) &&
		godzillaPos.empty()) {
		if (godzillaRadius >= minGodzillaSpawn) {
			godzillaRadius--;
		}
		else {
			godzillaSpeed += godzillaSpeedIncrease;
			numGodzillas++;
		}
		g_light.x--;
		g_light.z++;
		addGodzillas(numGodzillas);
		numMinesLeft = 5;
		addStar();
	}
	else if (playerDead && key == GLFW_KEY_ENTER && action == GLFW_PRESS) {
		playerDead = false;
		g_light = vec3(30, 30, 0);
		position = vec3(0, 0, 0);
		godzillaPos.clear();
		godzillaSpeed = 0.01;
		godzillaRadius = 38;
		towerPos.clear();
		addTowers(numTowers);
		minePos.clear();
		starPos.clear();
		numMinesLeft = 5;
	}
	else if (key == GLFW_KEY_Q && action == GLFW_PRESS) {
		g_light += vec3(1, 0, -1);
	}
	else if (key == GLFW_KEY_E && action == GLFW_PRESS) {
		g_light += vec3(-1, 0, 1);
	}
}

void mouse_callback(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_1 && action == GLFW_RELEASE && !playerDead) {
		addBullet();
	}
	else if (button == GLFW_MOUSE_BUTTON_2 && action == GLFW_RELEASE && 
		numMinesLeft > 0 && !playerDead) {
		addMine();
		numMinesLeft--;
	}
}

int main(int argc, char **argv)
{
	srand(time(NULL));

	// Initialise GLFW
	if (!glfwInit()) {
		fprintf(stderr, "Failed to initialize GLFW\n");
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

	// Open a window and create its OpenGL context
	g_width = 1200;
	g_height = 720;
	window = glfwCreateWindow(g_width, g_height, "godzilla and ground", NULL, NULL);
	if (window == NULL){
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetKeyCallback(window, key_callback);
	glfwSetMouseButtonCallback(window, mouse_callback);
	glfwSetWindowSizeCallback(window, window_size_callback);
	// Initialize GLEW
	if (!gladLoadGL()) {

		fprintf(stderr, "Unable to initialize glad");
		glfwDestroyWindow(window);
		glfwTerminate();
		return 1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	loadShapes("godzilla.obj", godzilla);
	loadShapes("cube.obj", cube);
	loadShapes("gun.obj", gun);
	loadShapes("bullet.obj", bullet);
	loadShapes("star.obj", star);
	loadShapes("flag.obj", flag);
	loadShapes("mine.obj", mine);
	initGL();
	installShaders("vert.glsl", "frag.glsl");

	glClearColor(0.6f, 0.6f, 0.8f, 1.0f);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	addTowers(numTowers);

	do{
		vec3 oldPosition = position;

		if (!playerDead) {
			if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
				position -= glm::vec3(rightV.x * cameraSpeed, 0 , rightV.z * cameraSpeed);
			}
			else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
				position += glm::vec3(rightV.x * cameraSpeed, 0, rightV.z * cameraSpeed);
			}
			else if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
				position += glm::vec3(direction.x * cameraSpeed, 0, direction.z * cameraSpeed);
			}
			else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
				position -= glm::vec3(direction.x * backPedalSpeed, 0, direction.z * backPedalSpeed);
			}
			if (position.x >= 39) {
				position.x = 39;
			}
			else if (position.x <= -39) {
				position.x = -39;
			}
			if (position.z >= 39) {
				position.z = 39;
			}
			else if (position.z <= -39) {
				position.z = -39;
			}
		}

		godzillaBulletCollide();
		godzillaMineCollide();
		towerBulletCollide();
		godzillaCollide();
		towerCollide(oldPosition);
		drawGL();

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
	glfwWindowShouldClose(window) == 0);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}
