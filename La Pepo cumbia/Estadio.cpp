
#include <iostream>
#include <vector>
#include <cmath>
#include <string>
#include "ModeloOBJ.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// =====================================================
// CONFIGURACION GENERAL
// =====================================================

const unsigned int ANCHO_VENTANA = 1280;
const unsigned int ALTO_VENTANA = 720;

const float PI = 3.14159265359f;

// Dimensiones del campo (proporciones aproximadas)
const float CAMPO_ANCHO = 68.0f;
const float CAMPO_LARGO = 105.0f;

const float MITAD_ANCHO = CAMPO_ANCHO / 2.0f;
const float MITAD_LARGO = CAMPO_LARGO / 2.0f;

// Vector donde guardaremos:
// X, Y, Z, R, G, B
std::vector<float> vertices;

// =====================================================
// CAMBIAR VIEWPORT AL REDIMENSIONAR LA VENTANA
// =====================================================

void cambiarTamano(GLFWwindow* window, int ancho, int alto)
{
    glViewport(0, 0, ancho, alto);
}

// =====================================================
// AGREGAR UN VERTICE
// =====================================================

void agregarVertice(
    float x, float y, float z,
    const glm::vec3& color)
{
    vertices.push_back(x);
    vertices.push_back(y);
    vertices.push_back(z);

    vertices.push_back(color.r);
    vertices.push_back(color.g);
    vertices.push_back(color.b);
}

// =====================================================
// CREAR RECTANGULO HORIZONTAL CON DOS TRIANGULOS
// =====================================================

void agregarRectangulo(
    float xMin, float xMax,
    float zMin, float zMax,
    float altura,
    const glm::vec3& color)
{
    // Primer triangulo
    agregarVertice(xMin, altura, zMin, color);
    agregarVertice(xMax, altura, zMin, color);
    agregarVertice(xMax, altura, zMax, color);

    // Segundo triangulo
    agregarVertice(xMin, altura, zMin, color);
    agregarVertice(xMax, altura, zMax, color);
    agregarVertice(xMin, altura, zMax, color);
}

// =====================================================
// CREAR LINEA SOBRE EL CAMPO
// =====================================================

void agregarLineaVertical(
    float x,
    float zMin,
    float zMax,
    float grosor,
    const glm::vec3& color)
{
    agregarRectangulo(
        x - grosor / 2.0f,
        x + grosor / 2.0f,
        zMin,
        zMax,
        0.035f,
        color
    );
}

void agregarLineaHorizontal(
    float z,
    float xMin,
    float xMax,
    float grosor,
    const glm::vec3& color)
{
    agregarRectangulo(
        xMin,
        xMax,
        z - grosor / 2.0f,
        z + grosor / 2.0f,
        0.035f,
        color
    );
}

// =====================================================
// CREAR CIRCULO COMO ANILLO DE TRIANGULOS
// =====================================================

void agregarCirculo(
    float centroX,
    float centroZ,
    float radio,
    float grosor,
    float altura,
    const glm::vec3& color)
{
    const int segmentos = 100;

    for (int i = 0; i < segmentos; i++)
    {
        float angulo1 = 2.0f * PI * i / segmentos;
        float angulo2 = 2.0f * PI * (i + 1) / segmentos;

        float radioInterior = radio - grosor / 2.0f;
        float radioExterior = radio + grosor / 2.0f;

        float x1i = centroX + cos(angulo1) * radioInterior;
        float z1i = centroZ + sin(angulo1) * radioInterior;

        float x1e = centroX + cos(angulo1) * radioExterior;
        float z1e = centroZ + sin(angulo1) * radioExterior;

        float x2i = centroX + cos(angulo2) * radioInterior;
        float z2i = centroZ + sin(angulo2) * radioInterior;

        float x2e = centroX + cos(angulo2) * radioExterior;
        float z2e = centroZ + sin(angulo2) * radioExterior;

        agregarVertice(x1i, altura, z1i, color);
        agregarVertice(x1e, altura, z1e, color);
        agregarVertice(x2e, altura, z2e, color);

        agregarVertice(x1i, altura, z1i, color);
        agregarVertice(x2e, altura, z2e, color);
        agregarVertice(x2i, altura, z2i, color);
    }
}

// =====================================================
// CREAR PUNTO CIRCULAR
// =====================================================

void agregarPunto(
    float centroX,
    float centroZ,
    float radio,
    float altura,
    const glm::vec3& color)
{
    const int segmentos = 32;

    for (int i = 0; i < segmentos; i++)
    {
        float a1 = 2.0f * PI * i / segmentos;
        float a2 = 2.0f * PI * (i + 1) / segmentos;

        agregarVertice(
            centroX, altura, centroZ, color
        );

        agregarVertice(
            centroX + cos(a1) * radio,
            altura,
            centroZ + sin(a1) * radio,
            color
        );

        agregarVertice(
            centroX + cos(a2) * radio,
            altura,
            centroZ + sin(a2) * radio,
            color
        );
    }
}

