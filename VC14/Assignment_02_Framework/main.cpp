#include <glew.h>
#include <freeglut.h>
#define GLM_SWIZZLE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cstdio>
#include <cstdlib>
#include <tiny_obj_loader.h>
#include "fbxloader.h"
#include <IL/il.h>
#include <iostream>
#include <math.h>
#define  PI 3.14159265358979323846
#define  deg2rad(x) ((x)*((PI)/(180.0)))
#define MENU_TIMER_START 1
#define MENU_TIMER_STOP 2
#define MENU_EXIT 3
#define MENU_ORIG 4
#define MENU_BLOOM 5
#define MENU_REDBLUE 6
#define MENU_ABSTR 7
#define MENU_SINE 8
#define MENU_FISHEYE 9
#define MENU_SHARP 10
#define MENU_LAPLA 11
#define MENU_MAG 12
#define MENU_VOL 13
#define MENU_LOCK 14
GLubyte timer_cnt = 0;
GLfloat timer_cnt1 = 0.0;
int states = 0;
bool button_down = false;
bool timer_enabled = true;
unsigned int timer_speed = 16;
int ob;
double viewportAspect;
int pri = 0;
using namespace glm;
using namespace std;
vec3 eye(0.8f, 17.5f, 0.0f);
vec3 center(-3.2f, 17.5f, 0.0f);
vec3 up(0.0f, 1.0f, 0.0f);
vec3 initeye(0.8f, 17.5f, 0.0f);
vec3 initcenter(-3.2f, 17.5f, 0.0f);
vec3 initup(0.0f, 1.0f, 0.0f);
vec3 zombie(0.0f,0.0f,0.0f);
float zombierad = 0.0f;
int fromx, fromy;
float rotx = 0, roty = 0, rotxn, rotyn;
mat4 mvp;
mat4 mvp2;
mat4 rotate = mat4(1.0f);
GLint um4mvp;
int state = 1;
fbx_handles myFbx[4];
GLint time_uni;
struct obj {
	int shapeCount, materialCount;
	vector<tinyobj::shape_t> shapes;
	vector<tinyobj::material_t> materials;
	GLuint *vao;
	GLuint *vbo;
	GLuint *bufferi;
	GLuint *texture;
}typedef obj;
obj object[4];
string err;
GLuint program;
bool cameralock = false;
bool pressed = false;

GLuint modes;
GLuint img_size;
GLuint mouse_uni;
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
// For Frame Buffer Object(FBO)
GLuint vao2;
GLuint fbo;
GLuint depthrbo;
GLuint program2;
GLuint window_vertex_buffer;
GLuint fboDataTexture;

vec2 mouse;

static const GLfloat window_positions[] =
{
	//vec2 position vec2 texture_coord
	1.0f,-1.0f,1.0f,0.0f,
	-1.0f,-1.0f,0.0f,0.0f,
	-1.0f,1.0f,0.0f,1.0f,
	1.0f,1.0f,1.0f,1.0f
};




//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

void checkError(const char *functionName)
{
	GLenum error;
	while ((error = glGetError()) != GL_NO_ERROR) {
		fprintf(stderr, "GL error 0x%X detected in %s\n", error, functionName);
	}
}

