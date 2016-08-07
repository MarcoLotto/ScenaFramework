/**********************************
* SCAENA FRAMEWORK
* Author: Marco Andrés Lotto
* License: MIT - 2016
**********************************/

#include <GLES3/gl3.h>
#include <GLES3/gl2ext.h>

#include <iostream>
#include <fstream>
#include <map>
#include "GLSLProgramGLES3Implementation.h"
#include "InvalidUniformNameException.h"
#include "InvalidSubroutineNameException.h"
#include "FeatureNotAvaibleException.h"
#include "Logger.h"
using namespace std;

GLSLProgramGLES3Implementation::GLSLProgramGLES3Implementation(){
	this->programHandle = 0;	
	this->linked = false;
	this->compiled = false;
	this->logString = "";
	firstTime=true;

	this->vertexShaderName = "";
	this->geometryShaderName = "";
	this->tesselationEvaluationShaderName = "";
	this->tesselationControlShaderName = "";
	this->computeShaderName = "";
	this->fragmentShaderName = "";	
}
GLSLProgramGLES3Implementation::~GLSLProgramGLES3Implementation(){

}

int GLSLProgramGLES3Implementation::getUniformLocationByName(string name){
	// Verifico si existe en la cache, si existe devuelvo la location
	map<string, int>::iterator it = this->uniformlocationsCache.find(name);
	if(it != this->uniformlocationsCache.end()){
		return it->second;
	}

	// No existe en cache, si existe en el shader agrego la location a al cache
	int location = glGetUniformLocation(this->getHandle(), name.c_str());
	if(location != -1){
		this->uniformlocationsCache[name] = location; 
		return location;
	}
	// REVIEW: La location no existe en el shader, no deberia estar buscandola
	//throw new InvalidUniformNameException(name);
    return 0;
}

int GLSLProgramGLES3Implementation::getSubroutineLocationByName(string name, unsigned int shaderType){
	throw new FeatureNotAvaibleException("OpenGL ES 3 no permite subrutinas en shaders");
	return 0;
}

//Compila el shader pasado por parametro. El segundo argumento es el tipo de shader a compilar
bool GLSLProgramGLES3Implementation::compileShaderFromFile( const char * fileName, GLSLShader::GLSLShaderType type ){

	//Creo el shader correspondiente
	GLuint shader = 0;
	if(type == GLSLShader::VERTEX)
		shader = glCreateShader( GL_VERTEX_SHADER );
	else if(type == GLSLShader::FRAGMENT)
		shader = glCreateShader( GL_FRAGMENT_SHADER );
	if( 0 == shader )
	{
		logString =  "Error creando el shader " + string(fileName);
		Logger::getInstance()->logError(new Log(logString));
		this->compiled = false;
		return false;
	}

	//Cargo el codigo fuente
	std::ifstream v_shader_file(fileName, std::ifstream::in);
    std::string v_str((std::istreambuf_iterator<char>(v_shader_file)), std::istreambuf_iterator<char>());
    const char* vs_code_array[] = {v_str.c_str()};        
    glShaderSource( shader, 1, vs_code_array, NULL);

	//Compilo el shader
	glCompileShader( shader );

	//Verifico que se haya compilado correctamente
	GLint result;
	glGetShaderiv( shader, GL_COMPILE_STATUS, &result );
	if( GL_FALSE == result )
	{
		logString =  "Error compilando el shader " + string(fileName);
		Logger::getInstance()->logError(new Log(logString));

		GLint logLen;
		glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &logLen );
		if( logLen > 0 )
		{
			char * log = (char *)malloc(logLen);
		
		GLsizei written;
		glGetShaderInfoLog(shader, logLen, &written, log);

		logString =  "Shader log: \n" + string(log);
		Logger::getInstance()->logError(new Log(logString));

		free(log);		
		}
		this->compiled = false;
		return false;
	}

	//Creo el program handle
	if(firstTime){
		this->programHandle = glCreateProgram();
		if( 0 == programHandle )
		{
			this->compiled=false;
			logString =  "Error creando el shader program handler " + string(fileName);
			Logger::getInstance()->logError(new Log(logString));		
		}
		firstTime = false;
	}
		
	//Asocio el shader al program handle
	glAttachShader( programHandle, shader );
	glDeleteShader(shader);

	this->compiled = true;
	return true;
}

//Asocia el indice "location" al nombre de variable del shader
void GLSLProgramGLES3Implementation::bindAttribLocation( GLuint location, const char * name){
	glBindAttribLocation( this->programHandle, location, name );
}

