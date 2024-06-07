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
		std::cout << "W PRESS" << std::endl;
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.cameraPos -= camera.cameraSpeed * camera.cameraFront;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.cameraPos -= glm::normalize(glm::cross(camera.cameraFront, camera.cameraUp)) * camera.cameraSpeed;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.cameraPos += glm::normalize(glm::cross(camera.cameraFront, camera.cameraUp)) * camera.cameraSpeed;
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

struct GameObject {

	glm::vec3 position = glm::vec3(0.f);
	glm::vec3 rotation = glm::vec3(0.f);
	glm::vec3 scale = glm::vec3(1.f);
	float r, g, b;

};



struct ShaderProgram {
	GLuint vertexShader = 0;
	GLuint geometryShader = 0;
	GLuint fragmentShader = 0;
};

class Texture {
private:
	int width, height, nrChannels;
	unsigned char* imageData;
	GLuint textureID;

public:
	Texture(const char* id)
	{
		imageData = stbi_load(id, &width, &height, &nrChannels, 0);
	}

	void CreateTexture(const char* id)
	{
		imageData = stbi_load(id, &width, &height, &nrChannels, 0);
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

	void GetCroma(GameObject gameObject)
	{
		//Cromas
		int valuePosition = glGetUniformLocation(compiledPrograms[0], "color");

		if (valuePosition != -1)
		{
			glUniform3f(valuePosition, gameObject.r, gameObject.g, gameObject.b);
		}
		else
			std::cout << "No se ha podido encontrar la direccion" << std::endl;
	}

	GLuint GetTextureID()
	{
		return textureID;
	}

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

	//Para los fps
	auto lastTime = std::chrono::high_resolution_clock::now();

	//Inicializamos GLEW y controlamos errores
	if (glewInit() == GLEW_OK) {


		// Habilitamos el depth test
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);

		// Habilitamos cull face
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		GameObject troll1;
		GameObject troll2;
		GameObject troll3;

		GameObject rock1;

		GameObject cloud1;

		troll1.r = 1;
		troll1.g = 1;
		troll1.b = 1;

		troll2.r = 0;
		troll2.g = 1;
		troll2.b = 1;

		troll3.r = 1;
		troll3.g = 1;
		troll3.b = 0;

		rock1.r = 1;
		rock1.g = 1;
		rock1.b = 1;

		cloud1.r = 3;
		cloud1.g = 3;
		cloud1.b = 3;


		glm::vec3 lookAt;

		//Compilar shaders
		ShaderProgram myFirstProgram;
		myFirstProgram.vertexShader = LoadVertexShader("MyFirstVertexShader.glsl");
		myFirstProgram.geometryShader = LoadGeometryShader("MyFirstGeometryShader.glsl");
		myFirstProgram.fragmentShader = LoadFragmentShader("MyFirstFragmentShader.glsl");

		//Cargo Modelo
		models.push_back(LoadOBJModel("Assets/Models/troll.obj"));
		models.push_back(LoadOBJModel("Assets/Models/rock.obj"));

		//Compilar programa
		compiledPrograms.push_back(CreateProgram(myFirstProgram));

		//LOAD TEXTURE
		trollTexture.LoadTexture();
		rockTexture.LoadTexture();

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

			processInput(window);

			auto currentTime = std::chrono::high_resolution_clock::now();
			float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
			lastTime = currentTime;

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

			//Que plano de camara hacer en funcion del input 
			switch (stateCamera)
			//{
			//case CameraStates::STATE1:

			//	//troll izquierda plano general
			//	camera.position = glm::vec3(camera.position.x, camera.position.y, camera.position.z);
			//	/*lookAt = glm::vec3(-0.9f, 0.f, 0.f);

			//	camera.fFov = 45;*/

			//	break;
			//case CameraStates::STATE2:

			//	//troll derecha plano detalle
			//	camera.position = glm::vec3(0.3f, 0.45f, 0.77f);
			//	lookAt = glm::vec3(0.9f, 0.f, 0.f);

			//	camera.fFov = 10;

			//	break;
			//case CameraStates::STATE3:

			//	//troll medio plano
			//	camera.position = glm::vec3(0.f, 0.5f, 1.2f);
			//	lookAt = glm::vec3(0.f, 0.f, 0.f); // Punto central de la escena

			//	camera.fFov = 45;

			//	break;

			//case CameraStates::ORBIT:

			//	camera.orbitAngle += camera.orbitVelocity * deltaTime;

			//	cameraX = sin(camera.orbitAngle) * camera.orbitRadius; // Coordenada x
			//	cameraZ = cos(camera.orbitAngle) * camera.orbitRadius; // Coordenada z

			//	camera.position.x = cameraX;
			//	camera.position.z = cameraZ;

			//	lookAt = glm::vec3(0.f, 0.f, 0.f); // Punto central de la escena

			//	camera.fFov = 45;

			//	break;

			//default:
			//	break;
			//}

			troll1.position = glm::vec3(0.f, 0.f, 0.f);
			troll1.rotation = glm::vec3(0.f, 1.f, 0.f);
			troll1.scale = glm::vec3(0.2f, 0.2f, 0.2f);

			troll2.position = glm::vec3(0.5f, 0.f, 0.f);
			troll2.rotation = glm::vec3(0.f, 315.f, 0.f);
			troll2.scale = glm::vec3(0.2f, 0.2f, 0.2f);

			troll3.position = glm::vec3(-0.5f, 0.f, 0.f);
			troll3.rotation = glm::vec3(0.f, 45.f, 0.f);
			troll3.scale = glm::vec3(0.2f, 0.2f, 0.2f);

			rock1.position = glm::vec3(0.f, 0.f, 0.5f);
			rock1.rotation = glm::vec3(0.f, 45.f, 0.f);
			rock1.scale = glm::vec3(0.2f, 0.2f, 0.2f);

			cloud1.position = glm::vec3(0.f, 0.8f, 0.f);
			cloud1.rotation = glm::vec3(180.f, 90.f, 0.f);
			cloud1.scale = glm::vec3(0.3f, 0.2f, 0.2f);

			glm::mat4 translationMatrix = GenerateTranslationMatrix(troll1.position);
			glm::mat4 rotationMatrix = GenerateRotationMatrix(troll1.rotation, troll1.rotation.y);
			glm::mat4 scaleMatrix = GenerateScaleMatrix(troll1.scale);

			glm::mat4 translationMatrix2 = GenerateTranslationMatrix(troll2.position);
			glm::mat4 rotationMatrix2 = GenerateRotationMatrix(troll2.rotation, troll2.rotation.y);
			glm::mat4 scaleMatrix2 = GenerateScaleMatrix(troll2.scale);

			glm::mat4 translationMatrix3 = GenerateTranslationMatrix(troll3.position);
			glm::mat4 rotationMatrix3 = GenerateRotationMatrix(troll3.rotation, troll3.rotation.y);
			glm::mat4 scaleMatrix3 = GenerateScaleMatrix(troll3.scale);

			glm::mat4 translationMatrix4 = GenerateTranslationMatrix(rock1.position);
			glm::mat4 rotationMatrix4 = GenerateRotationMatrix(rock1.rotation, rock1.rotation.y);
			glm::mat4 scaleMatrix4 = GenerateScaleMatrix(rock1.scale);

			glm::mat4 translationMatrix5 = GenerateTranslationMatrix(cloud1.position);
			glm::mat4 rotationMatrix5 = GenerateRotationMatrix(cloud1.rotation, cloud1.rotation.y);
			glm::mat4 scaleMatrix5 = GenerateScaleMatrix(cloud1.scale);

			glm::mat4 viewMatrix = glm::lookAt(camera.cameraPos, camera.cameraPos + camera.cameraFront, camera.cameraUp);
			glUniformMatrix4fv(glGetUniformLocation(compiledPrograms[0], "viewMatrix"), 1, GL_FALSE, glm::value_ptr(viewMatrix));

			glm::mat4 projectionMatrix = glm::perspective(glm::radians(camera.fov), (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, camera.fNear, camera.fFar);
			glUniformMatrix4fv(glGetUniformLocation(compiledPrograms[0], "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(projectionMatrix));
			
			glUniformMatrix4fv(glGetUniformLocation(compiledPrograms[0], "viewMatrix"), 1, GL_FALSE, glm::value_ptr(viewMatrix));
			glUniformMatrix4fv(glGetUniformLocation(compiledPrograms[0], "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(projectionMatrix));

			//Limpiamos los buffers
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

			glUniformMatrix4fv(glGetUniformLocation(compiledPrograms[0], "translationMatrix"), 1, GL_FALSE, glm::value_ptr(translationMatrix));
			glUniformMatrix4fv(glGetUniformLocation(compiledPrograms[0], "rotationMatrix"), 1, GL_FALSE, glm::value_ptr(rotationMatrix));
			glUniformMatrix4fv(glGetUniformLocation(compiledPrograms[0], "scaleMatrix"), 1, GL_FALSE, glm::value_ptr(scaleMatrix));
			//Cambiar textura
			glBindTexture(GL_TEXTURE_2D, trollTexture.GetTextureID());
			//Croma
			trollTexture.GetCroma(troll1);
			models[0].Render();

			glUniformMatrix4fv(glGetUniformLocation(compiledPrograms[0], "translationMatrix"), 1, GL_FALSE, glm::value_ptr(translationMatrix2));
			glUniformMatrix4fv(glGetUniformLocation(compiledPrograms[0], "rotationMatrix"), 1, GL_FALSE, glm::value_ptr(rotationMatrix2));
			glUniformMatrix4fv(glGetUniformLocation(compiledPrograms[0], "scaleMatrix"), 1, GL_FALSE, glm::value_ptr(scaleMatrix2));
			//Cambiar textura
			glBindTexture(GL_TEXTURE_2D, trollTexture.GetTextureID());
			//Croma
			trollTexture.GetCroma(troll2);
			models[0].Render();

			glUniformMatrix4fv(glGetUniformLocation(compiledPrograms[0], "translationMatrix"), 1, GL_FALSE, glm::value_ptr(translationMatrix3));
			glUniformMatrix4fv(glGetUniformLocation(compiledPrograms[0], "rotationMatrix"), 1, GL_FALSE, glm::value_ptr(rotationMatrix3));
			glUniformMatrix4fv(glGetUniformLocation(compiledPrograms[0], "scaleMatrix"), 1, GL_FALSE, glm::value_ptr(scaleMatrix3));
			//Cambiar textura
			glBindTexture(GL_TEXTURE_2D, trollTexture.GetTextureID());
			//Croma
			trollTexture.GetCroma(troll3);
			models[0].Render();

			glUniformMatrix4fv(glGetUniformLocation(compiledPrograms[0], "translationMatrix"), 1, GL_FALSE, glm::value_ptr(translationMatrix4));
			glUniformMatrix4fv(glGetUniformLocation(compiledPrograms[0], "rotationMatrix"), 1, GL_FALSE, glm::value_ptr(rotationMatrix4));
			glUniformMatrix4fv(glGetUniformLocation(compiledPrograms[0], "scaleMatrix"), 1, GL_FALSE, glm::value_ptr(scaleMatrix4));
			//Cambiar textura
			glBindTexture(GL_TEXTURE_2D, rockTexture.GetTextureID());
			//Croma
			rockTexture.GetCroma(rock1);
			models[1].Render();

			glUniformMatrix4fv(glGetUniformLocation(compiledPrograms[0], "translationMatrix"), 1, GL_FALSE, glm::value_ptr(translationMatrix5));
			glUniformMatrix4fv(glGetUniformLocation(compiledPrograms[0], "rotationMatrix"), 1, GL_FALSE, glm::value_ptr(rotationMatrix5));
			glUniformMatrix4fv(glGetUniformLocation(compiledPrograms[0], "scaleMatrix"), 1, GL_FALSE, glm::value_ptr(scaleMatrix5));
			//Cambiar textura
			glBindTexture(GL_TEXTURE_2D, rockTexture.GetTextureID());
			//Croma
			rockTexture.GetCroma(cloud1);
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