// Print OpenGL context related information.
void dumpInfo(void)
{
	printf("Vendor: %s\n", glGetString(GL_VENDOR));
	printf("Renderer: %s\n", glGetString(GL_RENDERER));
	printf("Version: %s\n", glGetString(GL_VERSION));
	printf("GLSL: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
}

char** loadShaderSource(const char* file)
{
	FILE* fp = fopen(file, "rb");
	fseek(fp, 0, SEEK_END);
	long sz = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	char *src = new char[sz + 1];
	fread(src, sizeof(char), sz, fp);
	src[sz] = '\0';
	char **srcp = new char*[1];
	srcp[0] = src;
	return srcp;
}

void freeShaderSource(char** srcp)
{
	delete srcp[0];
	delete srcp;
}

void shaderLog(GLuint shader)
{
	GLint isCompiled = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
	if (isCompiled == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

		// The maxLength includes the NULL character
		GLchar* errorLog = new GLchar[maxLength];
		glGetShaderInfoLog(shader, maxLength, &maxLength, &errorLog[0]);

		printf("%s\n", errorLog);
		delete errorLog;
	}
}

void My_Init()
{
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	program = glCreateProgram();
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	char** vertexShaderSource = loadShaderSource("vertex.vs.glsl");
	char** fragmentShaderSource = loadShaderSource("fragment.fs.glsl");
	glShaderSource(vertexShader, 1, vertexShaderSource, NULL);
	glShaderSource(fragmentShader, 1, fragmentShaderSource, NULL);
	freeShaderSource(vertexShaderSource);
	freeShaderSource(fragmentShaderSource);
	glCompileShader(vertexShader);
	glCompileShader(fragmentShader);
	shaderLog(vertexShader);
	shaderLog(fragmentShader);
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glLinkProgram(program);
	um4mvp = glGetUniformLocation(program, "um4mvp");
	//glUseProgram(program);

	program2 = glCreateProgram();
	GLuint vertexShader2 = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShader2 = glCreateShader(GL_FRAGMENT_SHADER);
	char** vertexShaderSource2 = loadShaderSource("vertex2.vs.glsl");
	char** fragmentShaderSource2 = loadShaderSource("fragment2.fs.glsl");
	glShaderSource(vertexShader2, 1, vertexShaderSource2, NULL);
	glShaderSource(fragmentShader2, 1, fragmentShaderSource2, NULL);
	freeShaderSource(vertexShaderSource2);
	freeShaderSource(fragmentShaderSource2);
	glCompileShader(vertexShader2);
	glCompileShader(fragmentShader2);
	shaderLog(vertexShader2);
	shaderLog(fragmentShader2);
	glAttachShader(program2, vertexShader2);
	glAttachShader(program2, fragmentShader2);
	glLinkProgram(program2);
	//um4mvp = glGetUniformLocation(program2, "um4mvp");
	//glUseProgram(program2);


	glGenVertexArrays(1, &vao2);
	glBindVertexArray(vao2);

	glGenBuffers(1, &window_vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, window_vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(window_positions), window_positions, GL_STATIC_DRAW);

	glVertexAttribPointer(5, 2, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 4, 0);
	glVertexAttribPointer(6, 2, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 4, (const GLvoid*)(sizeof(GL_FLOAT) * 2));

	glEnableVertexAttribArray(5);
	glEnableVertexAttribArray(6);

	glGenFramebuffers(1, &fbo);

}

void My_LoadModels()
{
	for (pri = 0; pri <7; pri++) {
		bool ret;	// TODO: If You Want to Load FBX, Use these. The Returned Values are The Same.
					// Save this Object, You Will Need It to Retrieve Animations Later.
					// bool ret2 = LoadFbx(myFbx, shapes, materials, err, "sponza.mtl");
		if (pri == 0) {
			ret = tinyobj::LoadObj(object[0].shapes, object[0].materials, err, "summoner_rift.obj");
			ob = 0;
		}
		else {
			if (pri == 1)ret = LoadFbx(myFbx[pri], object[pri].shapes, object[pri].materials, err, "zombie_walk.FBX");
			else if (pri == 2)ret = LoadFbx(myFbx[pri], object[pri].shapes, object[pri].materials, err, "zombie_fury.FBX");
			else ret = LoadFbx(myFbx[pri], object[pri].shapes, object[pri].materials, err, "zombie_dead.FBX");
			ob = pri;
		}
		if (ret)
		{
			object[ob].shapeCount = object[ob].shapes.size();
			object[ob].materialCount = object[ob].materials.size();
			//	std::cout << shapeCount << std::endl;
			object[ob].vbo = new GLuint[object[ob].shapeCount];
			object[ob].bufferi = new GLuint[object[ob].shapeCount];
			object[ob].vao = new GLuint[object[ob].shapeCount];
			object[ob].texture = new GLuint[object[ob].materialCount];
			// For Each Material
			for (int i = 0; i < object[ob].materials.size(); i++)
			{
				// materials[i].diffuse_texname; // This is the Texture Path You Need
				ILuint ilTexName;
				ilGenImages(1, &ilTexName);
				ilBindImage(ilTexName);
				//	cout << object[ob].materials[i].diffuse_texname << endl;
				if (ilLoadImage(object[ob].materials[i].diffuse_texname.c_str()) && ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE))
				{
					unsigned char *data = new unsigned char[ilGetInteger(IL_IMAGE_WIDTH) * ilGetInteger(IL_IMAGE_HEIGHT) * 4];
					int Image_Width = ilGetInteger(IL_IMAGE_WIDTH);
					int Image_Height = ilGetInteger(IL_IMAGE_HEIGHT);
					ilCopyPixels(0, 0, 0, ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT), 1, IL_RGBA, IL_UNSIGNED_BYTE, data);
					// TODO: Generate an OpenGL Texture and use the [unsigned char *data] as Input Here.
					glGenTextures(1, &object[ob].texture[i]);
					glBindTexture(GL_TEXTURE_2D, object[ob].texture[i]);
					glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Image_Width, Image_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
					glGenerateMipmap(GL_TEXTURE_2D);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
					delete[] data;
					ilDeleteImages(1, &ilTexName);
				}
			}

			// For Each Shape (or Mesh, Object)
			for (int i = 0; i < object[ob].shapes.size(); i++)
			{
				// shapes[i].mesh.positions; // VertexCount * 3 Floats, Load Them to a GL_ARRAY_BUFFER
				// shapes[i].mesh.normals; // VertexCount * 3 Floats, Load Them to a GL_ARRAY_BUFFER
				// shapes[i].mesh.texcoords; // VertexCount * 2 Floats, Load Them to a GL_ARRAY_BUFFER
				// shapes[i].mesh.indices; // TriangleCount * 3 Unsigned Integers, Load Them to a GL_ELEMENT_ARRAY_BUFFER
				// shapes[i].mesh.material_ids[0] // The Material ID of This Shape

				// TODO: 
				// 1. Generate and Bind a VAO
				glGenVertexArrays(1, &object[ob].vao[i]);
				glBindVertexArray(object[ob].vao[i]);
				// 2. Generate and Bind a Buffer for position/normal/texcoord
				int SizeOfPositions = sizeof(float)*object[ob].shapes[i].mesh.positions.size();
				int SizeOfNormals = sizeof(float)*object[ob].shapes[i].mesh.normals.size();
				int SizeOfTexCoords = sizeof(float)*object[ob].shapes[i].mesh.texcoords.size();
				glGenBuffers(1, &object[ob].vbo[i]);
				glBindBuffer(GL_ARRAY_BUFFER, object[ob].vbo[i]);
				glBufferData(GL_ARRAY_BUFFER, (SizeOfPositions + SizeOfNormals + SizeOfTexCoords), NULL, GL_STATIC_DRAW);
				// 3. Upload Data to The Buffers	
				for (int j = 0; j < object[ob].shapes[i].mesh.positions.size() / 3; j++) {
					glBufferSubData(GL_ARRAY_BUFFER, (j * 8 * sizeof(float)), 3 * sizeof(float), &(object[ob].shapes[i].mesh.positions.data()[3 * j]));
					glBufferSubData(GL_ARRAY_BUFFER, (j * 8 + 3) * sizeof(float), 3 * sizeof(float), &(object[ob].shapes[i].mesh.normals.data()[3 * j]));
					glBufferSubData(GL_ARRAY_BUFFER, (j * 8 + 6) * sizeof(float), 2 * sizeof(float), &(object[ob].shapes[i].mesh.texcoords.data()[2 * j]));
				}
				// 4. Generate and Bind a Buffer for indices (Will Be Saved In The VAO, You Can Restore Them By Binding The VAO)
				glGenBuffers(1, &object[ob].bufferi[i]);
				int SizeOfIndice = sizeof(unsigned int)*object[ob].shapes[i].mesh.indices.size();
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, object[ob].bufferi[i]);
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, SizeOfIndice, object[ob].shapes[i].mesh.indices.data(), GL_STATIC_DRAW);
				// 5. glVertexAttribPointer Calls (Will Be Saved In The VAO, You Can Restore Them By Binding The VAO)
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 0);
				glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
				glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
				// 6. glEnableVertexAttribArray Calls For Your Shader Program Here
				glEnableVertexAttribArray(0);
				glEnableVertexAttribArray(1);
				glEnableVertexAttribArray(2);

			}
		}
	}

}