//Asocia el output del fragment shader a un indice "location"
void GLSLProgramGLES3Implementation::bindFragDataLocation( GLuint location, const char * name ){
	// REVIEW: Verificar si es correcto sacar esto
	//glBindFragDataLocation(this->programHandle, location, name);
}

//Linkea los shaders al programa
bool GLSLProgramGLES3Implementation::link(string programBaseName){
	
	//linkeo el shader
	glLinkProgram( programHandle );

	GLint status;
	glGetProgramiv( programHandle, GL_LINK_STATUS, &status );
	if( GL_FALSE == status ) {
		logString =  "Error linkeando el shader " + string(programBaseName);
		Logger::getInstance()->logError(new Log(logString));

		GLint logLen;
		glGetProgramiv(programHandle, GL_INFO_LOG_LENGTH,
		&logLen);
		if( logLen > 0 )
		{
			char * log = (char *)malloc(logLen);
			GLsizei written;
			glGetProgramInfoLog(programHandle, logLen,
			&written, log);
	
			logString =  "Shader log: \n" + string(log);
			Logger::getInstance()->logError(new Log(logString));

			free(log);
		}
		this->linked = false;
		return false;
	}	

	this->linked = true;
	return true;	
}

//Empiza a usar los shaders cargados.
void GLSLProgramGLES3Implementation::use(){
	if(this->linked)
		glUseProgram( programHandle );
}

int GLSLProgramGLES3Implementation::getHandle(){
	return this->programHandle;
}


bool GLSLProgramGLES3Implementation::isLinked(){
	return this->linked;
}

void GLSLProgramGLES3Implementation::setUniform(GLSLUniform* uniform, float x, float y, float z){	
	glUniform3f(uniform->getUniformLocation(), x, y, z);
}

void GLSLProgramGLES3Implementation::setUniform(GLSLUniform* uniform, const glm::vec2 & v){	
	glUniform2fv(uniform->getUniformLocation(), 1, &v[0]);
}

void GLSLProgramGLES3Implementation::setUniform(GLSLUniform* uniform, const glm::vec3 & v){	
	glUniform3fv(uniform->getUniformLocation(), 1, &v[0]);
}

void GLSLProgramGLES3Implementation::setUniform(GLSLUniform* uniform, int size, const glm::vec3 & v){	
	glUniform3fv(uniform->getUniformLocation(), size, &v[0]);
}

void GLSLProgramGLES3Implementation::setUniform(GLSLUniform* uniform, const glm::vec4 & v){	
	glUniform4fv(uniform->getUniformLocation(), 1, &v[0]);
}

void GLSLProgramGLES3Implementation::setUniform(GLSLUniform* uniform, int size, const glm::vec4 & v){	
	glUniform4fv(uniform->getUniformLocation(), size, &v[0]);
}

void GLSLProgramGLES3Implementation::setUniform(GLSLUniform* uniform, const glm::mat4 &m){	
	glUniformMatrix4fv(uniform->getUniformLocation(), 1, GL_FALSE, &m[0][0]);
}

void GLSLProgramGLES3Implementation::setUniform(GLSLUniform* uniform, const glm::mat3 & m){
	glUniformMatrix3fv(uniform->getUniformLocation(), 1, GL_FALSE, &m[0][0]);
}

void GLSLProgramGLES3Implementation::setUniform(GLSLUniform* uniform, const glm::mat2 & m){
	glUniformMatrix2fv(uniform->getUniformLocation(), 1, GL_FALSE, &m[0][0]);
}

void GLSLProgramGLES3Implementation::setUniform(GLSLUniform* uniform, int size, const glm::mat4 & m){	
	glUniformMatrix4fv(uniform->getUniformLocation(), size, GL_FALSE, &m[0][0]);
}

void GLSLProgramGLES3Implementation::setUniform(GLSLUniform* uniform, int size, const glm::mat3 & m){
	glUniformMatrix3fv(uniform->getUniformLocation(), size, GL_FALSE, &m[0][0]);
}

void GLSLProgramGLES3Implementation::setUniform(GLSLUniform* uniform, int size, const glm::mat2 & m){
	glUniformMatrix2fv(uniform->getUniformLocation(), size, GL_FALSE, &m[0][0]);
}


void GLSLProgramGLES3Implementation::setUniform(GLSLUniform* uniform, float val ){
	glUniform1f(uniform->getUniformLocation(), val);
}

void GLSLProgramGLES3Implementation::setUniform(GLSLUniform* uniform, int val ){
	glUniform1i(uniform->getUniformLocation(), val);
}

void GLSLProgramGLES3Implementation::setUniform(GLSLUniform* uniform, unsigned int val ){
	glUniform1ui(uniform->getUniformLocation(), val);
}