// =====================================================
// CONSTRUIR TODA LA CANCHA
// =====================================================

void construirCancha()
{
    vertices.clear();

    // Colores del cesped
    glm::vec3 verdeOscuro(0.055f, 0.34f, 0.12f);
    glm::vec3 verdeClaro(0.085f, 0.43f, 0.15f);

    glm::vec3 blanco(0.96f, 0.96f, 0.92f);

    // -------------------------------------------------
    // 1. FRANJAS DE CESPED
    // -------------------------------------------------

    const int numeroFranjas = 10;

    float largoFranja = CAMPO_LARGO / numeroFranjas;

    for (int i = 0; i < numeroFranjas; i++)
    {
        float zMin = -MITAD_LARGO + i * largoFranja;
        float zMax = zMin + largoFranja;

        glm::vec3 color =
            (i % 2 == 0) ? verdeOscuro : verdeClaro;

        agregarRectangulo(
            -MITAD_ANCHO,
            MITAD_ANCHO,
            zMin,
            zMax,
            0.0f,
            color
        );
    }

    // -------------------------------------------------
    // 2. CONTORNO DEL CAMPO
    // -------------------------------------------------

    const float grosorLinea = 0.18f;

    // Laterales
    agregarLineaVertical(
        -MITAD_ANCHO,
        -MITAD_LARGO,
        MITAD_LARGO,
        grosorLinea,
        blanco
    );

    agregarLineaVertical(
        MITAD_ANCHO,
        -MITAD_LARGO,
        MITAD_LARGO,
        grosorLinea,
        blanco
    );

    // Lineas de fondo
    agregarLineaHorizontal(
        -MITAD_LARGO,
        -MITAD_ANCHO,
        MITAD_ANCHO,
        grosorLinea,
        blanco
    );

    agregarLineaHorizontal(
        MITAD_LARGO,
        -MITAD_ANCHO,
        MITAD_ANCHO,
        grosorLinea,
        blanco
    );

    // -------------------------------------------------
    // 3. LINEA CENTRAL
    // -------------------------------------------------

    agregarLineaHorizontal(
        0.0f,
        -MITAD_ANCHO,
        MITAD_ANCHO,
        grosorLinea,
        blanco
    );

    // -------------------------------------------------
    // 4. CIRCULO CENTRAL
    // -------------------------------------------------

    agregarCirculo(
        0.0f,
        0.0f,
        9.15f,
        0.18f,
        0.04f,
        blanco
    );

    agregarPunto(
        0.0f,
        0.0f,
        0.35f,
        0.045f,
        blanco
    );

    // -------------------------------------------------
    // 5. AREAS DE PENALTI
    // -------------------------------------------------

    const float anchoArea = 40.32f;
    const float largoArea = 16.5f;

    const float anchoAreaPequena = 18.32f;
    const float largoAreaPequena = 5.5f;

    // Area de penalti norte
    agregarLineaVertical(
        -anchoArea / 2.0f,
        MITAD_LARGO - largoArea,
        MITAD_LARGO,
        grosorLinea,
        blanco
    );

    agregarLineaVertical(
        anchoArea / 2.0f,
        MITAD_LARGO - largoArea,
        MITAD_LARGO,
        grosorLinea,
        blanco
    );

    agregarLineaHorizontal(
        MITAD_LARGO - largoArea,
        -anchoArea / 2.0f,
        anchoArea / 2.0f,
        grosorLinea,
        blanco
    );

    // Area de penalti sur
    agregarLineaVertical(
        -anchoArea / 2.0f,
        -MITAD_LARGO,
        -MITAD_LARGO + largoArea,
        grosorLinea,
        blanco
    );

    agregarLineaVertical(
        anchoArea / 2.0f,
        -MITAD_LARGO,
        -MITAD_LARGO + largoArea,
        grosorLinea,
        blanco
    );

    agregarLineaHorizontal(
        -MITAD_LARGO + largoArea,
        -anchoArea / 2.0f,
        anchoArea / 2.0f,
        grosorLinea,
        blanco
    );

    // -------------------------------------------------
    // 6. AREAS PEQUENAS
    // -------------------------------------------------

    // Norte
    agregarLineaVertical(
        -anchoAreaPequena / 2.0f,
        MITAD_LARGO - largoAreaPequena,
        MITAD_LARGO,
        grosorLinea,
        blanco
    );

    agregarLineaVertical(
        anchoAreaPequena / 2.0f,
        MITAD_LARGO - largoAreaPequena,
        MITAD_LARGO,
        grosorLinea,
        blanco
    );

    agregarLineaHorizontal(
        MITAD_LARGO - largoAreaPequena,
        -anchoAreaPequena / 2.0f,
        anchoAreaPequena / 2.0f,
        grosorLinea,
        blanco
    );

    // Sur
    agregarLineaVertical(
        -anchoAreaPequena / 2.0f,
        -MITAD_LARGO,
        -MITAD_LARGO + largoAreaPequena,
        grosorLinea,
        blanco
    );

    agregarLineaVertical(
        anchoAreaPequena / 2.0f,
        -MITAD_LARGO,
        -MITAD_LARGO + largoAreaPequena,
        grosorLinea,
        blanco
    );

    agregarLineaHorizontal(
        -MITAD_LARGO + largoAreaPequena,
        -anchoAreaPequena / 2.0f,
        anchoAreaPequena / 2.0f,
        grosorLinea,
        blanco
    );

    // -------------------------------------------------
    // 7. PUNTOS DE PENALTI
    // -------------------------------------------------

    float distanciaPenal = 11.0f;

    agregarPunto(
        0.0f,
        MITAD_LARGO - distanciaPenal,
        0.35f,
        0.045f,
        blanco
    );

    agregarPunto(
        0.0f,
        -MITAD_LARGO + distanciaPenal,
        0.35f,
        0.045f,
        blanco
    );
}