// GLUT callback. Called to draw the scene.
void My_Display()
{

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);

	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for (pri = 0; pri <= 5; pri++) {
		// TODO: For Your FBX Model, Get New Animation Here

		glUseProgram(program);
		if (pressed) {
			state = 1;
		}
		else {
			state = 2;
		}

		if (pri == 1 ) {
			std::vector<tinyobj::shape_t> new_shapes;
			GetFbxAnimation(myFbx[state], new_shapes, 0.01*(timer_cnt % 100)); // The Last Parameter is A Float in [0, 1], Specifying The Animation Location You Want to Retrieve
			//const float *m= (const float*)value_ptr(mvp*(glm::scale<float>(mat4(1.0f), vec3(0.025f, 0.025f, 0.025f))*(glm::translate(mat4(1.0), vec3(0+40*zombie.x, 700+40*zombie.y, 0+40*zombie.z))*(glm::rotate<float>(mat4(1.0f), deg2rad(-90.0f), vec3(1, 0, 0))))));
			
			glUniformMatrix4fv(um4mvp, 1, GL_FALSE, value_ptr(mvp*(glm::scale<float>(mat4(1.0f), vec3(0.01f, 0.01f, 0.01f))*(glm::translate(mat4(1.0), vec3(0 + 100 * zombie.x, 1730 + 100 * zombie.y, 100 * zombie.z)))*(glm::rotate<float>(mat4(1.0f), deg2rad(-90.0f), vec3(1, 0, 0)))*(glm::rotate<float>(mat4(1.0f), deg2rad(zombierad), vec3(0, 0, 1))))));
			ob = state;
			for (int i = 0; i < new_shapes.size(); i++)
			{
				// new_shapes[i].mesh.positions; // VertexCount * 3 Floats, Updated Vertex Positions of This Shape
				// 1. Bind The Buffer You Created In My_LoadModels() For This Shape
				glBindVertexArray(object[state].vao[i]);
				glBindBuffer(GL_ARRAY_BUFFER, object[state].vbo[i]);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, object[state].bufferi[i]);
				// 2. Update Position Data by glBufferSubData() Call
				for (int j = 0; j < new_shapes[i].mesh.positions.size() / 3; j++) {
					glBufferSubData(GL_ARRAY_BUFFER, (j * 8 * sizeof(float)), 3 * sizeof(float), &(new_shapes[i].mesh.positions.data()[3 * j]));
				}
			}
			pressed = false;
		}

		else {
			glUniformMatrix4fv(um4mvp, 1, GL_FALSE, value_ptr(mvp));
			ob = 0;
		}
		// TODO: 
		// For Each Shapes You Loaded In My_LoadModels()
		for (int i = 0; i < object[ob].shapes.size(); i++) {
			glBindVertexArray(object[ob].vao[i]);
			glBindBuffer(GL_ARRAY_BUFFER, object[ob].vbo[i]);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, object[ob].bufferi[i]);
			for (int j = 0; j < object[ob].shapes[i].mesh.indices.size(); j += 3) {
				// 1. Bind The VAO of the Shape

				// 3. Bind Textures
				//	
				// 4. Update Uniform Values by glUniform*

				int MaterialID = object[ob].shapes[i].mesh.material_ids[j / 3];
				glBindTexture(GL_TEXTURE_2D, object[ob].texture[MaterialID]);

				//glUniform1i(u1iflag, color);
				// 5. glDrawElements Call
				glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (void*)(sizeof(unsigned int)*j));

			}
		}

	}
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, fboDataTexture);

	glBindVertexArray(vao2);
	modes = glGetUniformLocation(program2, "modes");
	img_size = glGetUniformLocation(program2, "img_size");
	mouse_uni = glGetUniformLocation(program2, "mouse"); 
	time_uni = glGetUniformLocation(program2, "time");
	glUseProgram(program2);
	//glBindTexture( GL_TEXTURE_2D, m_shpae.m_texture );
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glutSwapBuffers();

	glGetError();
}

