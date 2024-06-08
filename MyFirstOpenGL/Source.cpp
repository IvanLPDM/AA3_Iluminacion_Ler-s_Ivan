#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <gtc/type_ptr.hpp>
#include <gtc/matrix_transform.hpp>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <sstream>
#include <stb_image.h>
#include "Model.h"
#include <chrono>

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

std::vector<GLuint> compiledPrograms;
std::vector<Model> models;

enum class CameraStates
{
	STATE1,
	STATE2,
	STATE3,
	ORBIT
};

CameraStates stateCamera = CameraStates::ORBIT;

struct Light
{
	glm::vec3 position;
};

struct Camera {
	float fFov = 45.f;
	float fNear = 0.1;
	float fFar = 10.f;

	float orbitAngle = 0.0f; // Ángulo inicial
	float orbitRadius = 2.0f; // Radio de la órbita
	float orbitVelocity = 1;
	float orbitVelocity_2 = 1.0f; // Añade esto a la estructura Camera

	glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
	glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

	float deltaTime = 0.0f; // Time between current frame and last frame
	float lastFrame = 0.0f; // Time of last frame

	bool firstMouse = true;
	float yaw = -90.0f; // Yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right
	float pitch = 0.0f;
	float lastX = WINDOW_WIDTH / 2.0;
	float lastY = WINDOW_HEIGHT / 2.0;
	float fov = 45.0f;
	float mouseSensitivity = 0.1f;
	float cameraSpeed = 0.001f; // Adjust accordingly
	//float maxDistance = 100.0f;

	//Player
	bool flashlightOn;
	float innerConeAngle;
	float outerConeAngle;
	int a = 0;
};
Camera camera;

//Inputs
void processInput(GLFWwindow* window) {
	float currentFrame = glfwGetTime();
	camera.deltaTime = currentFrame - camera.lastFrame;
	camera.lastFrame = currentFrame;

	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
		camera.orbitVelocity += 0.1f; // Aumenta la velocidad
	}
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
		camera.orbitVelocity -= 0.1f; // Disminuye la velocidad
	}

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		camera.cameraPos += camera.cameraSpeed * camera.cameraFront;
		
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.cameraPos -= camera.cameraSpeed * camera.cameraFront;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.cameraPos -= glm::normalize(glm::cross(camera.cameraFront, camera.cameraUp)) * camera.cameraSpeed;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.cameraPos += glm::normalize(glm::cross(camera.cameraFront, camera.cameraUp)) * camera.cameraSpeed;


	static bool flashlightKeyPressed = false; 

	if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS && !flashlightKeyPressed) {
		std::cout << "F PRESS" << std::endl;
		camera.flashlightOn = !camera.flashlightOn;
		flashlightKeyPressed = true; 
	}
	if (glfwGetKey(window, GLFW_KEY_F) == GLFW_RELEASE) {
		flashlightKeyPressed = false; // Reiniciar la variable booleana cuando se suelta la tecla F
	}
		
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	if (camera.firstMouse) {
		camera.lastX = xpos;
		camera.lastY = ypos;
		camera.firstMouse = false;
	}

	float xoffset = xpos - camera.lastX;
	float yoffset = camera.lastY - ypos; // Reversed since y-coordinates go from bottom to top
	camera.lastX = xpos;
	camera.lastY = ypos;

	xoffset *= camera.mouseSensitivity;
	yoffset *= camera.mouseSensitivity;

	camera.yaw += xoffset;
	camera.pitch += yoffset;

	// Make sure that when pitch is out of bounds, screen doesn't get flipped
	if (camera.pitch > 89.0f)
		camera.pitch = 89.0f;
	if (camera.pitch < -89.0f)
		camera.pitch = -89.0f;

	glm::vec3 front;
	front.x = cos(glm::radians(camera.yaw)) * cos(glm::radians(camera.pitch));
	front.y = sin(glm::radians(camera.pitch));
	front.z = sin(glm::radians(camera.yaw)) * cos(glm::radians(camera.pitch));
	camera.cameraFront = glm::normalize(front);
}

class Texture {
private:
	int width, height, nrChannels;
	unsigned char* imageData;
	GLuint textureID;

public:
	Texture()
	{

	}
	Texture(const char* id)
	{
		imageData = stbi_load(id, &width, &height, &nrChannels, 0);
	}