// =====================================================
// ESCENARIO INTERACTIVO: grada, zonas laterales, porterias,
// marcador, luces, jugador jerarquico y animacion por keyframes.
// =====================================================

struct Caja { glm::vec3 centro, escala, color; };
std::vector<float> geometriaEscenario;
std::vector<Caja> cajas;
bool teclas[1024] = {};
bool primeraPersona = false, reflectores = true, animando = false;
int golesLocal = 0, golesVisita = 0;
float anguloMouseX = -90.0f, anguloMouseY = 43.0f;
float zoomCamara = 165.0f;
float campoVision = 55.0f;
double mouseXAnterior = ANCHO_VENTANA / 2.0, mouseYAnterior = ALTO_VENTANA / 2.0;
bool primerMovimientoMouse = true;
glm::vec3 jugador(0.0f, 0.0f, 0.0f);
glm::vec3 arbitro(7.0f, 0.0f, -4.0f);
float tiempoAnimacion = 0.0f;
struct KeyframeBalon { float tiempo, x, y, z; };
const KeyframeBalon keyframesBalon[] = {
    {0.00f,-0.60f,0.35f,0.0f}, {0.35f,-6.00f,0.85f,0.0f},
    {0.80f,-17.0f,2.50f,0.0f}, {1.35f,-31.0f,3.20f,0.0f},
    {1.90f,-45.0f,0.70f,0.0f}, {2.20f,-50.0f,0.35f,0.0f}
};
const glm::vec3 AMARILLO(0.95f, 0.72f, 0.12f), AZUL(0.04f, 0.25f, 0.82f);

void agregarCaja(float x, float y, float z, float sx, float sy, float sz, glm::vec3 c) {
    cajas.push_back({ glm::vec3(x,y,z), glm::vec3(sx,sy,sz), c });
}

void agregarTriangulo(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 color) {
    agregarVertice(a.x,a.y,a.z,color); agregarVertice(b.x,b.y,b.z,color); agregarVertice(c.x,c.y,c.z,color);
}

void agregarCilindroVertical(float x, float z, float radio, float alto, glm::vec3 color) {
    const int n=12;
    for(int i=0;i<n;++i) {
        float a=2*PI*i/n, b=2*PI*(i+1)/n;
        glm::vec3 p1(x+cos(a)*radio,0,z+sin(a)*radio), p2(x+cos(b)*radio,0,z+sin(b)*radio);
        glm::vec3 p3=p2+glm::vec3(0,alto,0), p4=p1+glm::vec3(0,alto,0);
        agregarTriangulo(p1,p2,p3,color); agregarTriangulo(p1,p3,p4,color);
        agregarTriangulo(glm::vec3(x,alto,z),p4,p3,color);
    }
}

void construirEscenario() {
    construirCancha();
    glm::vec3 gris(0.28f,0.32f,0.38f), concreto(0.42f,0.43f,0.40f), blanco(0.96f);
    // Zona 2: gradas en ambos laterales, con pasillos y asientos en niveles.
    for(int lado=-1;lado<=1;lado+=2) {
        for(int fila=0;fila<9;++fila) {
            float x=lado*(39.0f+fila*1.65f), y=1.0f+fila*0.78f;
            agregarCaja(x,y,-2,3.2f,0.65f,96,gris);
            for(int z=-44;z<=44;z+=4) {
                glm::vec3 asiento=((z/4+fila)%2==0)?AZUL:AMARILLO;
                agregarCaja(x-lado*0.2f,y+0.42f,(float)z,2.1f,0.45f,2.1f,asiento);
            }
        }
        // Torre de iluminacion y luminarias encendidas/apagadas por tecla F.
        agregarCaja(lado*58.0f,17.0f,0,0.8f,34,0.8f,concreto);
        for(int j=0;j<5;++j) agregarCaja(lado*58.0f,34.0f,(-2.0f+j)*1.3f,1.5f,0.8f,1.0f,reflectores?glm::vec3(1.0f,0.92f,0.55f):gris);
    }
    // Zona 3: bancas/calentamiento y cabinas en las cabeceras.
    for(int lado=-1;lado<=1;lado+=2) {
        for(int i=0;i<8;++i) agregarCaja(-12.0f+i*3.4f,0.6f,lado*59.0f,2.8f,1.2f,1.0f,AZUL);
        agregarCaja(0,14.0f,lado*58.0f,30.0f,5.0f,3.0f,gris);
        agregarCaja(0,17.0f,lado*58.0f,28.0f,0.5f,3.4f,AMARILLO);
    }
    // Perimetro, accesos, tuneles y torres de soporte.
    agregarCaja(0,0.0f,-69,138,0.3f,0.5f,concreto); agregarCaja(0,0.0f,69,138,0.3f,0.5f,concreto);
    agregarCaja(-69,0,0,0.5f,0.3f,138,concreto); agregarCaja(69,0,0,0.5f,0.3f,138,concreto);
    for(int x=-2;x<=2;++x) agregarCaja(x*2.2f,2.1f,57,1.8f,4.2f,3.0f,AZUL);
}

