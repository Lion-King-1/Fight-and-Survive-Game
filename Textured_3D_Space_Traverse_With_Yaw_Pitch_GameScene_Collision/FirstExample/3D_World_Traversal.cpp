
using namespace std;

#include "vgl.h"
#include "LoadShaders.h"
#include "glm\glm.hpp"
#include "glm\gtc\matrix_transform.hpp"
#include "glm\gtx\rotate_vector.hpp"
#include "..\SOIL\src\SOIL.h"
#include <vector>
#include <iostream>


enum GameObject_Type {
	PLAYER,
	ENEMY,
	BULLET,
	PBULLET,
	OBSTACLE
};

struct GameObject {

	glm::vec3 location;
	glm::vec3 rotation;
	glm::vec3 scale;
	glm::vec3 moving_direction;
	GLfloat velocity;
	GLfloat collider_dimension;
	int living_time;
	int life_span;
	int type;
	bool isAlive;
	bool isCollided;
	int target;
};

enum VAO_IDs { Triangles, NumVAOs };
enum Buffer_IDs { ArrayBuffer };
enum Attrib_IDs { vPosition = 0 };

const GLint NumBuffers = 2;
GLuint VAOs[NumVAOs];
GLuint Buffers[NumBuffers];
GLuint location;
GLuint cam_mat_location;
GLuint proj_mat_location;
GLuint texture[4];


const GLuint NumVertices = 28;

float height = 0.8f;

float travel_speed = 300.0f;
float mouse_sensitivity = 0.01f;

int x0 = 0;
int y_0 = 0;

glm::mat4 model_view;
glm::vec3 unit_z_vector = glm::vec3(0, 0, 1);
glm::vec3 cam_pos = glm::vec3(0.0f, 0.0f, height);
glm::vec3 forward_vector = glm::vec3(1, 1, 0);

glm::vec3 looking_dir_vector = glm::vec3(1, 1, 0);
glm::vec3 up_vector = unit_z_vector;
glm::vec3 side_vector = glm::cross(up_vector, forward_vector);

int oldTimeSinceStart = 0;
int deltaTime;
float alpha = 0;
int m_point = 0;
int blast_Count = 0;

const int Num_Obstacles = 1;
float obstacle_data[Num_Obstacles][3];


std::vector<GameObject> sceneGraph;


//Helper function to generate a random float number within a range
float randomFloat(float a, float b)
{
	float random = ((float)rand()) / (float)RAND_MAX;
	float diff = b - a;
	float r = random * diff;
	return a + r;
}

int lost = 0;
bool win = 1;