	void CreateTexture(Texture texture)
	{
		const char* id = (const char*)texture.imageData;
		int _width = texture.width;
		int _height = texture.height;
		int _nrChannels = texture.nrChannels;


		imageData = stbi_load(id, &_width, &_height, &_nrChannels, 0);
	}

	void LoadTexture()
	{
		//Definimos canal de textura activo
		glActiveTexture(GL_TEXTURE0);


		glGenTextures(1, &textureID);

		//Vinculamos texture
		glBindTexture(GL_TEXTURE_2D, textureID);


		//Cargar datos de la imagen de la textura
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData);

		//Configuar textura
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

		//Generar mipmap
		glGenerateMipmap(GL_TEXTURE_2D);

		//Liberar memoria de la imagen cargada
		stbi_image_free(imageData);
	}

	void GetCroma(float r, float g, float b)
	{
		//Cromas
		int valuePosition = glGetUniformLocation(compiledPrograms[0], "color");

		if (valuePosition != -1)
		{
			glUniform3f(valuePosition, r, g, b);
		}
		//else
			//std::cout << "No se ha podido encontrar la direccion" << std::endl;
	}

	GLuint GetTextureID()
	{
		return textureID;
	}

};







struct ShaderProgram {
	GLuint vertexShader = 0;
	GLuint geometryShader = 0;
	GLuint fragmentShader = 0;
};



void Resize_Window(GLFWwindow* window, int iFrameBufferWidth, int iFrameBufferHeight) {

	//Definir nuevo tamaño del viewport
	glViewport(0, 0, iFrameBufferWidth, iFrameBufferHeight);
	glUniform2f(glGetUniformLocation(compiledPrograms[0], "windowSize"), iFrameBufferWidth, iFrameBufferHeight);
}

//Funcion que genera una matriz de escalado representada por un vector
glm::mat4 GenerateScaleMatrix(glm::vec3 scaleAxis) {

	return glm::scale(glm::mat4(1.0f), scaleAxis);
}

//Funcion que genera una matriz de rotacion dado un angulo y un vector
glm::mat4 GenerateRotationMatrix(glm::vec3 axis, float fDegrees) {

	return glm::rotate(glm::mat4(1.0f), glm::radians(fDegrees), glm::normalize(axis));
}

//Funcion que genera una matriz de traslacion representada por un vector
glm::mat4 GenerateTranslationMatrix(glm::vec3 translation) {

	return glm::translate(glm::mat4(1.0f), translation);
}


