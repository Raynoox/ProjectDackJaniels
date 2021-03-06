#include "GL/glew.h"
#include "GL/freeglut.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <stdio.h>
#include "tga.h"
#include "shaderprogram.h"
#include "cube.h"
#include "teapot.h"
#include "Model_obj.cpp"

//Macierze
glm::mat4  matP;//rzutowania
glm::mat4  matV;//widoku
glm::mat4  matM;//modelu

//Modele
Model_OBJ dack;

// k�t horyzontalny (na -Z)
float horizontalAngle = 3.14f;
//k�t horyzontalny (na horyzont)
float verticalAngle = 0.0f;
//pr�dko�� ruchu kamery
float speed = 0.1f;
//pr�dko�� obracania kamery
float mouseSpeed = 0.00005f;
int mxpos, mypos;
POINT pt;
// wektory pozycji, do g�ry, i w prawo
glm::vec3 position = glm::vec3(0, 0, 5);
glm::vec3 direction(
	cos(verticalAngle) * sin(horizontalAngle),
	sin(verticalAngle),
	cos(verticalAngle) * cos(horizontalAngle)
	);

glm::vec3 right = glm::vec3(
	sin(horizontalAngle - 3.14f / 2.0f),
	0,
	cos(horizontalAngle - 3.14f / 2.0f)
	);

//Ustawienia okna i rzutowania
int windowPositionX = 500;
int windowPositionY = 100;
int windowWidth = 900;
int windowHeight = 600;
float cameraAngle = 45.0f;

//Zmienne do animacji
float speed_x = 0;
float speed_y = 0;
int lastTime = 0;
float angle_x = 0;
float angle_y = 0;

//Uchwyty na shadery
ShaderProgram *shaderProgram; //Wska�nik na obiekt reprezentuj�cy program cieniuj�cy.

//Uchwyty na VAO i bufory wierzcho�k�w
GLuint vao;
GLuint bufVertices; //Uchwyt na bufor VBO przechowuj�cy tablic� wsp�rz�dnych wierzcho�k�w
GLuint bufColors;  //Uchwyt na bufor VBO przechowuj�cy tablic� kolor�w
GLuint bufNormals; //Uchwyt na bufor VBO przechowuj�cy tablic� wektor�w normalnych
GLuint bufTexCoords; //Uchwyt na bufor VBO przechowuj�cy tablic� wsp�rzednych teksturowania

//uchwyty kostka
GLuint vao2;
GLuint bufVertices2;
GLuint bufColors2;
GLuint bufNormals2;

//Uchwyty na tekstury
GLuint tex0;
GLuint tex1;

//"Model" kt�ry rysujemy. Dane wskazywane przez poni�sze wska�niki i o zadanej liczbie wierzcho�k�w s� p�niej wysowane przez program.
//W programie s� dwa modele, z kt�rych jeden mo�na wybra� komentuj�c/odkomentowuj�c jeden z poni�szych fragment�w.

//Kostka
float *vertices2 = cubeVertices;
float *colors2 = cubeColors;
float *normals2 = cubeNormals;
//float *texCoords=cubeTexCoords;
int vertexCount2 = cubeVertexCount;

//Czajnik
float *vertices = teapotVertices;
float *colors = teapotColors;
float *normals = teapotNormals2;
float *texCoords = teapotTexCoords;
int vertexCount = teapotVertexCount;