// inititializing buffers, coordinates, setting up pipeline, etc.
void init(void)
{
	//Normalizing all vectors
	up_vector = glm::normalize(up_vector);
	forward_vector = glm::normalize(forward_vector);
	looking_dir_vector = glm::normalize(looking_dir_vector);
	side_vector = glm::normalize(side_vector);

	for (int i = 0; i < Num_Obstacles; i++)
	{
		obstacle_data[i][0] = randomFloat(-50, 50);
		obstacle_data[i][1] = randomFloat(-50, 50);

		GameObject go;
		go.location = glm::vec3(randomFloat(-10, 10), randomFloat(-10, 10), 1.0f);
		go.rotation = glm::vec3(0, 0, 0);
		go.scale = glm::vec3(0.45f, 0.45f, 0.45f);

		go.collider_dimension = go.scale.x;
		go.isAlive = true;
		go.living_time = 0;
		go.isCollided = false;
		go.velocity = randomFloat(1, 7) / 10000;
		go.type = OBSTACLE;
		go.moving_direction = glm::vec3(randomFloat(-50, 50), randomFloat(-50, 50), 0);
		go.life_span = -1;
		int target = 0;
		sceneGraph.push_back(go);
	}


	ShaderInfo shaders[] = {
		{ GL_VERTEX_SHADER, "triangles.vert" },
		{ GL_FRAGMENT_SHADER, "triangles.frag" },
		{ GL_NONE, NULL }
	};

	GLuint program = LoadShaders(shaders);
	glUseProgram(program);

	GLfloat vertices[NumVertices][3] = {

		{ -100.0, -100.0, 0.0 }, //Plane to walk on and a sky
		{ 100.0, -100.0, 0.0 },
		{ 100.0, 100.0, 0.0 },
		{ -100.0, 100.0, 0.0 },

		{ -0.45, -0.45 ,-0.45 }, // bottom face
		{ 0.45, -0.45 ,-0.45 },
		{ 0.45, 0.45 ,-0.45 },
		{ -0.45, 0.45 ,-0.45 },

		{ -0.45, -0.45 ,0.45 }, //top face
		{ 0.45, -0.45 ,0.45 },
		{ 0.45, 0.45 ,0.45 },
		{ -0.45, 0.45 ,0.45 },

		{ 0.45, -0.45 ,-0.45 }, //left face
		{ 0.45, 0.45 , -0.45 },
		{ 0.45, 0.45 ,0.45 },
		{ 0.45, -0.45 ,0.45 },

		{ -0.45, -0.45, -0.45 }, //right face
		{ -0.45, 0.45 , -0.45 },
		{ -0.45, 0.45 ,0.45 },
		{ -0.45, -0.45 ,0.45 },

		{ -0.45, 0.45 , -0.45 }, //front face
		{ 0.45, 0.45 , -0.45 },
		{ 0.45, 0.45 ,0.45 },
		{ -0.45, 0.45 ,0.45 },

		{ -0.45, -0.45 , -0.45 }, //back face
		{ 0.45, -0.45 , -0.45 },
		{ 0.45, -0.45 ,0.45 },
		{ -0.45, -0.45 ,0.45 },
	};

	//These are the texture coordinates for the second texture
	GLfloat textureCoordinates[28][2] = {
		0.0f, 0.0f,
		200.0f, 0.0f,
		200.0f, 200.0f,
		0.0f, 200.0f,

		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,

		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,

		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,

		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,

		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,

		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,
	};


	//Creating our texture:
	//This texture is loaded from file. To do this, we use the SOIL (Simple OpenGL Imaging Library) library.
	//When using the SOIL_load_image() function, make sure the you are using correct patrameters, or else, your image will NOT be loaded properly, or will not be loaded at all.
	GLint width1, height1;
	unsigned char* textureData1 = SOIL_load_image("hay.png", &width1, &height1, 0, SOIL_LOAD_RGB);

	GLint width2, height2;
	unsigned char* textureData2 = SOIL_load_image("skull.jpg", &width2, &height2, 0, SOIL_LOAD_RGB);

	GLint width3, height3;
	unsigned char* textureData3 = SOIL_load_image("tank.jpg", &width3, &height3, 0, SOIL_LOAD_RGB);

	GLint width4, height4;
	unsigned char* textureData4 = SOIL_load_image("apple.png", &width4, &height4, 0, SOIL_LOAD_RGB);

	glGenBuffers(2, Buffers);
	glBindBuffer(GL_ARRAY_BUFFER, Buffers[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBindAttribLocation(program, 0, "vPosition");
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, Buffers[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(textureCoordinates), textureCoordinates, GL_STATIC_DRAW);
	glBindAttribLocation(program, 1, "vTexCoord");
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(1);

	location = glGetUniformLocation(program, "model_matrix");
	cam_mat_location = glGetUniformLocation(program, "camera_matrix");
	proj_mat_location = glGetUniformLocation(program, "projection_matrix");

	///////////////////////TEXTURE SET UP////////////////////////

	//Allocating two buffers in VRAM
	glGenTextures(4, texture);

	//First Texture: 

	//Set the type of the allocated buffer as "TEXTURE_2D"
	glBindTexture(GL_TEXTURE_2D, texture[0]);

	//Loading the second texture into the second allocated buffer:
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width1, height1, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData1);

	//Setting up parameters for the texture that recently pushed into VRAM
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


	//And now, second texture: 

	//Set the type of the allocated buffer as "TEXTURE_2D"
	glBindTexture(GL_TEXTURE_2D, texture[1]);

	//Loading the second texture into the second allocated buffer:
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width2, height2, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData2);

	//Setting up parameters for the texture that recently pushed into VRAM
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//////////////////////////////////////////////////////////////


	glBindTexture(GL_TEXTURE_2D, texture[2]);

	//Loading the second texture into the second allocated buffer:
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width3, height3, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData3);

	//Setting up parameters for the texture that recently pushed into VRAM
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//////////////////////////////////////////////////////////////

	glBindTexture(GL_TEXTURE_2D, texture[3]);

	//Loading the second texture into the second allocated buffer:
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width4, height4, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData4);

	//Setting up parameters for the texture that recently pushed into VRAM
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//////////////////////////////////////////////////////////////

}

//Helper function to draw a cube
void drawCube(glm::vec3 scale)
{
	model_view = glm::scale(model_view, scale);
	glUniformMatrix4fv(location, 1, GL_FALSE, &model_view[0][0]);

	//Select the second texture (apple.png) when drawing the second geometry (cube)
	glDrawArrays(GL_QUADS, 4, 24);
}