void My_Reshape(int width, int height)
{
	glViewport(0, 0, width, height);
	viewportAspect = (double)width / (double)height;
	mvp = perspective(deg2rad(60.0f), viewportAspect, 0.3, 500.0);
	mvp2 = perspective(deg2rad(60.0f), viewportAspect, 0.3, 500.0);
	
	
		mvp = mvp * lookAt(eye, center, up)*glm::rotate<float>(mat4(1.0f), deg2rad(-roty), vec3(0, 0, 1))*glm::rotate<float>(mat4(1.0f), deg2rad(-rotx), vec3(0, 1, 0));
	
	//glm::translate(mat4(1.0), vec3(0 + 40 * zombie.x, 700 + 40 * zombie.y, 0)
	glDeleteRenderbuffers(1, &depthrbo);
	glDeleteTextures(1, &fboDataTexture);
	glGenRenderbuffers(1, &depthrbo);
	glBindRenderbuffer(GL_RENDERBUFFER, depthrbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, width, height);

	glGenTextures(1, &fboDataTexture);
	glBindTexture(GL_TEXTURE_2D, fboDataTexture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
	glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthrbo);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboDataTexture, 0);
	cout << width << " " << height<<endl;
	glUniform2f(img_size, (float)width, (float)height);
	
}