//Procedura rysuj�ca jaki� obiekt. Ustawia odpowiednie parametry dla vertex shadera i rysuje.
void drawObject() {
	//W��czenie programu cieniuj�cego, kt�ry ma zosta� u�yty do rysowania
	//W tym programie wystarczy�oby wywo�a� to raz, w setupShaders, ale chodzi o pokazanie,
	//�e mozna zmienia� program cieniuj�cy podczas rysowania jednej sceny
	shaderProgram->use();

	//Przeka� do shadera macierze P,V i M.
	//W linijkach poni�ej, polecenie:
	//  shaderProgram->getUniformLocation("P")
	//pobiera numer slotu odpowiadaj�cego zmiennej jednorodnej o podanej nazwie
	//UWAGA! "P" w powy�szym poleceniu odpowiada deklaracji "uniform mat4 P;" w vertex shaderze,
	//a matP w glm::value_ptr(matP) odpowiada deklaracji  "glm::mat4 matP;" TYM pliku.
	//Ca�a poni�sza linijka przekazuje do zmiennej jednorodnej P w vertex shaderze dane ze zmiennej matP
	//zadeklarowanej globalnie w tym pliku.
	//Pozosta�e polecenia dzia�aj� podobnie.
	glUniformMatrix4fv(shaderProgram->getUniformLocation("P"), 1, false, glm::value_ptr(matP));
	glUniformMatrix4fv(shaderProgram->getUniformLocation("V"), 1, false, glm::value_ptr(matV));
	glUniformMatrix4fv(shaderProgram->getUniformLocation("M"), 1, false, glm::value_ptr(matM));

	glUniform4f(shaderProgram->getUniformLocation("lightPosition"), 10, 0, 10, 1);
	glUniform4f(shaderProgram->getUniformLocation("lightPosition"), 1000, 1000, 1000, 1);

	glUniform1i(shaderProgram->getUniformLocation("textureMap0"), 0);
	glUniform1i(shaderProgram->getUniformLocation("textureMap1"), 1);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, tex1);

	//Uaktywnienie VAO i tym samym uaktywnienie predefiniowanych w tym VAO powi�za� slot�w atrybut�w z tablicami z danymi
	glBindVertexArray(vao);

	//Narysowanie obiektu
	glDrawArrays(GL_TRIANGLES, 0, vertexCount);

	//Posprz�tanie po sobie (niekonieczne w sumie je�eli korzystamy z VAO dla ka�dego rysowanego obiektu)
	glBindVertexArray(0);

	glBindVertexArray(vao2);

	glDrawArrays(GL_TRIANGLES, 0, vertexCount2);

	glBindVertexArray(0);
}

//Procedura rysuj�ca
void displayFrame() {

	//Wyczy�� bufor kolor�w i bufor g��boko�ci
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//obliczenie kierunku "patrzenia"
	glm::vec3 direction(
		cos(verticalAngle) * sin(horizontalAngle),
		sin(verticalAngle),
		cos(verticalAngle) * cos(horizontalAngle)
		);

	glm::vec3 right = glm::vec3(
		sin(horizontalAngle - 3.14f / 2.0f),
		0,
		cos(horizontalAngle - 3.14f / 2.0f)
		);
	glm::vec3 up = glm::cross(right, direction);
	//Wylicz macierz rzutowania
	matP = glm::perspective(cameraAngle, (float)windowWidth / (float)windowHeight, 1.0f, 100.0f);

	//Wylicz macierz widoku
	matV = glm::lookAt(position, position + direction, up);

	//Wylicz macierz modelu
	matM = glm::rotate(glm::mat4(1.0f), angle_x, glm::vec3(1, 0, 0));
	matM = glm::rotate(matM, angle_y, glm::vec3(0, 1, 0));

	//Narysuj obiekt

	drawObject();

	//Tylny bufor na przedni
	glutSwapBuffers();
}

GLuint makeBuffer(void *data, int vertexCount, int vertexSize) {
	GLuint handle;

	glGenBuffers(1, &handle);//Wygeneruj uchwyt na Vertex Buffer Object (VBO), kt�ry b�dzie zawiera� tablic� danych
	glBindBuffer(GL_ARRAY_BUFFER, handle);  //Uaktywnij wygenerowany uchwyt VBO
	glBufferData(GL_ARRAY_BUFFER, vertexCount*vertexSize, data, GL_STATIC_DRAW);//Wgraj tablic� do VBO

	return handle;
}

//Procedura tworz�ca bufory VBO zawieraj�ce dane z tablic opisuj�cych rysowany obiekt.
void setupVBO() {
	vertices2 = dack.vertexBuffer;
	normals2 = dack.normals;
	colors2 = dack.colors;
	vertexCount2 = dack.TotalConnectedPoints/4;
	cout << vertexCount2 << " " << vertexCount << endl;
	bufVertices = makeBuffer(vertices, vertexCount, sizeof(float) * 4); //Wsp�rz�dne wierzcho�k�w
	bufColors = makeBuffer(colors, vertexCount, sizeof(float) * 4);//Kolory wierzcho�k�w
	bufNormals = makeBuffer(normals, vertexCount, sizeof(float) * 4);//Wektory normalne wierzcho�k�w
	bufTexCoords = makeBuffer(texCoords, vertexCount, sizeof(float) * 4);//Wektory normalne wierzcho�k�w

	bufVertices2 = makeBuffer(vertices2, vertexCount2, sizeof(float) * 4);
	bufColors2 = makeBuffer(colors2, vertexCount2, sizeof(float) * 4);
	bufNormals2 = makeBuffer(normals2, vertexCount2, sizeof(float) * 4);
}