bool isColliding(GameObject one, GameObject two) {
	bool result = false;
	bool cond = glm::abs(one.location.x - two.location.x) <= (one.collider_dimension / 2 + two.collider_dimension / 2) &&
		glm::abs(one.location.y - two.location.y) <= (one.collider_dimension / 2 + two.collider_dimension / 2) &&
		glm::abs(one.location.z - two.location.z) <= (one.collider_dimension / 2 + two.collider_dimension / 2);

	if (cond) {
		result = true;
	}
	return result;
}


void checkCollisions() {

	for (int i = 0; i < sceneGraph.size(); i++) {
		for (int j = 0; j < sceneGraph.size(); j++) {
			if (i != j && sceneGraph[i].isAlive &&
				sceneGraph[j].isAlive &&
				!(sceneGraph[i].type == OBSTACLE && sceneGraph[j].type == OBSTACLE) && !sceneGraph[i].type == OBSTACLE && !sceneGraph[j].type == OBSTACLE && 
				isColliding(sceneGraph[i], sceneGraph[j])) {

				sceneGraph[i].isCollided = true;
				sceneGraph[j].isCollided = true;

				sceneGraph[i].isAlive = false;
				sceneGraph[j].isAlive = false;

			}

			if (i != j && 
				sceneGraph[i].isAlive &&
				sceneGraph[j].isAlive &&
				!(sceneGraph[i].type == PBULLET && sceneGraph[j].type == OBSTACLE) && sceneGraph[j].type == PBULLET && sceneGraph[j].type == PBULLET && isColliding(sceneGraph[i], sceneGraph[j])) {

				sceneGraph[i].isCollided = true;
				sceneGraph[j].isCollided = true;

				sceneGraph[i].isAlive = false;
				sceneGraph[j].isAlive = false;

				blast_Count += 1;
				cout << "Bullet " << blast_Count << " destroy 1 Tank" << endl;
			}
		}
	}
}
int N_Time = 0;


void updateSceneGraph() {

	checkCollisions();	

	for (int i = 0; i < sceneGraph.size(); i++) {

		GameObject go = sceneGraph[i];
	
		if (go.life_span > 0 && go.isAlive && go.living_time >= go.life_span)	
		{
			go.isAlive = false;
		}

		if (go.life_span > 0 && go.isAlive && go.living_time < go.life_span) {	
			go.location += ((GLfloat)deltaTime) * go.velocity * glm::normalize(go.moving_direction);

			go.living_time += deltaTime;

		}
		sceneGraph[i] = go;	
	}

	//Final Exam code Added
	N_Time++;
	if (N_Time >= 1000)
	{
		GameObject go;
		go.location = glm::vec3(randomFloat(-10, 10), randomFloat(-10, 10), 1.0f);	
		go.rotation = glm::vec3(0, 0, 0);
		go.scale = glm::vec3(0.45f, 0.45f, 0.45f);

		go.collider_dimension = go.scale.x;	
		go.isAlive = true;
		go.living_time = 0;
		go.isCollided = false;
		go.velocity = randomFloat(1, 7) / 10000;
		go.type = OBSTACLE;
		go.moving_direction = glm::vec3(randomFloat(-50, 50), randomFloat(-50, 50), 0);
		go.life_span = -1;
		int target = 0;
		sceneGraph.push_back(go);

		N_Time = 0;
	}
}