void cursor(GLFWwindow*, double x, double y) {
    if(primerMovimientoMouse) { mouseXAnterior=x; mouseYAnterior=y; primerMovimientoMouse=false; }
    float dx=(float)(x-mouseXAnterior), dy=(float)(mouseYAnterior-y); mouseXAnterior=x; mouseYAnterior=y;
    anguloMouseX += dx*0.10f;
    anguloMouseY=primeraPersona ? glm::clamp(anguloMouseY+dy*0.10f,-85.0f,85.0f) : glm::clamp(anguloMouseY+dy*0.10f,10.0f,80.0f);
}

void rueda(GLFWwindow*, double, double desplazamiento) {
    if(primeraPersona) campoVision=glm::clamp(campoVision-(float)desplazamiento*2.5f,30.0f,75.0f);
    else zoomCamara=glm::clamp(zoomCamara-(float)desplazamiento*14.0f,45.0f,320.0f);
}

void tecla(GLFWwindow* w,int key,int,int action,int) {
    if(key==GLFW_KEY_ESCAPE&&action==GLFW_PRESS) glfwSetWindowShouldClose(w,GL_TRUE);
    if(key>=0&&key<1024) { if(action==GLFW_PRESS) teclas[key]=true; if(action==GLFW_RELEASE) teclas[key]=false; }
    if(action==GLFW_PRESS) {
        if(key==GLFW_KEY_V) primeraPersona=!primeraPersona;
        if(key==GLFW_KEY_F) reflectores=!reflectores;
        if(key==GLFW_KEY_G) ++golesLocal;
        if(key==GLFW_KEY_H) ++golesVisita;
        if(key==GLFW_KEY_R) { golesLocal=golesVisita=0; }
        if(key==GLFW_KEY_SPACE) { animando=true; tiempoAnimacion=0.0f; }
        if(key==GLFW_KEY_PAGE_UP || key==GLFW_KEY_KP_ADD || key==GLFW_KEY_EQUAL) {
            if(primeraPersona) campoVision=glm::max(30.0f,campoVision-4.0f); else zoomCamara=glm::max(45.0f,zoomCamara-14.0f);
        }
        if(key==GLFW_KEY_PAGE_DOWN || key==GLFW_KEY_KP_SUBTRACT || key==GLFW_KEY_MINUS) {
            if(primeraPersona) campoVision=glm::min(75.0f,campoVision+4.0f); else zoomCamara=glm::min(320.0f,zoomCamara+14.0f);
        }
        if(key==GLFW_KEY_HOME) { zoomCamara=165.0f; campoVision=55.0f; }
    }
}

glm::mat4 transformacion(glm::vec3 p,glm::vec3 s) {
    glm::mat4 m(1.0f); m=glm::translate(m,p); return glm::scale(m,s);
}

// Cubo unitario (36 vertices), reutilizado por todos los elementos jerarquicos.
const float cubo[] = {
 -0.5f,-0.5f,-0.5f, 0.5f,-0.5f,-0.5f, 0.5f,0.5f,-0.5f, 0.5f,0.5f,-0.5f,-0.5f,0.5f,-0.5f,-0.5f,-0.5f,-0.5f,
 -0.5f,-0.5f,0.5f, 0.5f,-0.5f,0.5f, 0.5f,0.5f,0.5f, 0.5f,0.5f,0.5f,-0.5f,0.5f,0.5f,-0.5f,-0.5f,0.5f,
 -0.5f,0.5f,0.5f,-0.5f,0.5f,-0.5f,-0.5f,-0.5f,-0.5f,-0.5f,-0.5f,-0.5f,-0.5f,-0.5f,0.5f,-0.5f,0.5f,0.5f,
 0.5f,0.5f,0.5f,0.5f,0.5f,-0.5f,0.5f,-0.5f,-0.5f,0.5f,-0.5f,-0.5f,0.5f,-0.5f,0.5f,0.5f,0.5f,0.5f,
 -0.5f,-0.5f,-0.5f,0.5f,-0.5f,-0.5f,0.5f,-0.5f,0.5f,0.5f,-0.5f,0.5f,-0.5f,-0.5f,0.5f,-0.5f,-0.5f,-0.5f,
 -0.5f,0.5f,-0.5f,0.5f,0.5f,-0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,-0.5f,0.5f,0.5f,-0.5f,0.5f,-0.5f
};