//Funcion que leera un .obj y devolvera un modelo para poder ser renderizado
Model LoadOBJModel(const std::string& filePath) {

	//Verifico archivo y si no puedo abrirlo cierro aplicativo
	std::ifstream file(filePath);

	if (!file.is_open()) {
		std::cerr << "No se ha podido abrir el archivo: " << filePath << std::endl;
		std::exit(EXIT_FAILURE);
	}

	//Variables lectura fichero
	std::string line;
	std::stringstream ss;
	std::string prefix;
	glm::vec3 tmpVec3;
	glm::vec2 tmpVec2;

	//Variables elemento modelo
	std::vector<float> vertexs;
	std::vector<float> vertexNormal;
	std::vector<float> textureCoordinates;

	//Variables temporales para algoritmos de sort
	std::vector<float> tmpVertexs;
	std::vector<float> tmpNormals;
	std::vector<float> tmpTextureCoordinates;

	//Recorremos archivo linea por linea
	while (std::getline(file, line)) {

		//Por cada linea reviso el prefijo del archivo que me indica que estoy analizando
		ss.clear();
		ss.str(line);
		ss >> prefix;

		//Estoy leyendo un vertice
		if (prefix == "v") {

			//Asumo que solo trabajo 3D así que almaceno XYZ de forma consecutiva
			ss >> tmpVec3.x >> tmpVec3.y >> tmpVec3.z;

			//Almaceno en mi vector de vertices los valores
			tmpVertexs.push_back(tmpVec3.x);
			tmpVertexs.push_back(tmpVec3.y);
			tmpVertexs.push_back(tmpVec3.z);
		}

		//Estoy leyendo una UV (texture coordinate)
		else if (prefix == "vt") {

			//Las UVs son siempre imagenes 2D asi que uso el tmpvec2 para almacenarlas
			ss >> tmpVec2.x >> tmpVec2.y;

			//Almaceno en mi vector temporal las UVs
			tmpTextureCoordinates.push_back(tmpVec2.x);
			tmpTextureCoordinates.push_back(tmpVec2.y);

		}

		//Estoy leyendo una normal
		else if (prefix == "vn") {

			//Asumo que solo trabajo 3D así que almaceno XYZ de forma consecutiva
			ss >> tmpVec3.x >> tmpVec3.y >> tmpVec3.z;

			//Almaceno en mi vector temporal de normales las normales
			tmpNormals.push_back(tmpVec3.x);
			tmpNormals.push_back(tmpVec3.y);
			tmpNormals.push_back(tmpVec3.z);

		}

		//Estoy leyendo una cara
		else if (prefix == "f") {

			int vertexData;
			short counter = 0;

			//Obtengo todos los valores hasta un espacio
			while (ss >> vertexData) {

				//En orden cada numero sigue el patron de vertice/uv/normal
				switch (counter) {
				case 0:
					//Si es un vertice lo almaceno - 1 por el offset y almaceno dos seguidos al ser un vec3, salto 1 / y aumento el contador en 1
					vertexs.push_back(tmpVertexs[(vertexData - 1) * 3]);
					vertexs.push_back(tmpVertexs[((vertexData - 1) * 3) + 1]);
					vertexs.push_back(tmpVertexs[((vertexData - 1) * 3) + 2]);
					ss.ignore(1, '/');
					counter++;
					break;
				case 1:
					//Si es un uv lo almaceno - 1 por el offset y almaceno dos seguidos al ser un vec2, salto 1 / y aumento el contador en 1
					textureCoordinates.push_back(tmpTextureCoordinates[(vertexData - 1) * 2]);
					textureCoordinates.push_back(tmpTextureCoordinates[((vertexData - 1) * 2) + 1]);
					ss.ignore(1, '/');
					counter++;
					break;
				case 2:
					//Si es una normal la almaceno - 1 por el offset y almaceno tres seguidos al ser un vec3, salto 1 / y reinicio
					vertexNormal.push_back(tmpNormals[(vertexData - 1) * 3]);
					vertexNormal.push_back(tmpNormals[((vertexData - 1) * 3) + 1]);
					vertexNormal.push_back(tmpNormals[((vertexData - 1) * 3) + 2]);
					counter = 0;
					break;
				}
			}
		}
	}
	return Model(vertexs, textureCoordinates, vertexNormal);
}


//Funcion que devolvera una string con todo el archivo leido
std::string Load_File(const std::string& filePath) {

	std::ifstream file(filePath);

	std::string fileContent;
	std::string line;

	//Lanzamos error si el archivo no se ha podido abrir
	if (!file.is_open()) {
		std::cerr << "No se ha podido abrir el archivo: " << filePath << std::endl;
		std::exit(EXIT_FAILURE);
	}

	//Leemos el contenido y lo volcamos a la variable auxiliar
	while (std::getline(file, line)) {
		fileContent += line + "\n";
	}

	//Cerramos stream de datos y devolvemos contenido
	file.close();

	return fileContent;
}

GLuint LoadFragmentShader(const std::string& filePath) {

	// Crear un fragment shader
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	//Usamos la funcion creada para leer el fragment shader y almacenarlo 
	std::string sShaderCode = Load_File(filePath);
	const char* cShaderSource = sShaderCode.c_str();

	//Vinculamos el fragment shader con su código fuente
	glShaderSource(fragmentShader, 1, &cShaderSource, nullptr);

	// Compilar el fragment shader
	glCompileShader(fragmentShader);

	// Verificar errores de compilación
	GLint success;
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);

	//Si la compilacion ha sido exitosa devolvemos el fragment shader
	if (success) {

		return fragmentShader;

	}
	else {

		//Obtenemos longitud del log
		GLint logLength;
		glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &logLength);

		//Obtenemos el log
		std::vector<GLchar> errorLog(logLength);
		glGetShaderInfoLog(fragmentShader, logLength, nullptr, errorLog.data());

		//Mostramos el log y finalizamos programa
		std::cerr << "Se ha producido un error al cargar el fragment shader:  " << errorLog.data() << std::endl;
		std::exit(EXIT_FAILURE);
	}
}