//Renders level
void draw_level()
{
	
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	glDrawArrays(GL_QUADS, 0, 4);

	updateSceneGraph();

	for (int i = 0; i < sceneGraph.size(); i++) {

		GameObject go = sceneGraph[i];
		model_view = glm::mat4(1);

		if (go.isAlive && !go.isCollided) {

			model_view = glm::translate(model_view, go.location);
			glUniformMatrix4fv(location, 1, GL_FALSE, &model_view[0][0]);
			glBindTexture(GL_TEXTURE_2D, texture[1]);
			drawCube(go.scale);

			glm::mat4 temp = model_view;
			if (go.type == OBSTACLE) {
				glm::vec3 tyres = glm::vec3(1.5f, 1.5f, 1.5f);

				glBindTexture(GL_TEXTURE_2D, texture[2]);
				model_view = glm::translate(model_view, glm::vec3(0.45f, -0.45f, -0.45f));
				model_view = glm::rotate(model_view, alpha, glm::vec3(0.0f, 1.0f, 0.0f));
				model_view = glm::scale(model_view, glm::vec3(0.25, 0.25, 0.25));

				glUniformMatrix4fv(location, 1, GL_FALSE, &model_view[0][0]);
				drawCube(tyres);

				model_view = temp;

				model_view = glm::translate(model_view, glm::vec3(-0.45f, -0.35f, -0.45f));
				model_view = glm::rotate(model_view, alpha, glm::vec3(0.0f, 1.0f, 0.0f));
				model_view = glm::scale(model_view, glm::vec3(0.25, 0.25, 0.25));
				glUniformMatrix4fv(location, 1, GL_FALSE, &model_view[0][0]);
				drawCube(tyres);

				model_view = temp;

				model_view = glm::translate(model_view, glm::vec3(0.45f, 0.35f, -0.45f));
				model_view = glm::rotate(model_view, alpha, glm::vec3(0.0f, 1.0f, 0.0f));
				model_view = glm::scale(model_view, glm::vec3(0.25, 0.25, 0.25));
				glUniformMatrix4fv(location, 1, GL_FALSE, &model_view[0][0]);
				drawCube(tyres);

				model_view = temp;

				model_view = glm::translate(model_view, glm::vec3(-0.45f, 0.35f, -0.45f));
				model_view = glm::rotate(model_view, alpha, glm::vec3(0.0f, 1.0f, 0.0f));
				model_view = glm::scale(model_view, glm::vec3(0.25, 0.25, 0.25));
				glUniformMatrix4fv(location, 1, GL_FALSE, &model_view[0][0]);
				drawCube(tyres);

				model_view = temp;
			}
			else if (go.type == BULLET) {
				drawCube(go.scale);
			}

		}
		else {
			auto it = sceneGraph.begin();
			while (it != sceneGraph.end()) {
				if (!(*it).isAlive) {
					it = sceneGraph.erase(it);
				}
				else {
					it++;
				}
			}
		}

	}
}

void display(void)
{
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	model_view = glm::mat4(1.0);
	glUniformMatrix4fv(location, 1, GL_FALSE, &model_view[0][0]);

	glm::vec3 look_at = cam_pos + looking_dir_vector;

	glm::mat4 camera_matrix = glm::lookAt(cam_pos, look_at, up_vector);
	glUniformMatrix4fv(cam_mat_location, 1, GL_FALSE, &camera_matrix[0][0]);

	glm::mat4 proj_matrix = glm::frustum(-0.01f, +0.01f, -0.01f, +0.01f, 0.01f, 100.0f);
	glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, &proj_matrix[0][0]);

	draw_level();

	glFlush();
}

int B_Time = 0;
void keyboard(unsigned char key, int x, int y)
{
	if (!lost && win)
	{

		if (key == 'a')
		{
			//Moving camera along opposit direction of side vector
			cam_pos += side_vector * travel_speed * ((float)deltaTime) / 1000.0f;
		}
		if (key == 'd')
		{
			//Moving camera along side vector
			cam_pos -= side_vector * travel_speed * ((float)deltaTime) / 1000.0f;
		}
		if (key == 'w')
		{
			//Moving camera along forward vector. To be more realistic, we use X=V.T equation in physics
			cam_pos += forward_vector * travel_speed * ((float)deltaTime) / 1000.0f;
		}
		if (key == 's')
		{
			//Moving camera along backward (negative forward) vector. To be more realistic, we use X=V.T equation in physics
			cam_pos -= forward_vector * travel_speed * ((float)deltaTime) / 1000.0f;
		}
		if (key == 'f')
		{
			GameObject go;
			go.location = cam_pos;	
			go.rotation = glm::vec3(0, 0, 0);
			go.scale = glm::vec3(0.07, 0.07, 0.07);
			go.collider_dimension = go.scale.x;
			go.isAlive = true;
			go.living_time = 0;
			go.isCollided = false;
			go.velocity = 0.01;
			go.type = PBULLET;
			go.moving_direction = looking_dir_vector;
			go.life_span = 4000;	
			int target = 0;
			sceneGraph.push_back(go);
		}
	}
}

void mouse(int x, int y)
{
	int delta_x = x - x0;

	forward_vector = glm::rotate(forward_vector, -delta_x * mouse_sensitivity, unit_z_vector);
	looking_dir_vector = glm::rotate(looking_dir_vector, -delta_x * mouse_sensitivity, unit_z_vector);
	side_vector = glm::rotate(side_vector, -delta_x * mouse_sensitivity, unit_z_vector);
	up_vector = glm::rotate(up_vector, -delta_x * mouse_sensitivity, unit_z_vector);
	x0 = x;

	int delta_y = y - y_0;
	glm::vec3 tmp_up_vec = glm::rotate(up_vector, delta_y * mouse_sensitivity, side_vector);
	glm::vec3 tmp_looking_dir = glm::rotate(looking_dir_vector, delta_y * mouse_sensitivity, side_vector);

	GLfloat dot_product = glm::dot(tmp_looking_dir, forward_vector);

	if (dot_product > 0)
	{
		up_vector = glm::rotate(up_vector, delta_y * mouse_sensitivity, side_vector);
		looking_dir_vector = glm::rotate(looking_dir_vector, delta_y * mouse_sensitivity, side_vector);
	}
	y_0 = y;
}