void assignVBOtoAttribute(char* attributeName, GLuint bufVBO, int variableSize) {
	GLuint location = shaderProgram->getAttribLocation(attributeName); //Pobierz numery slot�w dla atrybutu
	glBindBuffer(GL_ARRAY_BUFFER, bufVBO);  //Uaktywnij uchwyt VBO
	glEnableVertexAttribArray(location); //W��cz u�ywanie atrybutu o numerze slotu zapisanym w zmiennej location
	glVertexAttribPointer(location, variableSize, GL_FLOAT, GL_FALSE, 0, NULL); //Dane do slotu location maj� by� brane z aktywnego VBO
}

//Procedura tworz�ca VAO - "obiekt" OpenGL wi���cy numery slot�w atrybut�w z buforami VBO
void setupVAO() {
	//Wygeneruj uchwyt na VAO i zapisz go do zmiennej globalnej
	glGenVertexArrays(1, &vao);
	glGenVertexArrays(1, &vao2);
	//Uaktywnij nowo utworzony VAO
	glBindVertexArray(vao);
	assignVBOtoAttribute("vertex", bufVertices, 4); //"vertex" odnosi si� do deklaracji "in vec4 vertex;" w vertex shaderze
	//assignVBOtoAttribute("color", bufColors, 4); //"color" odnosi si� do deklaracji "in vec4 color;" w vertex shaderze
	assignVBOtoAttribute("normal", bufNormals, 4); //"normal" odnosi si� do deklaracji "in vec4 normal;" w vertex shaderze
	assignVBOtoAttribute("texCoord", bufTexCoords, 2); //"texCoord" odnosi si� do deklaracji "in vec2 texCoord;" w vertex shaderze

	glBindVertexArray(0);

	//kostka
	glBindVertexArray(vao2);
	assignVBOtoAttribute("vertex", bufVertices2, 4);
	//assignVBOtoAttribute("color", bufColors2, 4);
	assignVBOtoAttribute("normal", bufNormals2, 4);

	glBindVertexArray(0);
}

//Procedura uruchamiana okresowo. Robi animacj�.
void nextFrame(void) {
	int actTime = glutGet(GLUT_ELAPSED_TIME);
	int interval = actTime - lastTime;
	lastTime = actTime;
	angle_x += speed_x*interval / 1000.0;
	angle_y += speed_y*interval / 1000.0;
	if (angle_x>360) angle_x -= 360;
	if (angle_y>360) angle_y -= 360;
	if (angle_x<0) angle_x += 360;
	if (angle_y<0) angle_y += 360;
	//resetowanie pozycji myszy zeby nie wyjechala
	GetCursorPos(&pt);
	mxpos = pt.x - windowPositionX;
	mypos = pt.y - windowPositionY;
	SetCursorPos(windowWidth / 2 + windowPositionX, windowHeight / 2 + windowPositionY);
	horizontalAngle += mouseSpeed  *interval* float(windowWidth / 2 - mxpos);
	verticalAngle += mouseSpeed  *interval * float(windowHeight / 2 - mypos);
	glutPostRedisplay();
}

void keyDown(int c, int x, int y) {
	switch (c) {
	case GLUT_KEY_LEFT:
		speed_y = -120;
		break;
	case GLUT_KEY_RIGHT:
		speed_y = 120;
		break;
	case GLUT_KEY_UP:
		speed_x = -120;
		break;
	case GLUT_KEY_DOWN:
		speed_x = 120;
		break;
	case GLUT_KEY_END: //end ko�czy
		exit(EXIT_SUCCESS);
	}
}

void keyUp(int c, int x, int y) {
	switch (c) {
	case GLUT_KEY_LEFT:
		speed_y = 0;
		break;
	case GLUT_KEY_RIGHT:
		speed_y = 0;
		break;
	case GLUT_KEY_UP:
		speed_x = 0;
		break;
	case GLUT_KEY_DOWN:
		speed_x = 0;
		break;
	}
}

void moveMouse(int x, int y)
{

}

void keyUnpressed(unsigned char key, int x, int y)
{
}
void keyPressed(unsigned char key, int x, int y){
	// nie mam pojecia dlaczego, ale globalnie nie dziala wiec tutaj obliczam nowe wektory
	glm::vec3 direction(
		cos(verticalAngle) * sin(horizontalAngle),
		0,
		cos(verticalAngle) * cos(horizontalAngle)
		);
	// zamieni� je�eli chcesz mie� woln� kamer� ( do g�ry i w d� te� mozna sie porusza�)
	/*glm::vec3 direction(
	cos(verticalAngle) * sin(horizontalAngle),
	sin(verticalAngle),
	cos(verticalAngle) * cos(horizontalAngle)
	);*/

	glm::vec3 right = glm::vec3(
		sin(horizontalAngle - 3.14f / 2.0f),
		0,
		cos(horizontalAngle - 3.14f / 2.0f)
		);

	if (key == 'w')
	{
		position += direction * speed;
	};//ruch do przodu
	if (key == 's')
	{
		position -= direction * speed;
	};//ruch do tylu
	if (key == 'a')
	{
		position -= right * speed;
	}
	if (key == 'd')
	{
		position += right * speed;
	}
}