GLuint LoadGeometryShader(const std::string& filePath) {

	// Crear un vertex shader
	GLuint geometryShader = glCreateShader(GL_GEOMETRY_SHADER);

	//Usamos la funcion creada para leer el vertex shader y almacenarlo 
	std::string sShaderCode = Load_File(filePath);
	const char* cShaderSource = sShaderCode.c_str();

	//Vinculamos el vertex shader con su código fuente
	glShaderSource(geometryShader, 1, &cShaderSource, nullptr);

	// Compilar el vertex shader
	glCompileShader(geometryShader);

	// Verificar errores de compilación
	GLint success;
	glGetShaderiv(geometryShader, GL_COMPILE_STATUS, &success);

	//Si la compilacion ha sido exitosa devolvemos el vertex shader
	if (success) {

		return geometryShader;

	}
	else {

		//Obtenemos longitud del log
		GLint logLength;
		glGetShaderiv(geometryShader, GL_INFO_LOG_LENGTH, &logLength);

		//Obtenemos el log
		std::vector<GLchar> errorLog(logLength);
		glGetShaderInfoLog(geometryShader, logLength, nullptr, errorLog.data());

		//Mostramos el log y finalizamos programa
		std::cerr << "Se ha producido un error al cargar el vertex shader:  " << errorLog.data() << std::endl;
		std::exit(EXIT_FAILURE);
	}
}

GLuint LoadVertexShader(const std::string& filePath) {

	// Crear un vertex shader
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);

	//Usamos la funcion creada para leer el vertex shader y almacenarlo 
	std::string sShaderCode = Load_File(filePath);
	const char* cShaderSource = sShaderCode.c_str();

	//Vinculamos el vertex shader con su código fuente
	glShaderSource(vertexShader, 1, &cShaderSource, nullptr);

	// Compilar el vertex shader
	glCompileShader(vertexShader);

	// Verificar errores de compilación
	GLint success;
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);

	//Si la compilacion ha sido exitosa devolvemos el vertex shader
	if (success) {

		return vertexShader;

	}
	else {

		//Obtenemos longitud del log
		GLint logLength;
		glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &logLength);

		//Obtenemos el log
		std::vector<GLchar> errorLog(logLength);
		glGetShaderInfoLog(vertexShader, logLength, nullptr, errorLog.data());

		//Mostramos el log y finalizamos programa
		std::cerr << "Se ha producido un error al cargar el vertex shader:  " << errorLog.data() << std::endl;
		std::exit(EXIT_FAILURE);
	}
}

//Función que dado un struct que contiene los shaders de un programa generara el programa entero de la GPU
GLuint CreateProgram(const ShaderProgram& shaders) {

	//Crear programa de la GPU
	GLuint program = glCreateProgram();

	//Verificar que existe un vertex shader y adjuntarlo al programa
	if (shaders.vertexShader != 0) {
		glAttachShader(program, shaders.vertexShader);
	}

	if (shaders.geometryShader != 0) {
		glAttachShader(program, shaders.geometryShader);
	}

	if (shaders.fragmentShader != 0) {
		glAttachShader(program, shaders.fragmentShader);
	}

	// Linkear el programa
	glLinkProgram(program);

	//Obtener estado del programa
	GLint success;
	glGetProgramiv(program, GL_LINK_STATUS, &success);

	//Devolver programa si todo es correcto o mostrar log en caso de error
	if (success) {

		//Liberamos recursos
		if (shaders.vertexShader != 0) {
			glDetachShader(program, shaders.vertexShader);
		}

		//Liberamos recursos
		if (shaders.geometryShader != 0) {
			glDetachShader(program, shaders.geometryShader);
		}

		//Liberamos recursos
		if (shaders.fragmentShader != 0) {
			glDetachShader(program, shaders.fragmentShader);
		}

		return program;
	}
	else {

		//Obtenemos longitud del log
		GLint logLength;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);

		//Almacenamos log
		std::vector<GLchar> errorLog(logLength);
		glGetProgramInfoLog(program, logLength, nullptr, errorLog.data());

		std::cerr << "Error al linkar el programa:  " << errorLog.data() << std::endl;
		std::exit(EXIT_FAILURE);
	}
}

class GameObject {
public:

	glm::vec3 position = glm::vec3(0.f);
	glm::vec3 rotation = glm::vec3(0.f);
	glm::vec3 scale = glm::vec3(1.f);
	float r, g, b;

	Texture texture;

	glm::mat4 translationMatrix;
	glm::mat4 rotationMatrix;
	glm::mat4 scaleMatrix;

	float angle = 0.0f; // Ángulo inicial
	float radius = 2.0f; // Radio de la órbita
	float orbitSpeed = 0.2f; // Velocidad de la órbita

	GameObject(float r, float g, float b, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale, Texture _texture)
	{
		this->r = r;
		this->g = g;
		this->b = b;
		this->position = position;
		this->rotation = rotation;
		this->scale = scale;

		texture.CreateTexture(_texture);
	}

	void preCarga()
	{
		translationMatrix = GenerateTranslationMatrix(position);
		rotationMatrix = GenerateRotationMatrix(rotation, rotation.y);
		scaleMatrix = GenerateScaleMatrix(scale);
	}

	void Render(Texture _texture)
	{
		glUniformMatrix4fv(glGetUniformLocation(compiledPrograms[0], "translationMatrix"), 1, GL_FALSE, glm::value_ptr(translationMatrix));
		glUniformMatrix4fv(glGetUniformLocation(compiledPrograms[0], "rotationMatrix"), 1, GL_FALSE, glm::value_ptr(rotationMatrix));
		glUniformMatrix4fv(glGetUniformLocation(compiledPrograms[0], "scaleMatrix"), 1, GL_FALSE, glm::value_ptr(scaleMatrix));

		//Cambiar textura
		glBindTexture(GL_TEXTURE_2D, _texture.GetTextureID());
		//Croma
		_texture.GetCroma(r, g, b);
	}

	void ObjectLoadTexture()
	{
		texture.LoadTexture();
	}

private:

};


void updateSunPosition(GameObject sun, float deltaTime) {

	
}