glm::vec3 dir;
glm::vec3 position;
int PTotalTime = 0;

void idle()
{
	int Number_of_Tank = 0, TP = 0;
	PTotalTime++;

	if (PTotalTime >= 100000 && !lost && win)
	{
		lost = 1;
		cout << endl << endl;
		cout << "\t\t\t\t\tL O S S ,Time Over" << endl;
	}

	if (blast_Count >= 5 && win)
	{
		win = 0;
		cout << endl << endl;
		cout << "\t\t\t\t\tV I C T O R Y ,You elliminate 5 Tanks" << endl;
	}
	int timeSinceStart = glutGet(GLUT_ELAPSED_TIME);
	deltaTime = timeSinceStart - oldTimeSinceStart;
	oldTimeSinceStart = timeSinceStart;

	float omega = 0.01;	

	alpha = omega * timeSinceStart;	

	auto it = sceneGraph.begin();
	while (it != sceneGraph.end()) {
		if (!(*it).isAlive) {
			it = sceneGraph.erase(it);
		}
		else {
			it++;
		}
	}
	B_Time++;

	for (auto it = sceneGraph.begin(); it != sceneGraph.end(); ++it)
	{
		if ((*it).type == OBSTACLE && (*it).isAlive && !lost && win)
			Number_of_Tank++;
	}

	int r = randomFloat(1, Number_of_Tank);
	for (auto it = sceneGraph.begin(); it != sceneGraph.end(); ++it)
	{
		if ((*it).type == OBSTACLE && (*it).isAlive && !lost && win)
		{
			TP++;
			bool condi = glm::abs(cam_pos.x - (*it).location.x) <= 0.35 && glm::abs(cam_pos.y - (*it).location.y) <= 0.35 && glm::abs(cam_pos.z - (*it).location.z) <= 0.35;
			dir.x = abs((*it).location.x);
			dir.y = abs((*it).location.y);

			(*it).location += ((GLfloat)deltaTime) * (*it).velocity * glm::normalize((*it).moving_direction);

			if (r == TP)
			{
				position = glm::vec3((*it).location.x, (*it).location.y, 1.0);
			}

			if (condi)
			{
				cout << endl;
				cout << "\t\t\t\t\tGame Over" << endl << endl;;
				cout << "\t\t\t\t\tL O S S ,Player is D E A D " << endl;
				lost = 1;
			}
		}

		if ((*it).type == BULLET && (*it).isAlive && !lost)
		{
			bool P_dead = glm::abs(cam_pos.x - (*it).location.x) <= 0.3 && glm::abs(cam_pos.y - (*it).location.y) <= 0.3 && glm::abs(cam_pos.z - (*it).location.z) <= 0.3;
			dir.x = abs((*it).location.x);
			dir.y = abs((*it).location.y);

			if (P_dead)
			{
				cout << endl;
				cout << "\t\t\t\t\tGame Over" << endl << endl;;
				cout << "\t\t\t\t\tL O S S ,Player is D E A D " << endl;
				lost = 1;
			}
		}
	}

	if (B_Time >= 1000 && !lost)
	{
		GameObject go;
		go.location = glm::vec3(position.x, position.y, 1);	
		go.rotation = glm::vec3(0, 0, 0);
		go.scale = glm::vec3(0.07, 0.07, 0.07);
		go.collider_dimension = go.scale.x;
		go.isAlive = true;
		go.living_time = 0;
		go.isCollided = false;
		go.velocity = 0.01;
		go.type = BULLET;
		go.moving_direction = -forward_vector;
		go.life_span = 4000;	
		int target = 0;
		sceneGraph.push_back(go);
		B_Time = 0;
	}
	glutPostRedisplay();
}

//---------------------------------------------------------------------
//
// main
//
int main(int argc, char** argv)
{
	cout << endl;
	cout << "\t\t\t\t\tWelcome to Tank Game" << endl;
	cout << endl;
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA);
	glutInitWindowSize(1024, 1024);
	glutCreateWindow("Camera and Projection");

	glewInit();	//Initializes the glew and prepares the drawing pipeline.

	init();

	glutDisplayFunc(display);

	glutKeyboardFunc(keyboard);

	glutIdleFunc(idle);

	glutPassiveMotionFunc(mouse);

	glutMainLoop();

}