//Procedura wywo�ywana przy zmianie rozmiaru okna
void changeSize(int w, int h) {
	//Ustawienie wymiarow przestrzeni okna
	glViewport(0, 0, w, h);
	//Zapami�tanie nowych wymiar�w okna dla poprawnego wyliczania macierzy rzutowania
	windowWidth = w;
	windowHeight = h;
}

//Procedura inicjuj�ca biblotek� glut
void initGLUT(int *argc, char** argv) {
	glutInit(argc, argv); //Zainicjuj bibliotek� GLUT
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH); //Alokuj bufory kolor�w (podw�jne buforowanie) i bufor kolor�w

	glutInitWindowPosition(windowPositionX, windowPositionY); //Wska� pocz�tkow� pozycj� okna
	glutInitWindowSize(windowWidth, windowHeight); //Wska� pocz�tkowy rozmiar okna
	glutCreateWindow("OpenGL 3.3"); //Utw�rz okno i nadaj mu tytu�

	glutReshapeFunc(changeSize); //Zarejestruj procedur� changeSize jako procedur� obs�uguj�ca zmian� rozmiaru okna
	glutDisplayFunc(displayFrame); //Zarejestruj procedur� displayFrame jako procedur� obs�uguj�ca od�wierzanie okna
	glutIdleFunc(nextFrame); //Zarejestruj procedur� nextFrame jako procedur� wywo�ywan� najcz�ci�j jak si� da (animacja)

	glutSpecialFunc(keyDown);
	glutSpecialUpFunc(keyUp);

	glutKeyboardFunc(keyPressed);
	glutKeyboardUpFunc(keyUnpressed);

	//do myszki
	glutPassiveMotionFunc(moveMouse);
	SetCursorPos(windowWidth / 2 + windowPositionX, windowHeight / 2 + windowPositionY);
}


//Procedura inicjuj�ca bibliotek� glew
void initGLEW() {
	GLenum err = glewInit();
	if (GLEW_OK != err) {
		fprintf(stderr, "%s\n", glewGetErrorString(err));
		exit(1);
	}

}



//Wczytuje vertex shader i fragment shader i ��czy je w program cieniuj�cy
void setupShaders() {
	shaderProgram = new ShaderProgram("vshader.txt", NULL, "fshader.txt");
}



GLuint readTexture(char* filename) {
	GLuint tex;
	TGAImg img;
	glActiveTexture(GL_TEXTURE0);
	if (img.Load(filename) == IMG_OK) {
		glGenTextures(1, &tex); //Zainicjuj uchwyt tex
		glBindTexture(GL_TEXTURE_2D, tex); //Przetwarzaj uchwyt tex
		if (img.GetBPP() == 24) //Obrazek 24bit
			glTexImage2D(GL_TEXTURE_2D, 0, 3, img.GetWidth(), img.GetHeight(), 0,
			GL_RGB, GL_UNSIGNED_BYTE, img.GetImg());
		else if (img.GetBPP() == 32) //Obrazek 32bit
			glTexImage2D(GL_TEXTURE_2D, 0, 4, img.GetWidth(), img.GetHeight(), 0,
			GL_RGBA, GL_UNSIGNED_BYTE, img.GetImg());
		else {
			printf("Nieobs�ugiwany format obrazka w pliku: %s \n", filename);
		}
	}
	else {
		printf("B��d przy wczytywaniu pliku: %s\n", filename);
	}
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	return tex;
}

//procedura inicjuj�ca r�ne sprawy zwi�zane z rysowaniem w OpenGL
void initOpenGL() {
	setupShaders();
	setupVBO();
	setupVAO();
	glEnable(GL_DEPTH_TEST);

	tex0 = readTexture("metal.tga");
	tex1 = readTexture("metal_spec.tga");
}

//Zwolnij pami�� karty graficznej z shader�w i programu cieniuj�cego
void cleanShaders() {
	delete shaderProgram;
}

void freeVBO() {
	glDeleteBuffers(1, &bufVertices);
	glDeleteBuffers(1, &bufColors);
	glDeleteBuffers(1, &bufNormals);
}

void freeVAO() {
	glDeleteVertexArrays(1, &vao);
}


int main(int argc, char** argv) {
	dack.Load("dack1.obj");
	initGLUT(&argc, argv);
	initGLEW();
	initOpenGL();

	

	glutMainLoop();

	freeVAO();
	freeVBO();
	cleanShaders();
	return 0;
}