int main() {
    if(!glfwInit()) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3); glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
    glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window=glfwCreateWindow(ANCHO_VENTANA,ALTO_VENTANA,"La Pepo Cumbia | Estadio interactivo",nullptr,nullptr);
    if(!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window); glfwSwapInterval(1); glewExperimental=GL_TRUE;
    if(glewInit()!=GLEW_OK) { glfwDestroyWindow(window); glfwTerminate(); return -1; }
    std::cout<<"Controles: WASD mover | mouse mirar/orbitar | rueda del mouse o RePág/AvPág zoom | V primera/tercera persona | F reflectores | espacio tiro | G/H marcador | R reiniciar | ESC salir"<<std::endl;
    glfwSetFramebufferSizeCallback(window,cambiarTamano); glfwSetKeyCallback(window,tecla); glfwSetCursorPosCallback(window,cursor); glfwSetScrollCallback(window,rueda);
    glfwSetInputMode(window,GLFW_CURSOR,GLFW_CURSOR_DISABLED); glEnable(GL_DEPTH_TEST); glDisable(GL_CULL_FACE);
    GLuint programa=glCreateProgram(); GLuint vs=glCreateShader(GL_VERTEX_SHADER),fs=glCreateShader(GL_FRAGMENT_SHADER);
    std::string vert="#version 330 core\nlayout(location=0) in vec3 p; layout(location=1) in vec3 inputColor; uniform mat4 model,view,projection; out vec3 world; out vec3 vertexColor; void main(){ vec4 w=model*vec4(p,1); world=w.xyz; vertexColor=inputColor; gl_Position=projection*view*w;}";
    std::string frag="#version 330 core\nin vec3 world; in vec3 vertexColor; uniform vec3 tint; uniform int field; uniform int lamps; uniform sampler2D grassTexture; out vec4 color; void main(){ float stripe=mod(floor((world.z+52.5)/10.5),2.0); vec3 texel=texture(grassTexture,world.xz*0.045).rgb*mix(0.82,1.0,stripe); float isLine=step(0.80,min(vertexColor.r,min(vertexColor.g,vertexColor.b))); vec3 c=(field==1)?mix(texel,vertexColor,isLine):tint; float light=mix(0.62,1.0,float(lamps)); color=vec4(c*light,1.0); }";
    const char* v=vert.c_str(); const char* f=frag.c_str(); glShaderSource(vs,1,&v,nullptr); glShaderSource(fs,1,&f,nullptr); glCompileShader(vs); glCompileShader(fs);
    GLint ok=0; glGetShaderiv(vs,GL_COMPILE_STATUS,&ok); if(!ok) std::cerr<<"Vertex shader compilation failed\n";
    glGetShaderiv(fs,GL_COMPILE_STATUS,&ok); if(!ok) std::cerr<<"Fragment shader compilation failed\n";
    glAttachShader(programa,vs); glAttachShader(programa,fs); glLinkProgram(programa); glGetProgramiv(programa,GL_LINK_STATUS,&ok);
    if(!ok) std::cerr<<"Shader link failed\n"; glDeleteShader(vs); glDeleteShader(fs);
    GLuint vao,vbo,cuboVao,cuboVbo; glGenVertexArrays(1,&vao); glGenBuffers(1,&vbo); glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER,vbo); glBufferData(GL_ARRAY_BUFFER,vertices.size()*sizeof(float),vertices.data(),GL_STATIC_DRAW);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,6*sizeof(float),(void*)0); glEnableVertexAttribArray(0);
    glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,6*sizeof(float),(void*)(3*sizeof(float))); glEnableVertexAttribArray(1);
    glGenVertexArrays(1,&cuboVao); glGenBuffers(1,&cuboVbo); glBindVertexArray(cuboVao); glBindBuffer(GL_ARRAY_BUFFER,cuboVbo);
    glBufferData(GL_ARRAY_BUFFER,sizeof(cubo),cubo,GL_STATIC_DRAW); glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(float),(void*)0); glEnableVertexAttribArray(0);
    // Textura procedural de cesped repetible: muestreo real mediante sampler2D en GLSL.
    unsigned char cesped[8*8*3];
    for(int y=0;y<8;++y) for(int x=0;x<8;++x) { int k=(y*8+x)*3; bool claro=((x+y)%2)==0; cesped[k]=(unsigned char)(claro?76:54); cesped[k+1]=(unsigned char)(claro?137:111); cesped[k+2]=(unsigned char)(claro?52:43); }
    GLuint texturaCesped; glGenTextures(1,&texturaCesped); glBindTexture(GL_TEXTURE_2D,texturaCesped);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,8,8,0,GL_RGB,GL_UNSIGNED_BYTE,cesped); glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR); glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT); glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
    GLuint programaModelos=glCreateProgram(); GLuint modeloVS=glCreateShader(GL_VERTEX_SHADER),modeloFS=glCreateShader(GL_FRAGMENT_SHADER);
    std::string vertexModelo="#version 330 core\nlayout(location=0) in vec3 p; layout(location=1) in vec3 n; layout(location=2) in vec2 uv; uniform mat4 model,view,projection; out vec3 normalWorld; out vec2 texCoord; void main(){ normalWorld=mat3(transpose(inverse(model)))*n; texCoord=uv; gl_Position=projection*view*model*vec4(p,1.0); }";
    std::string fragmentModelo="#version 330 core\nin vec3 normalWorld; in vec2 texCoord; uniform sampler2D diffuseTexture; uniform vec3 materialColor; uniform vec3 lightDirection; uniform float lightStrength; uniform int hasTexture; out vec4 fragColor; void main(){ vec3 base=hasTexture==1?texture(diffuseTexture,texCoord).rgb:materialColor; float diffuse=max(dot(normalize(normalWorld),normalize(lightDirection)),0.0); float illumination=mix(0.48+0.52*diffuse,0.48,1.0-lightStrength); fragColor=vec4(base*illumination,1.0); }";
    const char* mv=vertexModelo.c_str(); const char* mf=fragmentModelo.c_str(); glShaderSource(modeloVS,1,&mv,nullptr); glShaderSource(modeloFS,1,&mf,nullptr); glCompileShader(modeloVS); glCompileShader(modeloFS);
    glAttachShader(programaModelos,modeloVS); glAttachShader(programaModelos,modeloFS); glLinkProgram(programaModelos);
    glDeleteShader(modeloVS); glDeleteShader(modeloFS);

    struct Instancia { size_t modelo; glm::vec3 posicion; float tamano; float rotacion; float inclinacion; };
    const char* rutasModelos[]={
        "Models/ProyectoGrafica/autobus/autobus.obj", "Models/ProyectoGrafica/balon/balon.obj",
        "Models/ProyectoGrafica/camera/camera_estadio.obj", "Models/ProyectoGrafica/scoreboard/scoreboard.obj",
        "Models/ProyectoGrafica/carrito_emergencias/carrito_emergencias.obj", "Models/ProyectoGrafica/estadio/estadio.obj",
        "Models/ProyectoGrafica/podio/podio.obj", "Models/ProyectoGrafica/trofeo/trofeo.obj",
        "Models/ProyectoGrafica/iluminador_estadio/iluminador_estadio.obj",
        "Models/ProyectoGrafica/jugador_arbitro/jugador_arbitro.obj", "Models/ProyectoGrafica/jugador_arbitro/jugador_arbitro.obj",
        "Models/ProyectoGrafica/porteria_texturizada/porteria_texturizada.obj", "Models/ProyectoGrafica/banca_dia_noche/banca_dia_noche.obj",
        "Models/ProyectoGrafica/banca/banca.obj", "Models/ProyectoGrafica/carrito/carrito.obj",
        "Models/ProyectoGrafica/goyo/goyo.obj", "Models/ProyectoGrafica/tiro_esquina/tiro_esquina.obj"
    };
    const char* filtrosModelo[]={nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,
        "Jugador_CHICHARITO|CHICHARITO_|","Arbitro|ARBITRO_|",nullptr,nullptr,nullptr,nullptr,nullptr,nullptr};
    std::vector<ModeloOBJ> modelos; modelos.reserve(17);
    int modelosCargados=0;
    for(int i=0;i<17;++i) { modelos.emplace_back(); if(modelos.back().cargar(rutasModelos[i],filtrosModelo[i]?filtrosModelo[i]:"")) ++modelosCargados; }
    std::cout<<"Modelos OBJ cargados: "<<modelosCargados<<"/17 (16 modelos fuente; jugador y arbitro separados)"<<std::endl;
    std::vector<Instancia> instancias;
    auto instancia=[&](size_t m,glm::vec3 p,float s,float r=0.0f,float x=0.0f) { instancias.push_back({m,p,s,r,x}); };
    // El campo importado mide aprox. 101 x 68 unidades: largo en X, ancho en Z.
    instancia(0,glm::vec3(-78,0,48),14,90);
    instancia(2,glm::vec3(0,7,-40),1.5f);
    instancia(3,glm::vec3(-62,14,0),11,90);
    instancia(4,glm::vec3(-30,0,-24),2.6f);
    instancia(5,glm::vec3(0,0,0),210);
    instancia(6,glm::vec3(10,0,-26),8);
    instancia(7,glm::vec3(10,1.0f,-26),3.5f);
    // Bases de los iluminadores sobre el anillo exterior de la cubierta.
    instancia(8,glm::vec3(-50,17,-40),12,45,28); instancia(8,glm::vec3(50,17,-40),12,-45,28);
    instancia(8,glm::vec3(-50,17,40),12,135,28); instancia(8,glm::vec3(50,17,40),12,-135,28);
    // Porterias centradas en las lineas de fondo; la boca queda transversal al largo del campo.
    instancia(11,glm::vec3(-33.4f,0,0),8,90); instancia(11,glm::vec3(33.4f,0,0),8,-90);
    // Una banca junto al lateral de cada porteria.
    instancia(12,glm::vec3(0,0.15f,-27),5.0f,90); instancia(13,glm::vec3(0,0.15f,27),5.0f,270);
    // El carrito de comida va en la grada interior, elevado sobre el borde de la cancha.
    instancia(14,glm::vec3(-0,7,48),4.0f); instancia(15,glm::vec3(-14,0,23),2.4f);
    // Cuatro banderas sobre las esquinas reales del terreno.
    instancia(16,glm::vec3(-34.0f,-0.5,-21.9f),4.5f); instancia(16,glm::vec3(-34.0f,-0.5,21.9f),4.5f);
    instancia(16,glm::vec3(34.0f,-0.5,21.9f),4.5f); instancia(16,glm::vec3(34.0f,-0.5,-21.9f),4.5f);
    GLint locM=glGetUniformLocation(programa,"model"),locV=glGetUniformLocation(programa,"view"),locP=glGetUniformLocation(programa,"projection");
    GLint locColor=glGetUniformLocation(programa,"tint"),locField=glGetUniformLocation(programa,"field"),locLamps=glGetUniformLocation(programa,"lamps");
    glUseProgram(programa); glUniform1i(glGetUniformLocation(programa,"grassTexture"),0);
    double anterior=glfwGetTime();
    while(!glfwWindowShouldClose(window)) {
        double ahora=glfwGetTime(); float dt=(float)glm::clamp(ahora-anterior,0.0,0.05); anterior=ahora; glfwPollEvents();
        float yaw=glm::radians(anguloMouseX); glm::vec3 frente=glm::normalize(glm::vec3(cos(yaw),0,sin(yaw)));
        glm::vec3 derecha=glm::normalize(glm::cross(frente,glm::vec3(0,1,0)));
        float velocidadJugador=primeraPersona?9.0f:8.0f;
        if(teclas[GLFW_KEY_W]) jugador+=frente*dt*velocidadJugador;
        if(teclas[GLFW_KEY_S]) jugador-=frente*dt*velocidadJugador;
        if(teclas[GLFW_KEY_A]) jugador-=derecha*dt*velocidadJugador;
        if(teclas[GLFW_KEY_D]) jugador+=derecha*dt*velocidadJugador;
        jugador.x=glm::clamp(jugador.x,-48.0f,48.0f); jugador.z=glm::clamp(jugador.z,-31.0f,31.0f);
        if(animando) { tiempoAnimacion+=dt; if(tiempoAnimacion>=2.2f) { animando=false; ++golesLocal; } }
        glm::vec3 posCam, objetivo;
        if(primeraPersona) { posCam=jugador+glm::vec3(0,1.7f,0); float pitch=glm::radians(anguloMouseY); glm::vec3 f=glm::normalize(glm::vec3(cos(yaw)*cos(pitch),sin(pitch),sin(yaw)*cos(pitch))); objetivo=posCam+f; }
        else {
            objetivo=jugador+glm::vec3(0,2.0f,0);
            float pitch=glm::radians(glm::clamp(anguloMouseY,10.0f,80.0f));
            posCam=objetivo-frente*(zoomCamara*cos(pitch))+glm::vec3(0,zoomCamara*sin(pitch),0);
        }
        glm::mat4 vista=glm::lookAt(posCam,objetivo,glm::vec3(0,1,0)); int ww,hh; glfwGetFramebufferSize(window,&ww,&hh); if(hh==0) hh=1;
        glm::mat4 proy=glm::perspective(glm::radians(campoVision),(float)ww/hh,0.1f,520.0f);
        glViewport(0,0,ww,hh); glClearColor(0.015f,0.035f,0.08f,1); glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT); glUseProgram(programa);
        glUniformMatrix4fv(locV,1,GL_FALSE,glm::value_ptr(vista)); glUniformMatrix4fv(locP,1,GL_FALSE,glm::value_ptr(proy)); glUniform1i(locLamps,reflectores?1:0);
        glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D,texturaCesped);
        glUniformMatrix4fv(locM,1,GL_FALSE,glm::value_ptr(glm::mat4(1))); glUniform3f(locColor,1,1,1); glUniform1i(locField,1); glBindVertexArray(vao); glDrawArrays(GL_TRIANGLES,0,(GLsizei)(vertices.size()/6));
        glUniform1i(locField,0); glBindVertexArray(cuboVao);
        for(const Caja& c:cajas) { glm::mat4 m=transformacion(c.centro,c.escala); glUniformMatrix4fv(locM,1,GL_FALSE,glm::value_ptr(m)); glUniform3fv(locColor,1,glm::value_ptr(c.color)); glDrawArrays(GL_TRIANGLES,0,36); }
        glUseProgram(programaModelos);
        for(const Instancia& item:instancias) modelos[item.modelo].dibujar(programaModelos,vista,proy,item.posicion,item.tamano,item.rotacion,reflectores,item.inclinacion);
        glUseProgram(programa);
        glUniformMatrix4fv(locV,1,GL_FALSE,glm::value_ptr(vista)); glUniformMatrix4fv(locP,1,GL_FALSE,glm::value_ptr(proy)); glUniform1i(locLamps,reflectores?1:0);
        glBindVertexArray(cuboVao);
        auto dibujaParte=[&](glm::vec3 pos,glm::vec3 escala,glm::vec3 color,glm::mat4 base) { glm::mat4 m=base; m=glm::translate(m,pos); m=glm::scale(m,escala); glUniformMatrix4fv(locM,1,GL_FALSE,glm::value_ptr(m)); glUniform3fv(locColor,1,glm::value_ptr(color)); glDrawArrays(GL_TRIANGLES,0,36); };
        glUseProgram(programaModelos);
        modelos[9].dibujar(programaModelos,vista,proy,jugador,3.2f,0.0f,reflectores);
        modelos[10].dibujar(programaModelos,vista,proy,arbitro,3.2f,180.0f,reflectores);
        glUseProgram(programa);
        // Marcador de siete segmentos: G suma local, H suma visita, R reinicia.
        const int segmentos[10][7]={{1,1,1,1,1,1,0},{0,1,1,0,0,0,0},{1,1,0,1,1,0,1},{1,1,1,1,0,0,1},{0,1,1,0,0,1,1},{1,0,1,1,0,1,1},{1,0,1,1,1,1,1},{1,1,1,0,0,0,0},{1,1,1,1,1,1,1},{1,1,1,1,0,1,1}};
        auto digito=[&](int valor,float z) {
            const float xx[7]={0.0f,0.62f,0.62f,0.0f,-0.62f,-0.62f,0.0f};
            const float yy[7]={0.72f,0.36f,-0.36f,-0.72f,-0.36f,0.36f,0.0f};
            for(int i=0;i<7;++i) if(segmentos[valor%10][i]) {
                bool horizontal=(i==0||i==3||i==6);
                dibujaParte(glm::vec3(-61.65f,20.1f+yy[i],z+xx[i]),horizontal?glm::vec3(0.10f,0.12f,0.88f):glm::vec3(0.10f,0.62f,0.12f),glm::vec3(0.2f,0.95f,0.95f),glm::mat4(1));
            }
        };
        digito(golesLocal,-2.6f); digito(golesVisita,2.6f);
        dibujaParte(glm::vec3(-61.65f,20.1f,0),glm::vec3(0.10f,0.16f,0.12f),AMARILLO,glm::mat4(1));
        float bx=keyframesBalon[0].x, by=keyframesBalon[0].y, bz=keyframesBalon[0].z;
        if(animando) for(int k=0;k<5;++k) if(tiempoAnimacion>=keyframesBalon[k].tiempo&&tiempoAnimacion<=keyframesBalon[k+1].tiempo) {
            float u=(tiempoAnimacion-keyframesBalon[k].tiempo)/(keyframesBalon[k+1].tiempo-keyframesBalon[k].tiempo);
            bx=glm::mix(keyframesBalon[k].x,keyframesBalon[k+1].x,u); by=glm::mix(keyframesBalon[k].y,keyframesBalon[k+1].y,u); bz=glm::mix(keyframesBalon[k].z,keyframesBalon[k+1].z,u); break;
        }
        glm::vec3 balon=jugador+glm::vec3(bx,by,bz);
        if(modelos[1].listo()) { glUseProgram(programaModelos); modelos[1].dibujar(programaModelos,vista,proy,balon,1.5f,tiempoAnimacion*180.0f,reflectores); glUseProgram(programa); glBindVertexArray(cuboVao); }
        else dibujaParte(balon,glm::vec3(0.65f),glm::vec3(0.94f,0.94f,0.88f),glm::mat4(1));
        glfwSwapBuffers(window);
    }
    for(size_t i=0;i<modelos.size();++i) modelos[i].liberar();
    glDeleteTextures(1,&texturaCesped); glDeleteVertexArrays(1,&vao); glDeleteBuffers(1,&vbo); glDeleteVertexArrays(1,&cuboVao); glDeleteBuffers(1,&cuboVbo); glDeleteProgram(programa); glDeleteProgram(programaModelos);
    glfwDestroyWindow(window); glfwTerminate(); return 0;
}