void My_Timer(int val)
{
	timer_cnt+=4;
	timer_cnt1 = timer_cnt1 + 1.0;
	glutPostRedisplay();
	if (timer_enabled)
	{
		glutTimerFunc(timer_speed, My_Timer, val);
		glUniform1f(time_uni, -0.2 + (timer_cnt % 20)*0.02);
	}
}
void MouseMotion(int x, int y) {
	mouse = vec2(x, y);
	if (button_down) {
		int dx, dy;
		dx = x - fromx;
		dy = y - fromy;
		rotx = rotxn + dx / 10;
		roty = rotyn + dy / 10;
		mvp = perspective(deg2rad(60.0f), viewportAspect, 0.3, 500.0);
		mvp = mvp * lookAt(eye, center, up)*glm::rotate<float>(mat4(1.0f), deg2rad(-roty), vec3(0, 0, 1))*glm::rotate<float>(mat4(1.0f), deg2rad(-rotx), vec3(0, 1, 0));
		
		cout << eye.x << "," << eye.y << "," << eye.z << endl;
		cout << center.x << "," << center.y << "," << center.z << endl;

	}
	else glUniform2f(mouse_uni, (float)x, (float)glutGet(GLUT_WINDOW_HEIGHT) - (float)y);
}
void My_Mouse(int button, int state, int x, int y)
{
	if (button == 0 && state == GLUT_DOWN)
	{
		fromx = x;
		fromy = y;
		rotxn = rotx;
		rotyn = roty;
		button_down = true;
	}
	else if (button == 0 && state == GLUT_UP)
	{
		cout << x << "," << y << endl;
		button_down = false;
		
	}
	else if (button == 3)
	{
		eye.x += 1.0f;
		center.x += 1.0f;
		mvp = perspective(deg2rad(60.0f), viewportAspect, 0.3, 500.0);
		mvp = mvp * lookAt(eye, center, up)*glm::rotate<float>(mat4(1.0f), deg2rad(-roty), vec3(0, 0, 1))*glm::rotate<float>(mat4(1.0f), deg2rad(-rotx), vec3(0, 1, 0));
	}
	else if (button == 4) {
		eye.x -= 1.0f;
		center.x -= 1.0f;
		mvp = perspective(deg2rad(60.0f), viewportAspect, 0.3, 500.0);
		mvp = mvp * lookAt(eye, center, up)*glm::rotate<float>(mat4(1.0f), deg2rad(-roty), vec3(0, 0, 1))*glm::rotate<float>(mat4(1.0f), deg2rad(-rotx), vec3(0, 1, 0));
	}
}