void GLSLProgramGLES3Implementation::setUniform(GLSLUniform* uniform, bool val ){
	glUniform1i(uniform->getUniformLocation(), val);
}

void GLSLProgramGLES3Implementation::setVectorUniform(GLSLUniform* uniform, unsigned int size, float* vector ){
	glUniform1fv(uniform->getUniformLocation(), size, vector);
}

void GLSLProgramGLES3Implementation::setVectorUniform(GLSLUniform* uniform, unsigned int size, int* vector ){
	glUniform1iv(uniform->getUniformLocation(), size, vector);
}

void GLSLProgramGLES3Implementation::setVectorUniform(GLSLUniform* uniform, unsigned int size, unsigned int* vector ){	
	glUniform1uiv(uniform->getUniformLocation(), size, vector);
}
int GLSLProgramGLES3Implementation::getUniformLocation(const char *name){
	return getUniformLocationByName(name);
}

void GLSLProgramGLES3Implementation::changeSubroutineInFragmentShader(const char* subroutineName){
	throw new FeatureNotAvaibleException("OpenGL ES 3 no permite subrutinas en shaders");
}

void GLSLProgramGLES3Implementation::initialize(list<string>* inputAttributes, list<string>* outputAttributes, bool usedForFeedback){
	// Evito que un shader se inicialize dos veces
	if(!this->firstTime){
		return;
	}
	
	//Compilo los shaders (dependiendo de cuales haya disponibles)
	if(this->vertexShaderName.compare("") != 0)
		this->compileShaderFromFile(this->vertexShaderName.c_str(), GLSLShader::VERTEX);
	if(this->geometryShaderName.compare("") != 0)
		this->compileShaderFromFile(this->geometryShaderName.c_str(), GLSLShader::GEOMETRY);
	if(this->tesselationEvaluationShaderName.compare("") != 0)
		this->compileShaderFromFile(this->tesselationEvaluationShaderName.c_str(), GLSLShader::TESS_EVALUATION);
	if(this->tesselationControlShaderName.compare("") != 0)
		this->compileShaderFromFile(this->tesselationControlShaderName.c_str(), GLSLShader::TESS_CONTROL);
	if(this->fragmentShaderName.compare("") != 0)
		this->compileShaderFromFile(this->fragmentShaderName.c_str(), GLSLShader::FRAGMENT);	

	//Mapeo los atributos de entrada
	unsigned int i = 0;
	std::list<string>::iterator inputIt = inputAttributes->begin();
	while(inputIt != inputAttributes->end()){
		glBindAttribLocation( this->getHandle() , i, (*inputIt).c_str() );
		++inputIt;
		++i;
	}
	//Mapeo los atributos de salida (dependiendo si los uso para feedback o no)
	if(!usedForFeedback){
		i = 0;
		std::list<string>::iterator outputIt = outputAttributes->begin();
		while(outputIt != outputAttributes->end()){
			// REVIEW: Verificar si es correcto sacar esto
			//glBindFragDataLocation(this->getHandle(), i, (*outputIt).c_str());
			++outputIt;
			++i;
		}
	}
	else{
		unsigned int i = 0;
		const char** output = new const char*[outputAttributes->size()]();
		std::list<string>::iterator outputIt = outputAttributes->begin();
		while(outputIt != outputAttributes->end()){				
			output[i] = (*outputIt).c_str();
			++outputIt;
			++i;
		}
		glTransformFeedbackVaryings(this->getHandle(), outputAttributes->size(), output, GL_SEPARATE_ATTRIBS);
		delete output;		
	}
	//Linkeo el programa (le paso el nombre del vertex shader para el logguer nomas)
	this->link(string(vertexShaderName).substr(0, this->vertexShaderName.length() - 5));	
}


// Permito setear en clases hijas los nombres stages de shader que quieren usar
void GLSLProgramGLES3Implementation::setVertexShaderName(string filename){
	this->vertexShaderName = filename;
}
void GLSLProgramGLES3Implementation::setGeometryShaderName(string filename){
	this->geometryShaderName = filename;
}
void GLSLProgramGLES3Implementation::setTesselationEvaluationShaderName(string filename){
	this->tesselationEvaluationShaderName = filename;
}
void GLSLProgramGLES3Implementation::setTesselationControlShaderName(string filename){
	this->tesselationControlShaderName = filename;
}
void GLSLProgramGLES3Implementation::setComputeShaderName(string filename){
	this->computeShaderName = filename;
}
void GLSLProgramGLES3Implementation::setFragmentShaderName(string filename){
	this->fragmentShaderName = filename;
}