void main() {

	//Definir semillas del rand según el tiempo
	srand(static_cast<unsigned int>(time(NULL)));

	//Inicializamos GLFW para gestionar ventanas e inputs
	glfwInit();


	//Configuramos la ventana
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
	glfwWindowHint(GLFW_DEPTH_BITS, 24); // Aseguramos un depth buffer de 24 bits

	//Inicializamos la ventana
	GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "My Engine", NULL, NULL);

	//Asignamos función de callback para cuando el frame buffer es modificado
	glfwSetFramebufferSizeCallback(window, Resize_Window);

	//Definimos espacio de trabajo
	glfwMakeContextCurrent(window);

	// Desactivar el cursor y capturar el movimiento del ratón
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Configurar los callbacks
	glfwSetCursorPosCallback(window, mouse_callback);

	//Permitimos a GLEW usar funcionalidades experimentales
	glewExperimental = GL_TRUE;

	//Activamos cull face
	glEnable(GL_CULL_FACE);

	//Indicamos lado del culling
	glCullFace(GL_BACK);

	//Leer textura
	Texture trollTexture("Assets/Textures/troll_v2.png");
	Texture rockTexture("Assets/Textures/rock_v2.png");
	Texture sunTexture("Assets/Textures/Cube_Texture.png");

	//Para los fps
	auto lastTime = std::chrono::high_resolution_clock::now();
	auto currentTime = std::chrono::high_resolution_clock::now();
	float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();

	//Inicializamos GLEW y controlamos errores
	if (glewInit() == GLEW_OK) {


		// Habilitamos el depth test
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);

		// Habilitamos cull face
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);


		glm::vec3 lookAt;

		//Compilar shaders
		ShaderProgram myFirstProgram;
		myFirstProgram.vertexShader = LoadVertexShader("MyFirstVertexShader.glsl");
		myFirstProgram.geometryShader = LoadGeometryShader("MyFirstGeometryShader.glsl");
		myFirstProgram.fragmentShader = LoadFragmentShader("MyFirstFragmentShader.glsl");

		//Cargo Modelo
		models.push_back(LoadOBJModel("Assets/Models/troll.obj"));
		models.push_back(LoadOBJModel("Assets/Models/rock.obj"));
		models.push_back(LoadOBJModel("Assets/Models/ball.obj"));

		//Compilar programa
		compiledPrograms.push_back(CreateProgram(myFirstProgram));

		GameObject troll1(1, 1, 1, glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), glm::vec3(0.2f, 0.2f, 0.2f), trollTexture);
		GameObject troll2(0, 1, 1, glm::vec3(0.5f, 0.f, 0.f), glm::vec3(0.f, 315.f, 0.f), glm::vec3(0.2f, 0.2f, 0.2f), trollTexture);
		GameObject troll3(1, 1, 0, glm::vec3(-0.5f, 0.f, 0.f), glm::vec3(0.f, 45.f, 0.f), glm::vec3(0.2f, 0.2f, 0.2f), trollTexture);
		GameObject rock1(1, 1, 1, glm::vec3(0.f, 0.f, 0.5f), glm::vec3(0.f, 45.f, 0.f), glm::vec3(0.2f, 0.2f, 0.2f), rockTexture);
		GameObject cloud1(3, 3, 3, glm::vec3(0.f, 0.8f, 0.f), glm::vec3(180.f, 90.f, 0.f), glm::vec3(0.3f, 0.2f, 0.2f), rockTexture);
		GameObject sun(255, 0, 0, glm::vec3(0.f, 10.0f, 0.f), glm::vec3(180.f, 90.f, 0.f), glm::vec3(0.001f, 0.001f, 0.001f), sunTexture);
		GameObject moon(255, 255, 255, glm::vec3(0.f, 10.0f, 0.f), glm::vec3(180.f, 90.f, 0.f), glm::vec3(0.001f, 0.001f, 0.001f), sunTexture);
		Light lightSun;

		camera.flashlightOn = false;
		camera.innerConeAngle = 12.5f;
		camera.outerConeAngle = 17.5f;

		moon.angle = 180;

		//LOAD TEXTURE
		trollTexture.LoadTexture();
		rockTexture.LoadTexture();
		sunTexture.LoadTexture();

		//Definimos color para limpiar el buffer de color
		glClearColor(0.f, 0.f, 0.f, 1.f);

		//Definimos modo de dibujo para cada cara
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		//Indicar a la tarjeta GPU que programa debe usar
		glUseProgram(compiledPrograms[0]);

		//Asignar valores iniciales al programa
		glUniform2f(glGetUniformLocation(compiledPrograms[0], "windowSize"), WINDOW_WIDTH, WINDOW_HEIGHT);

		//Asignar valor variable de textura a usar
		glUniform1d(glGetUniformLocation(compiledPrograms[0], "textureSampler"), 0);
		//Generamos el game loop

		//para que la camara orbite
		float cameraX;
		float cameraZ;

		while (!glfwWindowShouldClose(window)) {

			currentTime = std::chrono::high_resolution_clock::now();
			deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();

			processInput(window);

			//Movimiento sol
				// Incrementar el ángulo en función del tiempo
				sun.angle += sun.orbitSpeed * deltaTime;

				if (sun.angle >= 360.0f) {
					sun.angle -= 360.0f;
				}


				// Actualizar la posición de la esfera
				sun.position.y = sun.radius * sin(sun.angle);
				sun.position.z = sun.radius * cos(sun.angle);

				lightSun.position = sun.position;

			//Movimiento luna
				// Incrementar el ángulo en función del tiempo
				moon.angle += moon.orbitSpeed * deltaTime;

				// Actualizar la posición de la esfera
				moon.position.y = moon.radius * sin(moon.angle);
				moon.position.z = moon.radius * cos(moon.angle);

			//Pulleamos los eventos (botones, teclas, mouse...)
			glfwPollEvents();

			if (glfwGetKey(window, GLFW_KEY_PERIOD) == GLFW_PRESS) {

				camera.fFov += 1.0f;

				if (camera.fFov > 180.f) {
					camera.fFov = 180;
				}
			}
			if (glfwGetKey(window, GLFW_KEY_COMMA) == GLFW_PRESS) {

				camera.fFov -= 1.0f;

				if (camera.fFov < 1.f) {
					camera.fFov = 1;
				}
			}

			

			troll1.preCarga();
			troll2.preCarga();
			troll3.preCarga();
			rock1.preCarga();
			cloud1.preCarga();
			sun.preCarga();
			moon.preCarga();

			glm::mat4 viewMatrix = glm::lookAt(camera.cameraPos, camera.cameraPos + camera.cameraFront, camera.cameraUp);
			glUniformMatrix4fv(glGetUniformLocation(compiledPrograms[0], "viewMatrix"), 1, GL_FALSE, glm::value_ptr(viewMatrix));

			glm::mat4 projectionMatrix = glm::perspective(glm::radians(camera.fov), (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, camera.fNear, camera.fFar);
			glUniformMatrix4fv(glGetUniformLocation(compiledPrograms[0], "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(projectionMatrix));

			glUniformMatrix4fv(glGetUniformLocation(compiledPrograms[0], "viewMatrix"), 1, GL_FALSE, glm::value_ptr(viewMatrix));
			glUniformMatrix4fv(glGetUniformLocation(compiledPrograms[0], "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(projectionMatrix));

			GLint lightDirLocation = glGetUniformLocation(compiledPrograms[0], "lightDirection");
			glUniform3f(lightDirLocation, lightSun.position.x, lightSun.position.y, lightSun.position.z);
			glm::vec3 lightDirection = glm::normalize(glm::vec3(1.0f, -1.0f, 0.0f));



			
			glUseProgram(compiledPrograms[0]);
			GLuint shaderProgram = compiledPrograms[0];
			GLuint lightPosLoc = glGetUniformLocation(shaderProgram, "lightPosition");
			GLuint moonPosLoc = glGetUniformLocation(shaderProgram, "moonPosition");
			GLuint cameraPosLoc = glGetUniformLocation(shaderProgram, "cameraPosition");
			GLuint cameraFt = glGetUniformLocation(shaderProgram, "cameraFront");
			GLuint flashlightOnLoc = glGetUniformLocation(shaderProgram, "flashlightOn");
			
			GLuint innerConeAngleLoc = glGetUniformLocation(shaderProgram, "innerConeAngle");
			GLuint outerConeAngleLoc = glGetUniformLocation(shaderProgram, "outerConeAngle");

			//GLuint maxDistanceLoc = glGetUniformLocation(shaderProgram, "maxDistance");

			

			glUniform3f(lightPosLoc, sun.position.x, sun.position.y, sun.position.z);
			glUniform3f(moonPosLoc, moon.position.x, moon.position.y, moon.position.z);
			glUniform3f(cameraPosLoc, camera.cameraPos.x, camera.cameraPos.y, camera.cameraPos.z);
			glUniform3f(cameraFt, camera.cameraFront.x, camera.cameraFront.y, camera.cameraFront.z);
			glUniform1i(flashlightOnLoc, camera.flashlightOn ? 1 : 0);

			glUniform1f(outerConeAngleLoc, camera.outerConeAngle);
			glUniform1f(innerConeAngleLoc, camera.innerConeAngle);

			//glUniform1f(maxDistanceLoc, camera.maxDistance);

			/*glUniform3f(flashlightPosLoc, camera.cameraPos.x, camera.cameraPos.y, camera.cameraPos.z);
			glUniform3f(flashlightDirLoc, camera.cameraFront.x, camera.cameraFront.y, camera.cameraFront.z);
			glUniform1f(innerConeAngleLoc, camera.innerConeAngle);
			glUniform1f(outerConeAngleLoc, camera.outerConeAngle);
			glUniform1i(flashlightOnLoc, camera.flashlightOn ? 1 : 0);*/

			
			


			//Limpiamos los buffers
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

			troll1.Render(trollTexture);
			models[0].Render();

			troll2.Render(trollTexture);
			models[0].Render();

			troll3.Render(trollTexture);
			models[0].Render();

			rock1.Render(rockTexture);
			models[1].Render();

			sun.Render(sunTexture);
			models[2].Render();

			moon.Render(sunTexture);
			models[2].Render();

			cloud1.Render(rockTexture);
			models[1].Render();



			// Guardar el tiempo actual para el próximo fotograma
			lastTime = currentTime;

			//Cambiamos buffers
			glFlush();
			glfwSwapBuffers(window);
		}

		//Desactivar y eliminar programa
		glUseProgram(0);
		glDeleteProgram(compiledPrograms[0]);

	}
	else {
		std::cout << "Ha petao." << std::endl;
		glfwTerminate();
	}

	//Finalizamos GLFW
	glfwTerminate();

}