void My_Keyboard(unsigned char key, int x, int y)
{
	
	switch (key) {
	case 'x':
		eye.y += 0.1f;
		center.y += 0.1f;
		mvp = perspective(deg2rad(60.0f), viewportAspect, 0.3, 500.0);
		if (cameralock)
		{
			mvp = mvp * lookAt(initeye, initcenter, initup)*glm::translate<float>(mat4(1.0f), -vec3(zombie.x, zombie.y, zombie.z))*glm::translate<float>(mat4(1.0f), vec3(zombie.x, zombie.y, zombie.z))*glm::rotate<float>(mat4(1.0f), deg2rad(-zombierad), vec3(0, 1, 0))*glm::translate<float>(mat4(1.0f), -vec3(zombie.x, zombie.y, zombie.z));
		}
		else {
			mvp = mvp * lookAt(eye, center, up)*glm::rotate<float>(mat4(1.0f), deg2rad(-roty), vec3(0, 0, 1))*glm::rotate<float>(mat4(1.0f), deg2rad(-rotx), vec3(0, 1, 0));

		}break;
	case 'z':
		eye.y -= 0.1f;
		center.y -= 0.1f;
		mvp = perspective(deg2rad(60.0f), viewportAspect, 0.3, 500.0);
		if (cameralock)
		{
			mvp = mvp * lookAt(initeye, initcenter, initup)*glm::translate<float>(mat4(1.0f), -vec3(zombie.x, zombie.y, zombie.z))*glm::translate<float>(mat4(1.0f), vec3(zombie.x, zombie.y, zombie.z))*glm::rotate<float>(mat4(1.0f), deg2rad(-zombierad), vec3(0, 1, 0))*glm::translate<float>(mat4(1.0f), -vec3(zombie.x, zombie.y, zombie.z));
		}
		else {
			mvp = mvp * lookAt(eye, center, up)*glm::rotate<float>(mat4(1.0f), deg2rad(-roty), vec3(0, 0, 1))*glm::rotate<float>(mat4(1.0f), deg2rad(-rotx), vec3(0, 1, 0));
		}
		break;
	case 's':
		eye.x += 0.1f;
		center.x += 0.1f;
		mvp = perspective(deg2rad(60.0f), viewportAspect, 0.3, 500.0);
		if (cameralock)
		{
			mvp = mvp * lookAt(initeye, initcenter, initup)*glm::translate<float>(mat4(1.0f), -vec3(zombie.x, zombie.y, zombie.z))*glm::translate<float>(mat4(1.0f), vec3(zombie.x, zombie.y, zombie.z))*glm::rotate<float>(mat4(1.0f), deg2rad(-zombierad), vec3(0, 1, 0))*glm::translate<float>(mat4(1.0f), -vec3(zombie.x, zombie.y, zombie.z));
		}
		else {
			mvp = mvp * lookAt(eye, center, up)*glm::rotate<float>(mat4(1.0f), deg2rad(-roty), vec3(0, 0, 1))*glm::rotate<float>(mat4(1.0f), deg2rad(-rotx), vec3(0, 1, 0));
		}
		break;
	case 'w':
		eye.x -= 0.1f;
		center.x -= 0.1f;
		mvp = perspective(deg2rad(60.0f), viewportAspect, 0.3, 500.0);
		if (cameralock)
		{
			mvp = mvp * lookAt(initeye, initcenter, initup)*glm::translate<float>(mat4(1.0f), -vec3(zombie.x, zombie.y, zombie.z))*glm::translate<float>(mat4(1.0f), vec3(zombie.x, zombie.y, zombie.z))*glm::rotate<float>(mat4(1.0f), deg2rad(-zombierad), vec3(0, 1, 0))*glm::translate<float>(mat4(1.0f), -vec3(zombie.x, zombie.y, zombie.z));
		}
		else {
			mvp = mvp * lookAt(eye, center, up)*glm::rotate<float>(mat4(1.0f), deg2rad(-roty), vec3(0, 0, 1))*glm::rotate<float>(mat4(1.0f), deg2rad(-rotx), vec3(0, 1, 0));
		}
		break;
	case 'd':
		eye.z -= 0.1f;
		center.z -= 0.1f;
		mvp = perspective(deg2rad(60.0f), viewportAspect, 0.3, 500.0);
		if (cameralock)
		{
			mvp = mvp * lookAt(initeye, initcenter, initup)*glm::translate<float>(mat4(1.0f), -vec3(zombie.x, zombie.y, zombie.z))*glm::translate<float>(mat4(1.0f), vec3(zombie.x, zombie.y, zombie.z))*glm::rotate<float>(mat4(1.0f), deg2rad(-zombierad), vec3(0, 1, 0))*glm::translate<float>(mat4(1.0f), -vec3(zombie.x, zombie.y, zombie.z));
		}
		else {
			mvp = mvp * lookAt(eye, center, up)*glm::rotate<float>(mat4(1.0f), deg2rad(-roty), vec3(0, 0, 1))*glm::rotate<float>(mat4(1.0f), deg2rad(-rotx), vec3(0, 1, 0));
		}
		break;
	case 'a':
		eye.z += 0.1f;
		center.z += 0.1f;
		mvp = perspective(deg2rad(60.0f), viewportAspect, 0.3, 500.0);
		if (cameralock)
		{
			mvp = mvp * lookAt(initeye, initcenter, initup)*glm::translate<float>(mat4(1.0f), -vec3(zombie.x, zombie.y, zombie.z))*glm::translate<float>(mat4(1.0f), vec3(zombie.x, zombie.y, zombie.z))*glm::rotate<float>(mat4(1.0f), deg2rad(-zombierad), vec3(0, 1, 0))*glm::translate<float>(mat4(1.0f), -vec3(zombie.x, zombie.y, zombie.z));
		}
		else {
			mvp = mvp * lookAt(eye, center, up)*glm::rotate<float>(mat4(1.0f), deg2rad(-roty), vec3(0, 0, 1))*glm::rotate<float>(mat4(1.0f), deg2rad(-rotx), vec3(0, 1, 0));
		}
		break;

	case 'i':
		pressed = true;
		eye.x -= 0.1f*cos(deg2rad(-zombierad));
		eye.z -= 0.1f*sin(deg2rad(-zombierad));
		zombie.x -= 0.1f*cos(deg2rad(-zombierad));
		zombie.z -= 0.1f*sin(deg2rad(-zombierad));
		center.x -= 0.1f*cos(deg2rad(-zombierad));
		center.z -= 0.1f*sin(deg2rad(-zombierad));
		mvp = perspective(deg2rad(60.0f), viewportAspect, 0.3, 500.0);
		if (cameralock)
		{
			mvp = mvp * lookAt(initeye, initcenter, initup)*glm::translate<float>(mat4(1.0f), -vec3(zombie.x, zombie.y, zombie.z))*glm::translate<float>(mat4(1.0f), vec3(zombie.x, zombie.y, zombie.z))*glm::rotate<float>(mat4(1.0f), deg2rad(-zombierad), vec3(0, 1, 0))*glm::translate<float>(mat4(1.0f), -vec3(zombie.x, zombie.y, zombie.z));
		}
		else {
			mvp = mvp * lookAt(eye, center, up)*glm::rotate<float>(mat4(1.0f), deg2rad(-roty), vec3(0, 0, 1))*glm::rotate<float>(mat4(1.0f), deg2rad(-rotx), vec3(0, 1, 0));
		}
		break;
	case 'k':
		pressed = true;
		eye.x += 0.1f*cos(deg2rad(-zombierad));
		eye.z += 0.1f*sin(deg2rad(-zombierad));
		zombie.x += 0.1f*cos(deg2rad(-zombierad));
		zombie.z += 0.1f*sin(deg2rad(-zombierad));
		center.x += 0.1f*cos(deg2rad(-zombierad));
		center.z += 0.1f*sin(deg2rad(-zombierad));
		mvp = perspective(deg2rad(60.0f), viewportAspect, 0.3, 500.0);
		if (cameralock)
		{
			mvp = mvp * lookAt(initeye, initcenter, initup)*glm::translate<float>(mat4(1.0f), -vec3(zombie.x, zombie.y, zombie.z))*glm::translate<float>(mat4(1.0f), vec3(zombie.x, zombie.y, zombie.z))*glm::rotate<float>(mat4(1.0f), deg2rad(-zombierad), vec3(0, 1, 0))*glm::translate<float>(mat4(1.0f), -vec3(zombie.x, zombie.y, zombie.z));
		}
		else {
			mvp = mvp * lookAt(eye, center, up)*glm::rotate<float>(mat4(1.0f), deg2rad(-roty), vec3(0, 0, 1))*glm::rotate<float>(mat4(1.0f), deg2rad(-rotx), vec3(0, 1, 0));
		}
		break;
	case 'l':
		pressed = true;
		zombierad -= 3;
		//roty += 1;
		mvp = perspective(deg2rad(60.0f), viewportAspect, 0.3, 500.0);
		if (cameralock)
		{
			mvp = mvp * lookAt(initeye, initcenter, initup)*glm::translate<float>(mat4(1.0f), -vec3(zombie.x, zombie.y, zombie.z))*glm::translate<float>(mat4(1.0f), vec3(zombie.x, zombie.y, zombie.z))*glm::rotate<float>(mat4(1.0f), deg2rad(-zombierad), vec3(0, 1, 0))*glm::translate<float>(mat4(1.0f), -vec3(zombie.x, zombie.y, zombie.z));
		}
		else {
			mvp = mvp * lookAt(eye, center, up)*glm::rotate<float>(mat4(1.0f), deg2rad(-roty), vec3(0, 1, 0));
			//*glm::rotate<float>(mat4(1.0f), deg2rad(-roty), vec3(0, 0, 1))*glm::rotate<float>(mat4(1.0f), deg2rad(-rotx), vec3(0, 1, 0));
		}
		break;
	case 'j':
		pressed = true;
		zombierad += 3;
		//roty -= 1;
		mvp = perspective(deg2rad(60.0f), viewportAspect, 0.3, 500.0);
		if (cameralock)
		{
			mvp = mvp * lookAt(initeye, initcenter, initup)*glm::translate<float>(mat4(1.0f), -vec3(zombie.x, zombie.y, zombie.z))*glm::translate<float>(mat4(1.0f), vec3(zombie.x, zombie.y, zombie.z))*glm::rotate<float>(mat4(1.0f), deg2rad(-zombierad), vec3(0, 1 ,0))*glm::translate<float>(mat4(1.0f), -vec3(zombie.x, zombie.y, zombie.z));
		}
		else {
			mvp = mvp * lookAt(eye, center, up)*glm::rotate<float>(mat4(1.0f), deg2rad(-roty), vec3(0, 1, 0));
		}
		break;
	default:
		printf("Other special key is pressed at (%d, %d)\n", x, y);
		break;
	}
	//	cout << eye.x<<","<<eye.y<<","<<eye.z << endl;
}

void My_SpecialKeys(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_PAGE_UP:
		state = (state) % 3 + 1;
		break;
	case GLUT_KEY_PAGE_DOWN:
		state = (state + 1) % 3 + 1;
		break;
	default:
		printf("Other special key is pressed at (%d, %d)\n", x, y);
		break;
	}
}

void My_Menu(int id)
{
	switch (id)
	{

	case MENU_TIMER_START:
		if (!timer_enabled)
		{
			timer_enabled = true;
			glutTimerFunc(timer_speed, My_Timer, 0);
		}
		break;
	case MENU_ORIG:
		glUniform1i(modes, 0);
		break;
	case MENU_REDBLUE:
		glUniform1i(modes, 1);
		break;
	case MENU_BLOOM:
		glUniform1i(modes, 2);
		break;	
	case MENU_ABSTR:
		glUniform1i(modes, 3);
		glUniform2f(img_size, (float)glutGet( GLUT_WINDOW_WIDTH), (float)glutGet(GLUT_WINDOW_HEIGHT));
		break;
	case MENU_SINE:
		glUniform1i(modes, 4);
		//glUniform2f(img_size, glutGet( GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
		break;
	case MENU_FISHEYE:
		glUniform1i(modes, 5);
		//glUniform2f(img_size, glutGet( GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
		break;	
	case MENU_SHARP:
		glUniform1i(modes, 6);
		glUniform2f(img_size, glutGet( GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
		break;
	case MENU_LAPLA:
		glUniform1i(modes, 7);
		glUniform2f(img_size, glutGet( GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
		break;
	case MENU_MAG:
		glUniform1i(modes, 8);
		glUniform2f(img_size, glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
		glUniform2f(mouse_uni, glutGet(GLUT_WINDOW_WIDTH)/2, glutGet(GLUT_WINDOW_HEIGHT)/2);
		break;
	case MENU_VOL:
		glUniform1i(modes, 9);
		glUniform2f(img_size, glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
		break;
	case MENU_LOCK:
		if (cameralock) {
			cameralock = false;
		}
		else {
			printf("cameralock\n");
			cameralock = true;
			mvp = perspective(deg2rad(60.0f), viewportAspect, 0.3, 500.0);
			if (cameralock)
			{
				mvp = mvp * lookAt(initeye, initcenter, initup)*glm::translate<float>(mat4(1.0f), -vec3(zombie.x, zombie.y, zombie.z))*glm::translate<float>(mat4(1.0f), vec3(zombie.x, zombie.y, zombie.z))*glm::rotate<float>(mat4(1.0f), deg2rad(-zombierad), vec3(0, 1, 0))*glm::translate<float>(mat4(1.0f), -vec3(zombie.x, zombie.y, zombie.z));
			}
		}

		break			;
	case MENU_TIMER_STOP:
		timer_enabled = false;
		break;
	case MENU_EXIT:
		exit(0);
		break;
	default:
		break;
	}
}

int main(int argc, char *argv[])
{
	// Initialize GLUT and GLEW, then create a window.
	////////////////////
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(600, 600);
	glutCreateWindow("Assignment 02 102021221"); // You cannot use OpenGL functions before this line; The OpenGL context must be created first by glutCreateWindow()!
	glewInit();
	ilInit();
	ilEnable(IL_ORIGIN_SET);
	ilOriginFunc(IL_ORIGIN_LOWER_LEFT);
	dumpInfo();
	My_Init();
	My_LoadModels();
	////////////////////

	// Create a menu and bind it to mouse right button.
	////////////////////////////
	int menu_main = glutCreateMenu(My_Menu);
	int menu_timer = glutCreateMenu(My_Menu);
	int menu_func = glutCreateMenu(My_Menu);

	glutSetMenu(menu_main);
	glutAddSubMenu("Timer", menu_timer);
	glutAddSubMenu("Function", menu_func);
	glutAddMenuEntry("Exit", MENU_EXIT);

	glutSetMenu(menu_timer);
	glutAddMenuEntry("Start", MENU_TIMER_START);
	glutAddMenuEntry("Stop", MENU_TIMER_STOP);
	glutSetMenu(menu_func);
	glutAddMenuEntry("ORIGINAL", MENU_ORIG);
	glutAddMenuEntry("REDBLUE", MENU_REDBLUE);
	glutAddMenuEntry("BLOOM", MENU_BLOOM);
	glutAddMenuEntry("ABSTRACTION", MENU_ABSTR);
	glutAddMenuEntry("SINE WAVE", MENU_SINE);
	glutAddMenuEntry("FISH EYE", MENU_FISHEYE);
	glutAddMenuEntry("SHARPNESS", MENU_SHARP);
	glutAddMenuEntry("LAPLACIAN", MENU_LAPLA); 
	glutAddMenuEntry("MAGNIFIER", MENU_MAG);
	glutAddMenuEntry("RADICAL BLUR", MENU_VOL);
	glutAddMenuEntry("CAMERALOCK", MENU_LOCK);
	glutSetMenu(menu_main);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
	////////////////////////////

	// Register GLUT callback functions.
	///////////////////////////////
	glutDisplayFunc(My_Display);
	glutReshapeFunc(My_Reshape);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glutMouseFunc(My_Mouse);
	glutMotionFunc(MouseMotion);
	glutKeyboardFunc(My_Keyboard);
	glutSpecialFunc(My_SpecialKeys);
	glutTimerFunc(timer_speed, My_Timer, 0);
	///////////////////////////////

	// Enter main event loop.
	//////////////
	glutMainLoop();
	//////////////
	return 0;
}