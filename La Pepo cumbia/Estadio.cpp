#include <iostream>
#include <vector>
#include <cmath>
#include <string>
#include <utility>
#include "ModeloOBJ.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

/*
    Proyecto final: escenario deportivo 3D interactivo - futbol soccer.

    En este archivo integro la escena del estadio, la carga de modelos, los
    controles y la animacion de una jugada. La aplicacion solicita OpenGL 3.3
    Core y usa GLFW para la ventana/entrada, GLEW para las funciones OpenGL,
    GLM para las transformaciones y SOIL para cargar algunas texturas.

    Flujo general:
      1. Preparar la ventana, shaders, geometria y recursos.
      2. Cargar los modelos OBJ y ubicarlos en la escena.
      3. En cada iteracion, leer controles y actualizar las animaciones.
      4. Dibujar el estadio, cancha, personajes, marcador y efectos.

    Las rutas de modelos y texturas son relativas al directorio de trabajo del
    proyecto. Por eso conservo la carpeta Models y la carpeta Tifos al ejecutar.
*/

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

// GLFW llama esta funcion cuando cambia el tamano del framebuffer. Actualizar
// el viewport hace que OpenGL dibuje sobre toda el area disponible.
void cambiarTamano(GLFWwindow* window, int ancho, int alto)
{
    glViewport(0, 0, ancho, alto);
}

// =====================================================
// AGREGAR UN VERTICE
// =====================================================

// Guarda un vertice con el formato que espera el VAO: posicion XYZ y color RGB.
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

// Divide una superficie rectangular en dos triangulos, que es la primitiva
// basica usada por OpenGL para representar superficies.
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

// Representa cada linea como un rectangulo delgado sobre el plano XZ.
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

// Variante horizontal de la linea: conserva el mismo grosor y cambia el eje.
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

// Forma un anillo aproximado con triangulos entre dos radios; se usa para
// dibujar la circunferencia central y otras marcas redondas del campo.
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

// Crea un disco relleno triangulando desde el centro hasta el borde.
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

// Construye la geometria de la cancha en el vector global `vertices`.
// Las medidas se expresan en unidades del mundo antes de orientar el campo.
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

// Representacion sencilla de un objeto estatico con forma de caja.
struct Caja { glm::vec3 centro, escala, color; };
// Geometria que se envia a OpenGL y objetos auxiliares del escenario.
std::vector<float> geometriaEscenario;
std::vector<Caja> cajas;
std::vector<Caja> vallasPublicidad;
std::vector<Caja> fondosTifo;
std::vector<float> geometriaConos;

// Estados que se comparten entre los callbacks de entrada y el ciclo principal.
// Los callbacks modifican estos valores y el ciclo los consulta para actualizar
// movimiento, animacion, camaras e interaccion con el marcador.
bool teclas[1024] = {};
bool primeraPersona = false, camaraLibre = false, reflectores = true, animando = false;
int golesLocal = 0, golesVisita = 0;
float anguloMouseX = -90.0f, anguloMouseY = 43.0f;
float zoomCamara = 100.0f;
float campoVision = 55.0f;
double mouseXAnterior = ANCHO_VENTANA / 2.0, mouseYAnterior = ALTO_VENTANA / 2.0;
bool primerMovimientoMouse = true;
glm::vec3 posicionCamaraLibre(0.0f, 48.0f, 112.0f);
glm::vec3 jugador(-3.0f, 0.0f, 0.0f);
glm::vec3 inicioJugada(-3.0f, 0.0f, 0.0f), objetivoJugada(-33.4f, 0.0f, 0.0f);
glm::vec3 direccionTiro(-1.0f, 0.0f, 0.0f), lateralJugada(0.0f, 0.0f, -1.0f);
float distanciaJugada = 30.4f;
bool jugadaTerminada = false;
bool marcadorJugadaActualizado = false;
// Estado del balon cuando el usuario juega directamente, fuera de la jugada automatizada.
bool balonLibre = false, balonEnMovimiento = false;
glm::vec3 posicionBalonManual(0.0f, 0.22f, 0.0f), velocidadBalonManual(0.0f);
float ajusteTiro = 0.0f;
float tiempoReaccionPortero = 0.0f;
int ladoPorteroReacciona = 0;
// El autobus empieza en el acceso exterior y se conduce con las flechas.
glm::vec3 posicionAutobus(-78.0f, 0.0f, 48.0f);
float rumboAutobus = 90.0f;
bool camaraAutobus = false;
glm::vec3 arbitro(7.0f, 0.0f, -4.0f);
float tiempoAnimacion = 0.0f;

// Keyframes del balon: tiempo, avance entre origen y porteria, altura y desvio lateral.
// Cada keyframe del balon especifica un instante y su posicion relativa al tiro:
// avance longitudinal, altura vertical y desplazamiento lateral.
struct KeyframeBalon { float tiempo, avance, altura, lateral; };
// Progreso relativo al recorrido a porteria: pase 1, pase 2 y remate elevado.
const KeyframeBalon keyframesBalon[] = {
    {0.00f,0.03f,0.22f,0.0f}, {0.28f,0.03f,0.22f,0.0f},
    {1.20f,0.20f,0.30f,8.0f}, {1.55f,0.20f,0.30f,8.0f},
    {2.60f,0.48f,0.30f,-4.5f}, {2.80f,0.48f,0.22f,-4.5f},
    {4.65f,0.86f,0.22f,0.0f}, {4.78f,0.90f,0.55f,-0.4f},
    {5.00f,0.96f,1.80f,-1.2f}, {5.22f,1.00f,1.95f,-1.8f},
    {5.42f,1.02f,0.75f,-1.8f}, {5.62f,1.015f,0.38f,-1.6f},
    {6.50f,1.015f,0.22f,-1.6f}
};
// Poses de la jugada para articular caderas, rodillas, tobillos y brazos.
// En cada instante se guardan angulos articulares para ambas piernas y brazos.
struct KeyframeJugador { float tiempo, caderaDer, rodillaDer, tobilloDer, caderaIzq, rodillaIzq, tobilloIzq, brazos; };
const KeyframeJugador keyframesJugador[] = {
    {0.00f,  0.0f,  0.0f, 0.0f,  0.0f, 0.0f, 0.0f,   0.0f},
    {0.16f, 18.0f,  8.0f,-2.0f, -6.0f, 1.0f, 0.0f, -10.0f},
    {0.30f,-34.0f,-16.0f, 5.0f, 10.0f, 2.0f, 0.0f,  17.0f},
    {0.46f,-52.0f,-10.0f, 3.0f,  8.0f, 1.0f, 0.0f,  21.0f},
    {0.66f,-28.0f, -3.0f, 1.0f,  2.0f, 0.0f, 0.0f,   8.0f},
    {0.92f, -5.0f,  0.0f, 0.0f,  0.0f, 0.0f, 0.0f,   1.0f},
    {1.25f,  0.0f,  0.0f, 0.0f,  0.0f, 0.0f, 0.0f,   0.0f}
};
// Una pose del ciclo de carrera contiene fase, paso de piernas, flexion de rodillas y brazos.
struct KeyframeCarrera { float fase, paso, rodillaDer, rodillaIzq, brazos; };
// Un ciclo de carrera en cuatro poses. La interpolacion ciclica Catmull-Rom
// conserva velocidad entre poses y evita el vaiven sinusoidal mecanico.
const KeyframeCarrera keyframesCarrera[] = {
    {0.00f, -18.0f, 20.0f,  0.0f,  11.0f},
    {0.25f,   0.0f,  5.0f,  8.0f,   0.0f},
    {0.50f,  18.0f,  0.0f, 22.0f, -11.0f},
    {0.75f,   0.0f, 12.0f,  0.0f,   0.0f}
};
const glm::vec3 AMARILLO(0.95f, 0.72f, 0.12f), AZUL(0.04f, 0.25f, 0.82f);
enum ModeloArticulado {
    CUERPO_JUGADOR = 17, BRAZO_SUP_DER, BRAZO_INF_DER, BRAZO_SUP_IZQ, BRAZO_INF_IZQ,
    MUSLO_DER, PANTORRILLA_DER, PIE_DER, MUSLO_IZQ, PANTORRILLA_IZQ, PIE_IZQ
};

// Agrega una caja a la lista que se dibuja despues usando el cubo unitario.
void agregarCaja(float x, float y, float z, float sx, float sy, float sz, glm::vec3 c) {
    cajas.push_back({ glm::vec3(x,y,z), glm::vec3(sx,sy,sz), c });
}

// Estas funciones convierten cada cono en triangulos con color por vertice.
// Guarda posicion y color de un vertice para la malla de conos.
void agregarVerticeCono(glm::vec3 p, glm::vec3 color) {
    geometriaConos.insert(geometriaConos.end(), { p.x, p.y, p.z, color.r, color.g, color.b });
}

// Agrega los tres vertices que forman una cara triangular del cono.
void agregarTrianguloCono(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 color) {
    agregarVerticeCono(a, color); agregarVerticeCono(b, color); agregarVerticeCono(c, color);
}

// Construye un cono de entrenamiento con base, cuerpo, franja blanca y punta.
void agregarConoEntrenamiento(float x, float z) {
    const int segmentos = 12;
    const float yBase = 0.075f, yHombro = 0.29f, yBanda = 0.37f, yPunta = 0.60f;
    const float radioBase = 0.23f, radioHombro = 0.15f, radioBanda = 0.115f;
    const glm::vec3 naranja(1.0f, 0.24f, 0.025f), blanco(0.96f, 0.96f, 0.88f);
    glm::vec3 punta(x, yPunta, z), centroBase(x, yBase, z);
    for (int i = 0; i < segmentos; ++i) {
        float a0 = 2.0f * PI * i / segmentos, a1 = 2.0f * PI * (i + 1) / segmentos;
        auto punto = [&](float angulo, float y, float radio) { return glm::vec3(x + cos(angulo) * radio, y, z + sin(angulo) * radio); };
        glm::vec3 b0 = punto(a0, yBase, radioBase), b1 = punto(a1, yBase, radioBase);
        glm::vec3 h0 = punto(a0, yHombro, radioHombro), h1 = punto(a1, yHombro, radioHombro);
        glm::vec3 w0 = punto(a0, yBanda, radioBanda), w1 = punto(a1, yBanda, radioBanda);
        agregarTrianguloCono(centroBase, b1, b0, naranja);
        agregarTrianguloCono(b0, b1, h1, naranja); agregarTrianguloCono(b0, h1, h0, naranja);
        agregarTrianguloCono(h0, h1, w1, blanco); agregarTrianguloCono(h0, w1, w0, blanco);
        agregarTrianguloCono(w0, w1, punta, naranja);
    }
}

// En esta funcion ubico la publicidad, los tifos y los conos procurando
// mantener libres las porterias y la franja central frente a las bancas.
// Genera elementos de utileria de forma procedimental para evitar archivos
// adicionales: paneles, fondos para tifos y conos en las zonas tecnicas.
void construirVallasYConos() {
    vallasPublicidad.clear(); fondosTifo.clear(); geometriaConos.clear();
    const glm::vec3 marco(0.035f, 0.045f, 0.065f), blanco(0.92f, 0.94f, 0.90f);
    const glm::vec3 marcas[] = {
        glm::vec3(0.05f,0.62f,0.88f), glm::vec3(0.88f,0.12f,0.08f),
        glm::vec3(0.95f,0.58f,0.04f), glm::vec3(0.08f,0.56f,0.24f),
        glm::vec3(0.48f,0.12f,0.72f), glm::vec3(0.02f,0.64f,0.66f)
    };
    auto agregarPiezaValla = [&](glm::vec3 centro, glm::vec3 escala, glm::vec3 color) {
        vallasPublicidad.push_back({ centro, escala, color });
        };
    auto agregarAnuncio = [&](float x, float z, bool orientadoX, int indice) {
        const float largo = orientadoX ? 5.1f : 4.6f;
        const float alto = 1.42f, y = 0.98f;
        glm::vec3 normal = orientadoX ? glm::vec3(0, 0, z > 0.0f ? -1.0f : 1.0f)
            : glm::vec3(x > 0.0f ? -1.0f : 1.0f, 0, 0);
        glm::vec3 centro(x, y, z), escalaMarco = orientadoX ? glm::vec3(largo, alto, 0.38f) : glm::vec3(0.38f, alto, largo);
        agregarPiezaValla(centro, escalaMarco, marco);
        glm::vec3 cara = centro + normal * 0.205f;
        glm::vec3 escalaCara = orientadoX ? glm::vec3(largo - 0.18f, alto - 0.18f, 0.055f) : glm::vec3(0.055f, alto - 0.18f, largo - 0.18f);
        agregarPiezaValla(cara, escalaCara, marcas[indice % 6]);
        glm::vec3 escalaFranja = orientadoX ? glm::vec3(largo - 0.18f, 0.12f, 0.065f) : glm::vec3(0.065f, 0.12f, largo - 0.18f);
        glm::vec3 franjaSuperior = cara + glm::vec3(0, 0.56f, 0) + normal * 0.035f;
        glm::vec3 franjaInferior = cara + glm::vec3(0, -0.56f, 0) + normal * 0.035f;
        agregarPiezaValla(franjaSuperior, escalaFranja, blanco);
        agregarPiezaValla(franjaInferior, escalaFranja, (indice % 2 == 0) ? marcas[(indice + 1) % 6] : blanco);
        glm::vec3 escalaLogo = orientadoX ? glm::vec3(0.52f, 0.48f, 0.07f) : glm::vec3(0.07f, 0.48f, 0.52f);
        glm::vec3 logo = cara + normal * 0.07f;
        agregarPiezaValla(logo, escalaLogo, blanco);
        glm::vec3 escalaTexto = orientadoX ? glm::vec3(0.82f, 0.075f, 0.075f) : glm::vec3(0.075f, 0.075f, 0.82f);
        for (int lado = -1; lado <= 1; lado += 2) {
            glm::vec3 linea = logo + normal * 0.015f;
            if (orientadoX) linea.x += lado * 1.25f; else linea.z += lado * 1.25f;
            agregarPiezaValla(linea, escalaTexto, blanco);
        }
        };

    // Laterales con paneles separados: el tramo central queda libre para bancas y entrenamiento.
    const float centrosLaterales[] = { -31.0f,-25.0f,-19.0f,19.0f,25.0f,31.0f };
    int indice = 0;
    for (float z : { -23.7f, 23.7f }) for (float x : centrosLaterales) agregarAnuncio(x, z, true, indice++);
    // Cabeceras: se deja el hueco central para cada porteria y su red.
    const float centrosCabecera[] = { -18.0f,-12.0f,-6.0f,6.0f,12.0f,18.0f };
    // Se alejan las cabeceras para dejar espacio a la porteria y su red.
    for (float x : { -43.5f, 43.5f }) for (float z : centrosCabecera) agregarAnuncio(x, z, false, indice++);

    // Tifos elevados en las cabeceras, hacia el interior del estadio.
    // Cada escudo se dibuja como textura sobre un paño de los colores del equipo.
    fondosTifo.push_back({ glm::vec3(-38.5f, 14.5f, 0.0f), glm::vec3(0.45f, 10.0f, 16.0f), glm::vec3(0.035f, 0.055f, 0.24f) });
    fondosTifo.push_back({ glm::vec3(38.5f, 14.5f, 0.0f), glm::vec3(0.45f, 10.0f, 16.0f), glm::vec3(0.80f, 0.58f, 0.12f) });

    // Conos en las dos zonas tecnicas, delante de las bancas y fuera de la cancha.
    const float posicionesX[] = { -12.0f,-8.0f,-4.0f,4.0f,8.0f,12.0f };
    for (int lado : { -1, 1 }) for (int i = 0; i < 6; ++i) {
        float z = lado * (23.0f + (i % 2) * 1.35f);
        agregarConoEntrenamiento(posicionesX[i], z);
    }
}

// Inserta un triangulo en la geometria comun del escenario.
void agregarTriangulo(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 color) {
    agregarVertice(a.x, a.y, a.z, color); agregarVertice(b.x, b.y, b.z, color); agregarVertice(c.x, c.y, c.z, color);
}

// Aproxima un cilindro vertical mediante caras triangulares.
void agregarCilindroVertical(float x, float z, float radio, float alto, glm::vec3 color) {
    const int n = 12;
    for (int i = 0;i < n;++i) {
        float a = 2 * PI * i / n, b = 2 * PI * (i + 1) / n;
        glm::vec3 p1(x + cos(a) * radio, 0, z + sin(a) * radio), p2(x + cos(b) * radio, 0, z + sin(b) * radio);
        glm::vec3 p3 = p2 + glm::vec3(0, alto, 0), p4 = p1 + glm::vec3(0, alto, 0);
        agregarTriangulo(p1, p2, p3, color); agregarTriangulo(p1, p3, p4, color);
        agregarTriangulo(glm::vec3(x, alto, z), p4, p3, color);
    }
}

// Organiza el campo, gradas, bancas, cabinas, torres y accesos en el espacio 3D.
void construirEscenario() {
    construirCancha();
    // El estadio y los jugadores usan X como eje largo del campo; la cancha
    // se definio localmente con el largo en Z, asi que la giramos al guardar.
    // El pequeño alzado evita que el piso verde del modelo importado la tape.
    for (size_t i = 0; i + 5 < vertices.size(); i += 6) {
        std::swap(vertices[i], vertices[i + 2]);
        vertices[i + 1] += 0.06f;
    }
    glm::vec3 gris(0.28f, 0.32f, 0.38f), concreto(0.42f, 0.43f, 0.40f), blanco(0.96f);
    // Zona 2: gradas en ambos laterales, con pasillos y asientos en niveles.
    for (int lado = -1;lado <= 1;lado += 2) {
        for (int fila = 0;fila < 9;++fila) {
            float x = lado * (39.0f + fila * 1.65f), y = 1.0f + fila * 0.78f;
            agregarCaja(x, y, -2, 3.2f, 0.65f, 96, gris);
            for (int z = -44;z <= 44;z += 4) {
                glm::vec3 asiento = ((z / 4 + fila) % 2 == 0) ? AZUL : AMARILLO;
                agregarCaja(x - lado * 0.2f, y + 0.42f, (float)z, 2.1f, 0.45f, 2.1f, asiento);
            }
        }
        // Torre de iluminacion y luminarias encendidas/apagadas por tecla F.
        agregarCaja(lado * 58.0f, 17.0f, 0, 0.8f, 34, 0.8f, concreto);
        for (int j = 0;j < 5;++j) agregarCaja(lado * 58.0f, 34.0f, (-2.0f + j) * 1.3f, 1.5f, 0.8f, 1.0f, reflectores ? glm::vec3(1.0f, 0.92f, 0.55f) : gris);
    }
    // Zona 3: bancas/calentamiento y cabinas en las cabeceras.
    for (int lado = -1;lado <= 1;lado += 2) {
        for (int i = 0;i < 8;++i) agregarCaja(-12.0f + i * 3.4f, 0.6f, lado * 59.0f, 2.8f, 1.2f, 1.0f, AZUL);
        agregarCaja(0, 14.0f, lado * 58.0f, 30.0f, 5.0f, 3.0f, gris);
        agregarCaja(0, 17.0f, lado * 58.0f, 28.0f, 0.5f, 3.4f, AMARILLO);
    }
    // Perimetro, accesos, tuneles y torres de soporte.
    agregarCaja(0, 0.0f, -69, 138, 0.3f, 0.5f, concreto); agregarCaja(0, 0.0f, 69, 138, 0.3f, 0.5f, concreto);
    agregarCaja(-69, 0, 0, 0.5f, 0.3f, 138, concreto); agregarCaja(69, 0, 0, 0.5f, 0.3f, 138, concreto);
    for (int x = -2;x <= 2;++x) agregarCaja(x * 2.2f, 2.1f, 57, 1.8f, 4.2f, 3.0f, AZUL);
}

// El callback de mouse actualiza la direccion de observacion de la camara.
// GLFW llama estos callbacks como respuesta a la entrada del usuario.
void cursor(GLFWwindow*, double x, double y) {
    // El primer evento solo fija la referencia y evita un salto brusco de camara.
    if (primerMovimientoMouse) { mouseXAnterior = x; mouseYAnterior = y; primerMovimientoMouse = false; }
    // Convierto el desplazamiento del mouse en cambios de yaw y pitch.
    float dx = (float)(x - mouseXAnterior), dy = (float)(mouseYAnterior - y); mouseXAnterior = x; mouseYAnterior = y;
    anguloMouseX += dx * 0.10f;
    anguloMouseY = (primeraPersona || camaraLibre) ? glm::clamp(anguloMouseY + dy * 0.10f, -85.0f, 85.0f) : glm::clamp(anguloMouseY + dy * 0.10f, 10.0f, 80.0f);
}

// La rueda acerca/aleja las camaras externas y ajusta el campo visual en primera persona.
void rueda(GLFWwindow*, double, double desplazamiento) {
    // Ajusto el modo activo: traslacion libre, campo visual o zoom orbital.
    if (camaraLibre) {
        float yaw = glm::radians(anguloMouseX), pitch = glm::radians(anguloMouseY);
        glm::vec3 direccion = glm::normalize(glm::vec3(cos(yaw) * cos(pitch), sin(pitch), sin(yaw) * cos(pitch)));
        posicionCamaraLibre += direccion * (float)desplazamiento * 8.0f;
    }
    else if (primeraPersona) campoVision = glm::clamp(campoVision - (float)desplazamiento * 2.5f, 30.0f, 75.0f);
    else zoomCamara = glm::clamp(zoomCamara - (float)desplazamiento * 20.0f, 12.0f, 320.0f);
}

// Controles principales: WASD mueve al jugador/camara libre; J pasa el balon,
// K tira a gol, I/O ajusta la direccion lateral del tiro y espacio inicia la
// jugada animada. Las demas teclas conservan sus funciones anteriores.
// Centralizo aqui las acciones de teclado para mantener los controles en un solo lugar.
void tecla(GLFWwindow* w, int key, int, int action, int) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) glfwSetWindowShouldClose(w, GL_TRUE);
    // Mantengo el estado presionado de cada tecla para leerlo continuamente en el ciclo principal.
    if (key >= 0 && key < 1024) { if (action == GLFW_PRESS) teclas[key] = true; if (action == GLFW_RELEASE) teclas[key] = false; }
    if (action == GLFW_PRESS) {
        // Al cambiar de modo, apago los otros seguimientos para que solo haya una camara activa.
        if (key == GLFW_KEY_V) { primeraPersona = !primeraPersona; camaraLibre = false; camaraAutobus = false; }
        if (key == GLFW_KEY_C) {
            camaraLibre = !camaraLibre; primeraPersona = false; camaraAutobus = false;
            if (camaraLibre) { posicionCamaraLibre = glm::vec3(0.0f, 48.0f, 112.0f); anguloMouseX = -90.0f; anguloMouseY = -22.0f; }
        }
        if (key == GLFW_KEY_T) { camaraAutobus = !camaraAutobus; primeraPersona = false; camaraLibre = false; }
        if (key == GLFW_KEY_B) { posicionAutobus = glm::vec3(-78.0f, 0.0f, 48.0f); rumboAutobus = 90.0f; }
        if (key == GLFW_KEY_F) reflectores = !reflectores;
        if (key == GLFW_KEY_G) { golesLocal = (golesLocal + 1) % 100; std::cout << "Marcador local: " << golesLocal << " - visitante: " << golesVisita << std::endl; }
        if (key == GLFW_KEY_H) { golesVisita = (golesVisita + 1) % 100; std::cout << "Marcador local: " << golesLocal << " - visitante: " << golesVisita << std::endl; }
        if (key == GLFW_KEY_R) { golesLocal = golesVisita = 0; animando = false; jugadaTerminada = false; marcadorJugadaActualizado = false; tiempoAnimacion = 0.0f; }
        // N devuelve el balon al jugador y deja lista otra jugada sin borrar el marcador.
        if (key == GLFW_KEY_N) {
            animando = false; jugadaTerminada = false; marcadorJugadaActualizado = false;
            balonLibre = false; balonEnMovimiento = false;
            velocidadBalonManual = glm::vec3(0.0f); tiempoAnimacion = 0.0f;
            tiempoReaccionPortero = 0.0f; ajusteTiro = 0.0f;
            std::cout << "Balon listo junto al jugador. Marcador conservado." << std::endl;
        }
        // I y O desplazan la punteria hacia los lados dentro del ancho de la porteria.
        if (key == GLFW_KEY_I) ajusteTiro = glm::clamp(ajusteTiro - 0.8f, -3.0f, 3.0f);
        if (key == GLFW_KEY_O) ajusteTiro = glm::clamp(ajusteTiro + 0.8f, -3.0f, 3.0f);
        // No permito patear de nuevo mientras el balon sigue en juego lejos del usuario.
        bool puedeTocarBalon = !balonLibre || (!balonEnMovimiento &&
            glm::distance(glm::vec2(jugador.x, jugador.z), glm::vec2(posicionBalonManual.x, posicionBalonManual.z)) < 1.5f);
        if ((key == GLFW_KEY_J || key == GLFW_KEY_K) && !animando && !puedeTocarBalon)
            std::cout << "Acercate al balon para volver a jugarlo." << std::endl;
        // J realiza un pase raso en la direccion hacia la que mira la camara/jugador.
        if (key == GLFW_KEY_J && !animando && puedeTocarBalon) {
            float yaw = glm::radians(anguloMouseX);
            glm::vec3 direccion = glm::normalize(glm::vec3(cos(yaw), 0.0f, sin(yaw)));
            posicionBalonManual = jugador + direccion * 0.9f + glm::vec3(0.0f, 0.22f, 0.0f);
            velocidadBalonManual = direccion * 17.0f + glm::vec3(0.0f, 2.3f, 0.0f);
            balonLibre = true; balonEnMovimiento = true;
            jugadaTerminada = false;
            std::cout << "Pase en juego: los jugadores van tras el balon." << std::endl;
        }
        // K dispara hacia una porteria; I/O permite apuntar a distintas zonas del arco.
        if (key == GLFW_KEY_K && !animando && puedeTocarBalon) {
            // Mantengo el mismo sentido de ataque que la jugada por keyframes:
            // desde la mitad izquierda se ataca la porteria de X negativo.
            float signoArco = jugador.x <= 0.0f ? -1.0f : 1.0f;
            glm::vec3 objetivo(signoArco * 33.4f, 0.0f, ajusteTiro);
            glm::vec3 direccion = glm::normalize(objetivo - jugador);
            direccion.y = 0.0f; direccion = glm::normalize(direccion);
            posicionBalonManual = jugador + direccion * 0.9f + glm::vec3(0.0f, 0.22f, 0.0f);
            velocidadBalonManual = direccion * 28.0f + glm::vec3(0.0f, 6.7f, 0.0f);
            balonLibre = true; balonEnMovimiento = true;
            jugadaTerminada = false;
            std::cout << "Tiro hacia la porteria rival." << std::endl;
        }
        // Espacio guarda el origen, calcula la porteria destino y reinicia el reloj de animacion.
        if (key == GLFW_KEY_SPACE) {
            balonLibre = false; balonEnMovimiento = false; ajusteTiro = 0.0f; tiempoReaccionPortero = 0.0f;
            inicioJugada = jugador;
            objetivoJugada = glm::vec3(jugador.x <= 0.0f ? -33.4f : 33.4f, 0.0f, 0.0f);
            glm::vec3 haciaGol = objetivoJugada - inicioJugada; haciaGol.y = 0.0f;
            distanciaJugada = glm::length(haciaGol);
            if (distanciaJugada < 18.0f) { objetivoJugada.x = -objetivoJugada.x;haciaGol = objetivoJugada - inicioJugada;distanciaJugada = glm::length(haciaGol); }
            direccionTiro = glm::normalize(haciaGol);
            lateralJugada = glm::normalize(glm::cross(direccionTiro, glm::vec3(0, 1, 0)));
            // Al iniciar, orientar la vista hacia la porteria de la jugada.
            anguloMouseX = glm::degrees(std::atan2(direccionTiro.z, direccionTiro.x));
            tiempoAnimacion = 0.0f; animando = true; jugadaTerminada = false; marcadorJugadaActualizado = false; camaraAutobus = false; camaraLibre = false; primeraPersona = false;
        }
        // RePag/mas acerca la camara o reduce el campo visual segun el modo actual.
        if (key == GLFW_KEY_PAGE_UP || key == GLFW_KEY_KP_ADD || key == GLFW_KEY_EQUAL) {
            if (camaraLibre) { float yaw = glm::radians(anguloMouseX), pitch = glm::radians(anguloMouseY); posicionCamaraLibre += glm::normalize(glm::vec3(cos(yaw) * cos(pitch), sin(pitch), sin(yaw) * cos(pitch))) * 8.0f; }
            else if (primeraPersona) campoVision = glm::max(30.0f, campoVision - 4.0f); else zoomCamara = glm::max(12.0f, zoomCamara - 20.0f);
        }
        // AvPag/menos aplica el efecto contrario y tambien mantiene limites razonables.
        if (key == GLFW_KEY_PAGE_DOWN || key == GLFW_KEY_KP_SUBTRACT || key == GLFW_KEY_MINUS) {
            if (camaraLibre) { float yaw = glm::radians(anguloMouseX), pitch = glm::radians(anguloMouseY); posicionCamaraLibre -= glm::normalize(glm::vec3(cos(yaw) * cos(pitch), sin(pitch), sin(yaw) * cos(pitch))) * 8.0f; }
            else if (primeraPersona) campoVision = glm::min(75.0f, campoVision + 4.0f); else zoomCamara = glm::min(320.0f, zoomCamara + 20.0f);
        }
        if (key == GLFW_KEY_HOME) { zoomCamara = 100.0f; campoVision = 55.0f; posicionCamaraLibre = glm::vec3(0.0f, 48.0f, 112.0f); anguloMouseX = -90.0f; anguloMouseY = -22.0f; }
    }
}

glm::mat4 transformacion(glm::vec3 p, glm::vec3 s) {
    // Matriz de modelo basica para trasladar y escalar una caja unitaria.
    glm::mat4 m(1.0f); m = glm::translate(m, p); return glm::scale(m, s);
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

// Punto de entrada. Aqui inicializo los recursos y mantengo el ciclo de la aplicacion.
int main() {
    // 1. Creo la ventana y solicito un contexto OpenGL 3.3 Core.
    if (!glfwInit()) return -1; // GLFW administra la ventana y los eventos de entrada.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(ANCHO_VENTANA, ALTO_VENTANA, "La Pepo Cumbia | Estadio interactivo", nullptr, nullptr);
    if (!window) { glfwTerminate(); return -1; } // Libero GLFW si no se pudo abrir la ventana.
    glfwMakeContextCurrent(window); glfwSwapInterval(1); glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) { glfwDestroyWindow(window); glfwTerminate(); return -1; } // GLEW habilita las funciones de OpenGL usadas en el proyecto.
    std::cout << "Controles: WASD jugador/camara libre | J pase | K tiro | I/O apuntar | espacio jugada por keyframes | N devolver balon sin borrar marcador | mouse mirar/orbitar | C camara libre (E/Q altura) | V primera/tercera persona | rueda o RePág/AvPág zoom | T camara autobus | flechas conducir autobus | G/H marcador | R reiniciar marcador | F reflectores | B autobus | ESC salir" << std::endl;
    glfwSetFramebufferSizeCallback(window, cambiarTamano); glfwSetKeyCallback(window, tecla); glfwSetCursorPosCallback(window, cursor); glfwSetScrollCallback(window, rueda);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); glEnable(GL_DEPTH_TEST); glDisable(GL_CULL_FACE);

    // 2. Compilo el shader de geometria sencilla. El uniform "field" permite
    //    distinguir el cesped texturizado, los colores por vertice y los objetos tintados.
    GLuint programa = glCreateProgram(); GLuint vs = glCreateShader(GL_VERTEX_SHADER), fs = glCreateShader(GL_FRAGMENT_SHADER);
    // El vertex shader transforma coordenadas del modelo a clip y envia al
    // fragment shader la posicion global y el color de cada vertice.
    std::string vert = "#version 330 core\nlayout(location=0) in vec3 p; layout(location=1) in vec3 inputColor; uniform mat4 model,view,projection; out vec3 world; out vec3 vertexColor; void main(){ vec4 w=model*vec4(p,1); world=w.xyz; vertexColor=inputColor; gl_Position=projection*view*w;}";
    // El fragment shader aplica cesped, color solido o color por vertice,
    // ademas de la iluminacion de los cuatro reflectores.
    std::string frag = R"GLSL(#version 330 core
in vec3 world;
in vec3 vertexColor;
uniform vec3 tint;
uniform int field;
uniform int lamps;
uniform int uiMode;
uniform vec3 spotPos[4];
uniform vec3 spotDir[4];
uniform sampler2D grassTexture;
out vec4 color;

// Iluminacion para geometria sencilla: una base ambiental y una luz
// direccional general permanecen activas; los reflectores agregan sus conos.
float stadiumLight(vec3 normal) {
    vec3 direccionDia = normalize(vec3(-0.35, 0.86, 0.30));
    float ambiente = 0.22;
    float direccional = max(dot(normal, direccionDia), 0.0) * 0.34;
    float result = ambiente + direccional;
    if (lamps == 1) {
        for (int i = 0; i < 4; ++i) {
            vec3 ray = world - spotPos[i];
            float d = length(ray);
            float cone = dot(normalize(ray), normalize(spotDir[i]));
            float beam = smoothstep(0.28, 0.78, cone);
            float diffuse = max(dot(normal, normalize(-ray)), 0.0);
            float attenuation = 1.0 / (1.0 + 0.006 * d + 0.00025 * d * d);
            result += beam * diffuse * 1.8 * attenuation;
        }
    }
    return clamp(result, 0.20, 1.55);
}

void main() {
    float stripe = mod(floor((world.z + 52.5) / 10.5), 2.0);
    vec3 texel = texture(grassTexture, world.xz * 0.20).rgb * mix(0.64, 1.0, stripe);
    float isLine = step(0.80, min(vertexColor.r, min(vertexColor.g, vertexColor.b)));
    vec3 c = (field == 2) ? vertexColor : ((field == 1) ? mix(texel, vertexColor, isLine) : tint);
    float illumination = uiMode == 1 ? 1.0 : max(0.55, stadiumLight(vec3(0, 1, 0)));
    color = vec4(c * illumination, 1.0);
}
)GLSL";
    const char* v = vert.c_str(); const char* f = frag.c_str(); glShaderSource(vs, 1, &v, nullptr); glShaderSource(fs, 1, &f, nullptr); glCompileShader(vs); glCompileShader(fs);
    GLint ok = 0; glGetShaderiv(vs, GL_COMPILE_STATUS, &ok); if (!ok) std::cerr << "Vertex shader compilation failed\n";
    glGetShaderiv(fs, GL_COMPILE_STATUS, &ok); if (!ok) std::cerr << "Fragment shader compilation failed\n";
    glAttachShader(programa, vs); glAttachShader(programa, fs); glLinkProgram(programa); glGetProgramiv(programa, GL_LINK_STATUS, &ok);
    if (!ok) std::cerr << "Shader link failed\n"; glDeleteShader(vs); glDeleteShader(fs);
    construirVallasYConos();

    // 3. Creo los VAO/VBO de la cancha, cubos, conos y fuegos artificiales.
    GLuint vao, vbo, cuboVao, cuboVbo, conosVao, conosVbo, fuegosVao, fuegosVbo; glGenVertexArrays(1, &vao); glGenBuffers(1, &vbo); glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo); glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0); glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float))); glEnableVertexAttribArray(1);
    glGenVertexArrays(1, &cuboVao); glGenBuffers(1, &cuboVbo); glBindVertexArray(cuboVao); glBindBuffer(GL_ARRAY_BUFFER, cuboVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubo), cubo, GL_STATIC_DRAW); glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0); glEnableVertexAttribArray(0);
    glGenVertexArrays(1, &conosVao); glGenBuffers(1, &conosVbo); glBindVertexArray(conosVao); glBindBuffer(GL_ARRAY_BUFFER, conosVbo);
    glBufferData(GL_ARRAY_BUFFER, geometriaConos.size() * sizeof(float), geometriaConos.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0); glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float))); glEnableVertexAttribArray(1);
    glBindVertexArray(cuboVao);
    glGenVertexArrays(1, &fuegosVao); glGenBuffers(1, &fuegosVbo); glBindVertexArray(fuegosVao); glBindBuffer(GL_ARRAY_BUFFER, fuegosVbo);
    glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0); glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float))); glEnableVertexAttribArray(1);
    glBindVertexArray(cuboVao);
    GLuint tifoVao, tifoVbo;
    // 4. Genero en memoria una textura repetible de cesped con variacion fina,
    //    manchones y fibras; asi no dependo de una imagen externa para el campo.
    const int TAMANO_CESPED = 128;
    unsigned char cesped[TAMANO_CESPED * TAMANO_CESPED * 3];
    auto canalCesped = [](float valor) {
        if (valor < 0.0f) valor = 0.0f;
        if (valor > 255.0f) valor = 255.0f;
        return (unsigned char)(valor + 0.5f);
        };
    for (int y = 0; y < TAMANO_CESPED; ++y) {
        for (int x = 0; x < TAMANO_CESPED; ++x) {
            unsigned int hash = (unsigned int)x * 374761393u + (unsigned int)y * 668265263u;
            hash = (hash ^ (hash >> 13)) * 1274126177u;
            float ruido = ((float)(hash & 0xffffu) / 32767.5f) - 1.0f;
            float manchon = sinf(x * 0.105f + sinf(y * 0.075f) * 2.2f) *
                cosf(y * 0.09f + sinf(x * 0.06f) * 1.7f);
            float fibra = sinf(x * 0.78f + y * 0.16f + sinf(y * 0.12f) * 2.0f);
            float variacion = manchon * 13.0f + fibra * 10.0f + ruido * 9.0f;
            int k = (y * TAMANO_CESPED + x) * 3;
            cesped[k + 0] = canalCesped(45.0f + variacion * 0.90f);
            cesped[k + 1] = canalCesped(104.0f + manchon * 19.0f + fibra * 17.0f + ruido * 15.0f);
            cesped[k + 2] = canalCesped(34.0f + manchon * 8.0f + fibra * 6.0f + ruido * 6.0f);
        }
    }
    GLuint texturaCesped; glGenTextures(1, &texturaCesped); glBindTexture(GL_TEXTURE_2D, texturaCesped);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TAMANO_CESPED, TAMANO_CESPED, 0, GL_RGB, GL_UNSIGNED_BYTE, cesped); glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    GLuint texturaAtlas = SOIL_load_OGL_texture("Models/ProyectoGrafica/uniformes/atlas.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);
    GLuint texturaCruzAzul = SOIL_load_OGL_texture("Models/ProyectoGrafica/uniformes/cruz_azul.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);
    // Los tifos se buscan en Tifos/ y tambien en carpetas cercanas, porque
    // Visual Studio puede iniciar el programa desde distintos directorios.
    auto cargarTexturaTifo = [](const std::vector<std::string>& nombres) {
        const std::vector<std::string> carpetas = { "Tifos/", "../Tifos/", "../../Tifos/" };
        for (const std::string& carpeta : carpetas) for (const std::string& nombre : nombres) {
            const std::string ruta = carpeta + nombre;
            GLuint textura = SOIL_load_OGL_texture(ruta.c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);
            if (textura) { std::cout << "Tifo cargado: " << ruta << std::endl; return textura; }
        }
        return (GLuint)0;
        };
    GLuint texturaTifoCruz = cargarTexturaTifo({ "escudo_cruz_azul.png", "cruz_azul.png", "Cruz Azul.png", "codex-clipboard-18ce7427-98b2-428b-b7f1-973176e6dda8.png" });
    GLuint texturaTifoPumas = cargarTexturaTifo({ "escudo_pumas.png", "pumas.png", "Pumas.png", "codex-clipboard-3d162b78-de82-48ef-84f8-f040064e9075.png" });
    if (!texturaAtlas) std::cerr << "No se pudo cargar el uniforme Atlas: Models/ProyectoGrafica/uniformes/atlas.png" << std::endl;
    if (!texturaCruzAzul) std::cerr << "No se pudo cargar el uniforme Cruz Azul: Models/ProyectoGrafica/uniformes/cruz_azul.png" << std::endl;
    if (!texturaTifoCruz) std::cerr << "No se pudo cargar el tifo de Cruz Azul" << std::endl;
    if (!texturaTifoPumas) std::cerr << "No se pudo cargar el tifo de Pumas" << std::endl;

    std::vector<float> verticesTifo;
    // Guarda por vertice los tres atributos que necesita el shader de modelos.
    auto agregaVerticeTifo = [&](glm::vec3 p, glm::vec3 n, glm::vec2 uv) {
        verticesTifo.insert(verticesTifo.end(), { p.x,p.y,p.z,n.x,n.y,n.z,uv.x,uv.y });
        };
    // Calcula las cuatro esquinas y las UV de un paño vertical rectangular.
    auto agregaPlanoTifo = [&](float x, float y, float z, float ancho, float alto, float normalX) {
        float z0 = z - ancho * 0.5f, z1 = z + ancho * 0.5f;
        float y0 = y - alto * 0.5f, y1 = y + alto * 0.5f;
        glm::vec3 n(normalX, 0, 0), a(x, y0, z0), b(x, y1, z0), c(x, y1, z1), d(x, y0, z1);
        agregaVerticeTifo(a, n, glm::vec2(0, 0)); agregaVerticeTifo(b, n, glm::vec2(0, 1)); agregaVerticeTifo(c, n, glm::vec2(1, 1));
        agregaVerticeTifo(a, n, glm::vec2(0, 0)); agregaVerticeTifo(c, n, glm::vec2(1, 1)); agregaVerticeTifo(d, n, glm::vec2(1, 0));
        };
    agregaPlanoTifo(-38.20f, 14.5f, 0.0f, 8.0f, 8.0f, 1.0f);
    agregaPlanoTifo(38.20f, 14.5f, 0.0f, 12.0f, 9.0f, -1.0f);
    glGenVertexArrays(1, &tifoVao); glGenBuffers(1, &tifoVbo); glBindVertexArray(tifoVao); glBindBuffer(GL_ARRAY_BUFFER, tifoVbo);
    glBufferData(GL_ARRAY_BUFFER, verticesTifo.size() * sizeof(float), verticesTifo.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0); glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float))); glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float))); glEnableVertexAttribArray(2);
    glBindVertexArray(cuboVao);
    // 5. El segundo shader procesa normales y coordenadas UV de los modelos OBJ,
    //    para aplicar sus materiales, uniformes y textura de cesped.
    GLuint programaModelos = glCreateProgram(); GLuint modeloVS = glCreateShader(GL_VERTEX_SHADER), modeloFS = glCreateShader(GL_FRAGMENT_SHADER);
    // Este vertex shader transforma tambien la normal con la matriz inversa
    // transpuesta para que la iluminacion siga siendo correcta al escalar.
    std::string vertexModelo = "#version 330 core\nlayout(location=0) in vec3 p; layout(location=1) in vec3 n; layout(location=2) in vec2 uv; uniform mat4 model,view,projection; out vec3 world; out vec3 normalWorld; out vec2 texCoord; void main(){ vec4 wp=model*vec4(p,1.0); world=wp.xyz; normalWorld=mat3(transpose(inverse(model)))*n; texCoord=uv; gl_Position=projection*view*wp; }";
    std::string fragmentModelo = R"GLSL(#version 330 core
in vec3 world;
in vec3 normalWorld;
in vec2 texCoord;
uniform sampler2D diffuseTexture;
uniform vec3 materialColor;
uniform vec3 teamColor;
uniform int teamColorable;
uniform int hasTexture;
uniform int lamps;
uniform vec3 pointLightPos;
uniform vec3 pointLightColor;
uniform vec3 spotPos[4];
uniform vec3 spotDir[4];
out vec4 fragColor;

// Ruido pseudoaleatorio por celda para evitar que los materiales arquitectonicos
// queden completamente planos.
float ruido(vec3 p) {
    return fract(sin(dot(p, vec3(127.1, 311.7, 74.7))) * 43758.5453);
}

// Simula grano y juntas para distinguir concreto, techo y paneles rojos del estadio.
vec3 texturaArquitectura(vec3 base) {
    vec3 celda = floor(world * 7.0);
    float grano = ruido(celda) - 0.5;
    float juntaX = 1.0 - smoothstep(0.006, 0.022, abs(fract(world.x * 0.12) - 0.5));
    float juntaZ = 1.0 - smoothstep(0.006, 0.022, abs(fract(world.z * 0.12) - 0.5));
    float juntaY = 1.0 - smoothstep(0.004, 0.018, abs(fract(world.y * 0.22) - 0.5));
    float juntas = max(max(juntaX, juntaZ), juntaY);

    // Clasifico el material por su color base y aplico un acabado acorde.
    float gris = step(0.28, materialColor.r) * (1.0 - step(0.82, materialColor.r))
               * (1.0 - smoothstep(0.10, 0.24, abs(materialColor.r - materialColor.g)))
               * (1.0 - smoothstep(0.10, 0.24, abs(materialColor.g - materialColor.b)));
    float blanco = step(0.82, materialColor.r) * step(0.82, materialColor.g) * step(0.82, materialColor.b);
    float rojo = step(materialColor.g * 2.0 + 0.08, materialColor.r) * step(materialColor.b * 2.0 + 0.08, materialColor.r);

    vec3 concreto = base * (0.94 + grano * 0.075);
    concreto *= 1.0 - juntas * 0.22;
    float panelTecho = 0.5 + 0.5 * sin(world.x * 0.16 + sin(world.z * 0.055));
    vec3 techo = base * (0.94 + panelTecho * 0.08 + grano * 0.025);
    techo *= 1.0 - max(juntaX, juntaZ) * 0.12;
    float ritmoRojo = 0.96 + 0.04 * step(0.5, fract(world.y * 0.13));
    vec3 panelRojo = base * ritmoRojo * (0.97 + grano * 0.035);

    vec3 acabado = mix(base, concreto, gris);
    acabado = mix(acabado, techo, blanco);
    acabado = mix(acabado, panelRojo, rojo);
    return acabado;
}

void main() {
    vec3 base = hasTexture == 1 ? texture(diffuseTexture, texCoord).rgb : materialColor;
    if (teamColorable == 1) base = teamColor;
    else if (hasTexture == 0) base = texturaArquitectura(base);
    // Aplicar el acabado directamente a cualquier material verde del modelo
    // del estadio: ese es el césped que se ve en la captura.
    // La mascara identifica materiales verdes sin afectar lineas ni uniformes.
    float mascaraVerde = smoothstep(0.025, 0.12, base.g)
                       * (1.0 - smoothstep(0.10, 0.36, base.r))
                       * (1.0 - smoothstep(0.08, 0.30, base.b))
                       * smoothstep(1.08, 1.35, base.g / (base.r + 0.005))
                       * smoothstep(1.08, 1.35, base.g / (base.b + 0.005));
    // Alterno franjas de corte y una fibra fina para dar variacion al cesped.
    float franjaCorte = mod(floor((world.z + 34.0) / 6.8), 2.0);
    float fibraCesped = sin(world.x * 10.0 + world.z * 1.8 + sin(world.z * 0.65) * 2.0);
    vec3 albedoCesped = base * mix(0.62, 1.0, franjaCorte) * (0.93 + 0.07 * fibraCesped);
    base = mix(base, albedoCesped, mascaraVerde);
    // La normal determina cuanto aporta cada reflector sobre la superficie.
    vec3 normal = normalize(normalWorld);
    // La luz ambiental da visibilidad uniforme y la direccional simula luz del dia.
    // Los reflectores del estadio se suman despues cuando estan encendidos.
    vec3 direccionDia = normalize(vec3(-0.35, 0.86, 0.30));
    float ambiente = 0.20;
    float direccional = max(dot(normal, direccionDia), 0.0) * 0.38;
    float illumination = ambiente + direccional;
    // Luz puntual calida junto al marcador; su aporte disminuye en todas
    // direcciones segun la distancia y la orientacion de cada superficie.
    vec3 haciaPuntual = pointLightPos - world;
    float distanciaPuntual = length(haciaPuntual);
    float difusaPuntual = max(dot(normal, normalize(haciaPuntual)), 0.0);
    float atenuacionPuntual = 1.0 / (1.0 + 0.08 * distanciaPuntual + 0.025 * distanciaPuntual * distanciaPuntual);
    vec3 aportePuntual = pointLightColor * difusaPuntual * atenuacionPuntual * 0.75;
    if (lamps == 1) for (int i = 0; i < 4; ++i) {
        vec3 ray = world - spotPos[i];
        float d = length(ray);
        float cone = dot(normalize(ray), normalize(spotDir[i]));
        float beam = smoothstep(0.28, 0.78, cone);
        float diffuse = max(dot(normal, normalize(-ray)), 0.0);
        illumination += beam * diffuse * 2.0 / (1.0 + 0.006 * d + 0.00025 * d * d);
    }
    fragColor = vec4(base * clamp(illumination, 0.20, 1.55) + aportePuntual, 1.0);
}
)GLSL";
    const char* mv = vertexModelo.c_str(); const char* mf = fragmentModelo.c_str(); glShaderSource(modeloVS, 1, &mv, nullptr); glShaderSource(modeloFS, 1, &mf, nullptr); glCompileShader(modeloVS); glCompileShader(modeloFS);
    glAttachShader(programaModelos, modeloVS); glAttachShader(programaModelos, modeloFS); glLinkProgram(programaModelos);
    glDeleteShader(modeloVS); glDeleteShader(modeloFS);
    // Fuente puntual cálida colocada encima del marcador del estadio.
    glUseProgram(programaModelos);
    glUniform3f(glGetUniformLocation(programaModelos, "pointLightPos"), -62.0f, 20.0f, 0.0f);
    glUniform3f(glGetUniformLocation(programaModelos, "pointLightColor"), 1.0f, 0.72f, 0.38f);
    glUseProgram(programa);

    struct Instancia { size_t modelo; glm::vec3 posicion; float tamano; float rotacion; float inclinacion; };
    // 6. Mantengo las rutas de los recursos en un solo arreglo para facilitar
    //    su revision si cambia la estructura de Models/ProyectoGrafica.
    const char* rutasModelos[] = {
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
    const char* filtrosModelo[] = { nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,
        "Jugador_CHICHARITO|CHICHARITO_|","Arbitro|ARBITRO_|",nullptr,nullptr,nullptr,nullptr,nullptr,nullptr };
    // Cada filtro permite cargar solo el grupo de mallas nombrado del OBJ;
    // asi separo torso, brazos y piernas para poder transformarlos por articulacion.
    std::vector<ModeloOBJ> modelos; modelos.reserve(28);
    int modelosCargados = 0;
    for (int i = 0;i < 17;++i) { modelos.emplace_back(); if (modelos.back().cargar(rutasModelos[i], filtrosModelo[i] ? filtrosModelo[i] : "")) ++modelosCargados; }
    std::cout << "Modelos OBJ cargados: " << modelosCargados << "/17 (16 modelos fuente; jugador y arbitro separados)" << std::endl;
    const std::string rutaJugador = rutasModelos[9];
    const char* filtrosArticulados[] = {
        "Jugador_CHICHARITO|CHICHARITO_|!Jugador_CHICHARITO_antebrazo_derecho_|!Jugador_CHICHARITO_antebrazo_izquierdo_|!Jugador_CHICHARITO_codo_derecho_|!Jugador_CHICHARITO_codo_izquierdo_|!Jugador_CHICHARITO_muslo_derecha_|!Jugador_CHICHARITO_muslo_izquierda_|!Jugador_CHICHARITO_pierna_derecha_|!Jugador_CHICHARITO_pierna_izquierda_|!Jugador_CHICHARITO_rodilla_derecha_|!Jugador_CHICHARITO_rodilla_izquierda_|!Jugador_CHICHARITO_tobillo_derecha_|!Jugador_CHICHARITO_tobillo_izquierda_|!Jugador_CHICHARITO_banda_media_|!Jugador_CHICHARITO_agujeta_|!Jugador_CHICHARITO_suela_|!Jugador_CHICHARITO_tachon_|!CHICHARITO_\\|_palma_|!CHICHARITO_\\|_manga_anatómica_",
        "CHICHARITO_\\|_manga_anatómica_derecha_",
        "Jugador_CHICHARITO_antebrazo_derecho_|Jugador_CHICHARITO_codo_derecho_|CHICHARITO_\\|_palma_derecha_",
        "CHICHARITO_\\|_manga_anatómica_izquierda_",
        "Jugador_CHICHARITO_antebrazo_izquierdo_|Jugador_CHICHARITO_codo_izquierdo_|CHICHARITO_\\|_palma_izquierda_",
        "Jugador_CHICHARITO_muslo_derecha_|Jugador_CHICHARITO_rodilla_derecha_",
        "Jugador_CHICHARITO_pierna_derecha_|Jugador_CHICHARITO_tobillo_derecha_|Jugador_CHICHARITO_banda_media_derecha",
        "Jugador_CHICHARITO_agujeta_derecha|Jugador_CHICHARITO_suela_derecha_|Jugador_CHICHARITO_tachon_derecha_",
        "Jugador_CHICHARITO_muslo_izquierda_|Jugador_CHICHARITO_rodilla_izquierda_",
        "Jugador_CHICHARITO_pierna_izquierda_|Jugador_CHICHARITO_tobillo_izquierda_|Jugador_CHICHARITO_banda_media_izquierda",
        "Jugador_CHICHARITO_agujeta_izquierda|Jugador_CHICHARITO_suela_izquierda_|Jugador_CHICHARITO_tachon_izquierda_"
    };
    // Estos patrones dividen el modelo del jugador en piezas y excluyen las
    // mismas piezas del cuerpo principal para evitar que aparezcan duplicadas.
    for (size_t i = 0;i < sizeof(filtrosArticulados) / sizeof(filtrosArticulados[0]);++i) {
        modelos.emplace_back();
        if (!modelos.back().cargar(rutaJugador, filtrosArticulados[i]))
            std::cerr << "No se pudo cargar una parte articulada del jugador: " << filtrosArticulados[i] << std::endl;
    }
    // Cada instancia combina el indice del modelo con posicion, escala y orientacion.
    std::vector<Instancia> instancias;
    auto instancia = [&](size_t m, glm::vec3 p, float s, float r = 0.0f, float x = 0.0f) { instancias.push_back({ m,p,s,r,x }); };
    // El campo importado mide aprox. 101 x 68 unidades: largo en X, ancho en Z.
    instancia(0, posicionAutobus, 14, rumboAutobus);
    instancia(2, glm::vec3(0, 7, -40), 1.5f);
    instancia(3, glm::vec3(-62, 14, 0), 11, 90);
    // Se retira el carrito de emergencias de la valla lateral para dejar un pasillo libre.
    instancia(4, glm::vec3(-30, 0, -27.0f), 2.6f);
    instancia(5, glm::vec3(0, 0, 0), 210);
    instancia(6, glm::vec3(10, 0, -26), 8);
    instancia(7, glm::vec3(10, 1.0f, -26), 3.5f);
    // Bases de los iluminadores sobre el anillo exterior de la cubierta.
    instancia(8, glm::vec3(-50, 17, -40), 12, 45, 28); instancia(8, glm::vec3(50, 17, -40), 12, -45, 28);
    instancia(8, glm::vec3(-50, 17, 40), 12, 135, 28); instancia(8, glm::vec3(50, 17, 40), 12, -135, 28);
    // Porterias centradas en las lineas de fondo; la boca queda transversal al largo del campo.
    instancia(11, glm::vec3(-33.4f, 0, 0), 8, 90); instancia(11, glm::vec3(33.4f, 0, 0), 8, -90);
    // Bancas de suplentes fuera de la linea lateral, una por equipo.
    instancia(12, glm::vec3(0, 0.15f, -27), 5.0f, 90); instancia(13, glm::vec3(0, 0.15f, 27), 5.0f, 270);
    // El carrito de comida va en la grada interior, elevado sobre el borde de la cancha.
    instancia(14, glm::vec3(-0, 7, 48), 4.0f); instancia(15, glm::vec3(-14, 0, 23), 2.4f);
    // Cuatro banderas sobre las esquinas reales del terreno.
    instancia(16, glm::vec3(-34.0f, -0.5, -21.9f), 4.5f); instancia(16, glm::vec3(-34.0f, -0.5, 21.9f), 4.5f);
    instancia(16, glm::vec3(34.0f, -0.5, 21.9f), 4.5f); instancia(16, glm::vec3(34.0f, -0.5, -21.9f), 4.5f);
    // Guardo las ubicaciones de uniformes una vez, en vez de buscarlas cada cuadro.
    GLint locM = glGetUniformLocation(programa, "model"), locV = glGetUniformLocation(programa, "view"), locP = glGetUniformLocation(programa, "projection");
    GLint locColor = glGetUniformLocation(programa, "tint"), locField = glGetUniformLocation(programa, "field"), locLamps = glGetUniformLocation(programa, "lamps"), locUI = glGetUniformLocation(programa, "uiMode");
    // Los focos de luz coinciden con los cuatro reflectores ya colocados en la escena.
    const glm::vec3 posicionesSpot[4] = { glm::vec3(-50,29,-40),glm::vec3(50,29,-40),glm::vec3(-50,29,40),glm::vec3(50,29,40) };
    glm::vec3 direccionesSpot[4];
    for (int i = 0;i < 4;++i) direccionesSpot[i] = glm::normalize(-posicionesSpot[i]);
    // Envio al shader las posiciones y direcciones de los cuatro reflectores.
    auto cargarSpots = [&](GLuint programaActivo) {
        for (int i = 0;i < 4;++i) {
            std::string nombrePos = "spotPos[" + std::to_string(i) + "]", nombreDir = "spotDir[" + std::to_string(i) + "]";
            glUniform3fv(glGetUniformLocation(programaActivo, nombrePos.c_str()), 1, glm::value_ptr(posicionesSpot[i]));
            glUniform3fv(glGetUniformLocation(programaActivo, nombreDir.c_str()), 1, glm::value_ptr(direccionesSpot[i]));
        }
        };
    glUseProgram(programa); glUniform1i(glGetUniformLocation(programa, "grassTexture"), 0);
    double anterior = glfwGetTime();
    // 7. Ciclo principal: mido el tiempo, proceso entrada, actualizo estado y dibujo.
    while (!glfwWindowShouldClose(window)) {
        double ahora = glfwGetTime(); float dt = (float)glm::clamp(ahora - anterior, 0.0, 0.05); anterior = ahora; glfwPollEvents();
        float yaw = glm::radians(anguloMouseX); glm::vec3 frente = glm::normalize(glm::vec3(cos(yaw), 0, sin(yaw)));
        glm::vec3 derecha = glm::normalize(glm::cross(frente, glm::vec3(0, 1, 0)));
        float velocidadJugador = primeraPersona ? 9.0f : 8.0f;
        // Conduccion independiente del jugador: acelerador/freno y giro del autobus.
        const float velocidadAutobus = 16.0f, giroAutobus = 75.0f;
        if (teclas[GLFW_KEY_LEFT]) rumboAutobus -= giroAutobus * dt;
        if (teclas[GLFW_KEY_RIGHT]) rumboAutobus += giroAutobus * dt;
        float rumboRad = glm::radians(rumboAutobus);
        glm::vec3 frenteAutobus(std::sin(rumboRad), 0.0f, std::cos(rumboRad));
        if (teclas[GLFW_KEY_UP]) posicionAutobus += frenteAutobus * velocidadAutobus * dt;
        if (teclas[GLFW_KEY_DOWN]) posicionAutobus -= frenteAutobus * velocidadAutobus * dt;
        // Limites exteriores para mantenerlo recorriendo el estadio y evitar perderlo de vista.
        posicionAutobus.x = glm::clamp(posicionAutobus.x, -88.0f, 88.0f);
        posicionAutobus.z = glm::clamp(posicionAutobus.z, -76.0f, 76.0f);
        instancias[0].posicion = posicionAutobus;
        instancias[0].rotacion = rumboAutobus;

        if (camaraLibre) {
            const float velocidadCamaraLibre = 32.0f;
            glm::vec3 frenteLibre(cos(yaw), 0.0f, sin(yaw));
            glm::vec3 derechaLibre = glm::normalize(glm::cross(frenteLibre, glm::vec3(0, 1, 0)));
            if (teclas[GLFW_KEY_W]) posicionCamaraLibre += frenteLibre * dt * velocidadCamaraLibre;
            if (teclas[GLFW_KEY_S]) posicionCamaraLibre -= frenteLibre * dt * velocidadCamaraLibre;
            if (teclas[GLFW_KEY_A]) posicionCamaraLibre -= derechaLibre * dt * velocidadCamaraLibre;
            if (teclas[GLFW_KEY_D]) posicionCamaraLibre += derechaLibre * dt * velocidadCamaraLibre;
            if (teclas[GLFW_KEY_E]) posicionCamaraLibre.y += dt * velocidadCamaraLibre;
            if (teclas[GLFW_KEY_Q]) posicionCamaraLibre.y -= dt * velocidadCamaraLibre;
            posicionCamaraLibre.y = glm::max(2.0f, posicionCamaraLibre.y);
        }
        else if (!animando) {
            if (teclas[GLFW_KEY_W]) jugador += frente * dt * velocidadJugador;
            if (teclas[GLFW_KEY_S]) jugador -= frente * dt * velocidadJugador;
            if (teclas[GLFW_KEY_A]) jugador -= derecha * dt * velocidadJugador;
            if (teclas[GLFW_KEY_D]) jugador += derecha * dt * velocidadJugador;
        }
        jugador.x = glm::clamp(jugador.x, -48.0f, 48.0f); jugador.z = glm::clamp(jugador.z, -31.0f, 31.0f);
        // Actualizo el tiro o pase independiente cuadro a cuadro, aplicando
        // gravedad, rebote y friccion para que el balon no se desplace sin fin.
        tiempoReaccionPortero = glm::max(0.0f, tiempoReaccionPortero - dt);
        if (balonLibre && !animando) {
            if (balonEnMovimiento) {
                glm::vec3 posicionAnterior = posicionBalonManual;
                velocidadBalonManual.y -= 9.8f * dt;
                posicionBalonManual += velocidadBalonManual * dt;
                if (posicionBalonManual.y < 0.22f) {
                    posicionBalonManual.y = 0.22f;
                    velocidadBalonManual.y = -velocidadBalonManual.y * 0.24f;
                    if (fabs(velocidadBalonManual.y) < 1.0f) velocidadBalonManual.y = 0.0f;
                    velocidadBalonManual.x *= 0.985f;
                    velocidadBalonManual.z *= 0.985f;
                }
                float signoPorteria = velocidadBalonManual.x >= 0.0f ? 1.0f : -1.0f;
                float planoPortero = signoPorteria * 31.0f;
                bool cruzaPortero = signoPorteria > 0.0f
                    ? posicionAnterior.x < planoPortero && posicionBalonManual.x >= planoPortero
                    : posicionAnterior.x > planoPortero && posicionBalonManual.x <= planoPortero;
                // Una pelota que pasa por el centro y a una altura alcanzable
                // activa una atajada; tiros laterales pueden superar al portero.
                if (cruzaPortero && fabs(posicionBalonManual.z) < 1.45f && posicionBalonManual.y < 2.25f) {
                    posicionBalonManual.x = planoPortero - signoPorteria * 0.25f;
                    velocidadBalonManual = glm::vec3(0.0f);
                    balonEnMovimiento = false;
                    tiempoReaccionPortero = 0.75f;
                    ladoPorteroReacciona = (int)signoPorteria;
                    std::cout << "Atajada del portero. Acercate al balon para recuperarlo." << std::endl;
                }
                bool cruzaLineaGol = signoPorteria > 0.0f
                    ? posicionAnterior.x < 33.4f && posicionBalonManual.x >= 33.4f
                    : posicionAnterior.x > -33.4f && posicionBalonManual.x <= -33.4f;
                if (balonEnMovimiento && cruzaLineaGol) {
                    if (fabs(posicionBalonManual.z) <= 3.65f && posicionBalonManual.y <= 2.44f) {
                        ++golesLocal;
                        posicionBalonManual.x = signoPorteria * 34.2f;
                        std::cout << "GOOOL. Marcador local: " << golesLocal << " - visitante: " << golesVisita << std::endl;
                    }
                    else {
                        std::cout << "Tiro desviado; el balon salio y vuelve contigo." << std::endl;
                        balonLibre = false;
                    }
                    balonEnMovimiento = false;
                    velocidadBalonManual = glm::vec3(0.0f);
                }
                // Las bandas estan en Z +/-22 aproximadamente. Si cruza una,
                // termina la jugada y recupero la posesion para el usuario.
                if (balonEnMovimiento && (fabs(posicionBalonManual.x) > 34.5f || fabs(posicionBalonManual.z) > 22.2f)) {
                    balonEnMovimiento = false;
                    balonLibre = false;
                    velocidadBalonManual = glm::vec3(0.0f);
                    tiempoReaccionPortero = 0.0f;
                    std::cout << "El balon salio de la cancha; ya lo tienes de nuevo." << std::endl;
                }
                glm::vec2 velocidadHorizontal(velocidadBalonManual.x, velocidadBalonManual.z);
                if (glm::length(velocidadHorizontal) < 0.55f && posicionBalonManual.y <= 0.23f) {
                    balonEnMovimiento = false;
                    velocidadBalonManual = glm::vec3(0.0f);
                }
            }
            // Cuando el jugador llega a un balon detenido, recupera la posesion.
            if (!balonEnMovimiento && glm::distance(glm::vec2(jugador.x, jugador.z), glm::vec2(posicionBalonManual.x, posicionBalonManual.z)) < 1.5f)
                balonLibre = false;
        }
        // La animacion usa un reloj en segundos. El marcador se actualiza una
        // sola vez al cruzar el balon la linea, y luego termina la secuencia.
        if (animando) {
            tiempoAnimacion += dt;
            // El marcador cambia justo cuando el balon cruza el plano de gol.
            if (!marcadorJugadaActualizado && tiempoAnimacion >= 5.22f) {
                golesLocal = (golesLocal + 1) % 100;
                marcadorJugadaActualizado = true;
                std::cout << "GOL. Marcador local: " << golesLocal << " - visitante: " << golesVisita << std::endl;
            }
            if (tiempoAnimacion >= 6.5f) { tiempoAnimacion = 6.5f; animando = false; jugadaTerminada = true; }
        }
        // Seleccion de camara: primera persona, libre o seguimiento del jugador/autobus.
        glm::vec3 posCam, objetivo;
        if (primeraPersona) { posCam = jugador + glm::vec3(0, 1.7f, 0); float pitch = glm::radians(anguloMouseY); glm::vec3 f = glm::normalize(glm::vec3(cos(yaw) * cos(pitch), sin(pitch), sin(yaw) * cos(pitch))); objetivo = posCam + f; }
        else if (camaraLibre) { posCam = posicionCamaraLibre; float pitch = glm::radians(anguloMouseY); glm::vec3 f = glm::normalize(glm::vec3(cos(yaw) * cos(pitch), sin(pitch), sin(yaw) * cos(pitch))); objetivo = posCam + f; }
        else {
            glm::vec3 centroCamaraJugada = (animando || jugadaTerminada) ? glm::mix(inicioJugada, objetivoJugada, glm::clamp(tiempoAnimacion / 5.8f, 0.0f, 1.0f)) : jugador;
            objetivo = (camaraAutobus ? posicionAutobus : centroCamaraJugada) + glm::vec3(0, 2.0f, 0);
            float pitch = glm::radians(glm::clamp(anguloMouseY, 10.0f, 80.0f));
            posCam = objetivo - frente * (zoomCamara * cos(pitch)) + glm::vec3(0, zoomCamara * sin(pitch), 0);
        }
        // Construyo las matrices de vista y perspectiva para proyectar la escena.
        glm::mat4 vista = glm::lookAt(posCam, objetivo, glm::vec3(0, 1, 0)); int ww, hh; glfwGetFramebufferSize(window, &ww, &hh); if (hh == 0) hh = 1;
        glm::mat4 proy = glm::perspective(glm::radians(campoVision), (float)ww / hh, 0.1f, 520.0f);
        glViewport(0, 0, ww, hh); glClearColor(0.015f, 0.035f, 0.08f, 1); glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); glUseProgram(programa); cargarSpots(programa);
        glUniformMatrix4fv(locV, 1, GL_FALSE, glm::value_ptr(vista)); glUniformMatrix4fv(locP, 1, GL_FALSE, glm::value_ptr(proy)); glUniform1i(locLamps, reflectores ? 1 : 0); glUniform1i(locUI, 0);
        // Dibujo primero la arquitectura y los modelos estaticos del estadio.
        glBindVertexArray(cuboVao);
        for (const Caja& c : cajas) { glm::mat4 m = transformacion(c.centro, c.escala); glUniformMatrix4fv(locM, 1, GL_FALSE, glm::value_ptr(m)); glUniform3fv(locColor, 1, glm::value_ptr(c.color)); glDrawArrays(GL_TRIANGLES, 0, 36); }
        // Dibujo los modelos OBJ con su shader, materiales y transformaciones.
        glUseProgram(programaModelos); cargarSpots(programaModelos);
        for (const Instancia& item : instancias) {
            Instancia instanciaAnimada = item;
            // Simular el latigazo de la red cuando el balon pega dentro del arco.
            bool esPorteriaObjetivo = item.modelo == 11 && item.posicion.x * objetivoJugada.x > 0.0f;
            float desdeImpacto = tiempoAnimacion - 5.22f;
            if ((animando || jugadaTerminada) && esPorteriaObjetivo && desdeImpacto >= 0.0f && desdeImpacto < 1.0f) {
                float vibracion = std::sin(desdeImpacto * 31.0f) * std::exp(-desdeImpacto * 5.0f);
                instanciaAnimada.posicion.x += direccionTiro.x * 0.35f * vibracion;
                instanciaAnimada.rotacion += 2.2f * vibracion;
                instanciaAnimada.inclinacion += 1.5f * vibracion;
            }
            modelos[instanciaAnimada.modelo].dibujar(programaModelos, vista, proy, instanciaAnimada.posicion,
                instanciaAnimada.tamano, instanciaAnimada.rotacion, reflectores, instanciaAnimada.inclinacion);
        }
        // La cancha se dibuja despues del modelo del estadio para conservar
        // visible la textura de cesped y las lineas reglamentarias.
        glUseProgram(programa);
        // Dibujar la cancha texturizada al final del escenario para que el
        // modelo del estadio no la cubra; las lineas blancas conservan su color.
        cargarSpots(programa);
        glUniformMatrix4fv(locV, 1, GL_FALSE, glm::value_ptr(vista));
        glUniformMatrix4fv(locP, 1, GL_FALSE, glm::value_ptr(proy));
        glUniform1i(locLamps, reflectores ? 1 : 0); glUniform1i(locUI, 0);
        glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, texturaCesped);
        glUniformMatrix4fv(locM, 1, GL_FALSE, glm::value_ptr(glm::mat4(1)));
        glUniform3f(locColor, 1, 1, 1); glUniform1i(locField, 1);
        glBindVertexArray(vao); glDrawArrays(GL_TRIANGLES, 0, (GLsizei)(vertices.size() / 6));
        glUniform1i(locField, 0); glBindVertexArray(cuboVao);
        glUniformMatrix4fv(locV, 1, GL_FALSE, glm::value_ptr(vista)); glUniformMatrix4fv(locP, 1, GL_FALSE, glm::value_ptr(proy)); glUniform1i(locLamps, reflectores ? 1 : 0);
        glBindVertexArray(cuboVao);
        auto dibujaParte = [&](glm::vec3 pos, glm::vec3 escala, glm::vec3 color, glm::mat4 base) { glm::mat4 m = base; m = glm::translate(m, pos); m = glm::scale(m, escala); glUniformMatrix4fv(locM, 1, GL_FALSE, glm::value_ptr(m)); glUniform3fv(locColor, 1, glm::value_ptr(color)); glDrawArrays(GL_TRIANGLES, 0, 36); };
        // Paneles publicitarios a ras del perimetro, sin cerrar accesos a las porterias
        // ni ocupar el hueco central de las bancas. Los conos quedan en el area de practica.
        glUseProgram(programa); glUniform1i(locField, 0); glBindVertexArray(cuboVao);
        for (const Caja& panel : vallasPublicidad) dibujaParte(panel.centro, panel.escala, panel.color, glm::mat4(1.0f));
        for (const Caja& panio : fondosTifo) dibujaParte(panio.centro, panio.escala, panio.color, glm::mat4(1.0f));
        glUniform1i(locField, 2); glUniformMatrix4fv(locM, 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));
        glBindVertexArray(conosVao); glDrawArrays(GL_TRIANGLES, 0, (GLsizei)(geometriaConos.size() / 6));
        glUniform1i(locField, 0); glBindVertexArray(cuboVao);
        // Dibujo los escudos como planos con textura en las cabeceras.
        glUseProgram(programaModelos);
        // Colocar los escudos de ambos equipos en sus respectivas cabeceras.
        cargarSpots(programaModelos);
        GLint locTifoModel = glGetUniformLocation(programaModelos, "model");
        GLint locTifoView = glGetUniformLocation(programaModelos, "view");
        GLint locTifoProjection = glGetUniformLocation(programaModelos, "projection");
        GLint locTifoTexture = glGetUniformLocation(programaModelos, "diffuseTexture");
        GLint locTifoHasTexture = glGetUniformLocation(programaModelos, "hasTexture");
        GLint locTifoColor = glGetUniformLocation(programaModelos, "materialColor");
        GLint locTifoTeamColorable = glGetUniformLocation(programaModelos, "teamColorable");
        glUniformMatrix4fv(locTifoModel, 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));
        glUniformMatrix4fv(locTifoView, 1, GL_FALSE, glm::value_ptr(vista));
        glUniformMatrix4fv(locTifoProjection, 1, GL_FALSE, glm::value_ptr(proy));
        glUniform1i(locTifoTexture, 0); glUniform1i(locTifoHasTexture, 1); glUniform1i(locTifoTeamColorable, 0);
        glUniform3f(locTifoColor, 1.0f, 1.0f, 1.0f); glActiveTexture(GL_TEXTURE0); glBindVertexArray(tifoVao);
        if (texturaTifoCruz) { glBindTexture(GL_TEXTURE_2D, texturaTifoCruz); glDrawArrays(GL_TRIANGLES, 0, 6); }
        if (texturaTifoPumas) { glBindTexture(GL_TEXTURE_2D, texturaTifoPumas); glDrawArrays(GL_TRIANGLES, 6, 6); }
        glBindVertexArray(cuboVao);
        // Tomo las dimensiones del modelo completo para centrarlo y escalarlo
        // antes de aplicar las transformaciones de sus articulaciones.
        const ModeloOBJ& marcoJugador = modelos[9];
        const float escalaJugador = 3.2f / marcoJugador.extensionMaximaModelo();
        glm::vec3 centroJugador = marcoJugador.centroModelo();
        // La matriz raiz posiciona y orienta al personaje; cada extremidad se
        // transforma despues con respecto a esta base comun.
        auto crearRaizJugador = [&](glm::vec3 posicion, float giroY) {
            glm::mat4 raiz(1.0f);
            raiz = glm::translate(raiz, posicion + glm::vec3(0, (centroJugador.y - marcoJugador.extensionVerticalMinima()) * escalaJugador, 0));
            raiz = glm::rotate(raiz, glm::radians(giroY), glm::vec3(0, 1, 0));
            raiz = glm::scale(raiz, glm::vec3(escalaJugador));
            return glm::translate(raiz, -centroJugador);
            };
        float rumboJugador = (animando || jugadaTerminada) ? glm::degrees(std::atan2(direccionTiro.x, direccionTiro.z)) : -90.0f;
        glm::mat4 raizJugador = crearRaizJugador(jugador, rumboJugador);
        const glm::vec3 colorAtlas(0.04f, 0.06f, 0.16f), colorCruzAzul(0.05f, 0.30f, 0.92f);
        // Dibuja la pose neutral del jugador, usando la misma raiz para que las
        // piezas importadas queden unidas como un modelo articulado.
        auto dibujaJugadorCompleto = [&](const glm::mat4& raiz, const glm::vec3& uniforme, GLuint texturaUniforme) {
            modelos[CUERPO_JUGADOR].dibujarConMatriz(programaModelos, vista, proy, raiz, reflectores, uniforme, true, texturaUniforme);
            modelos[BRAZO_SUP_DER].dibujarConMatriz(programaModelos, vista, proy, raiz, reflectores, uniforme, true, texturaUniforme);
            modelos[BRAZO_INF_DER].dibujarConMatriz(programaModelos, vista, proy, raiz, reflectores, uniforme, true, texturaUniforme);
            modelos[BRAZO_SUP_IZQ].dibujarConMatriz(programaModelos, vista, proy, raiz, reflectores, uniforme, true, texturaUniforme);
            modelos[BRAZO_INF_IZQ].dibujarConMatriz(programaModelos, vista, proy, raiz, reflectores, uniforme, true, texturaUniforme);
            modelos[MUSLO_DER].dibujarConMatriz(programaModelos, vista, proy, raiz, reflectores, uniforme, true, texturaUniforme);
            modelos[PANTORRILLA_DER].dibujarConMatriz(programaModelos, vista, proy, raiz, reflectores, uniforme, true, texturaUniforme);
            modelos[PIE_DER].dibujarConMatriz(programaModelos, vista, proy, raiz, reflectores, uniforme, true, texturaUniforme);
            modelos[MUSLO_IZQ].dibujarConMatriz(programaModelos, vista, proy, raiz, reflectores, uniforme, true, texturaUniforme);
            modelos[PANTORRILLA_IZQ].dibujarConMatriz(programaModelos, vista, proy, raiz, reflectores, uniforme, true, texturaUniforme);
            modelos[PIE_IZQ].dibujarConMatriz(programaModelos, vista, proy, raiz, reflectores, uniforme, true, texturaUniforme);
            };
        // Rota una pieza alrededor de una articulacion: lleva el pivote al
        // origen, aplica la rotacion y lo regresa a su posicion.
        auto giroArticulacion = [](glm::vec3 pivote, float grados, glm::vec3 eje) {
            glm::mat4 giro(1.0f); giro = glm::translate(giro, pivote); giro = glm::rotate(giro, glm::radians(grados), eje); return glm::translate(giro, -pivote);
            };
        bool jugadorEnMovimiento = teclas[GLFW_KEY_W] || teclas[GLFW_KEY_A] || teclas[GLFW_KEY_S] || teclas[GLFW_KEY_D];
        float caderaDer = 0, rodillaDer = 0, tobilloDer = 0, caderaIzq = 0, rodillaIzq = 0, tobilloIzq = 0, brazos = 0;
        if (animando) {
            auto interpolarPose = [&](float KeyframeJugador::* canal) {
                for (size_t k = 0;k + 1 < sizeof(keyframesJugador) / sizeof(keyframesJugador[0]);++k) {
                    const KeyframeJugador& a = keyframesJugador[k]; const KeyframeJugador& b = keyframesJugador[k + 1];
                    if (tiempoAnimacion >= a.tiempo && tiempoAnimacion <= b.tiempo) {
                        float u = (tiempoAnimacion - a.tiempo) / (b.tiempo - a.tiempo);
                        u = u * u * (3.0f - 2.0f * u);
                        return glm::mix(a.*canal, b.*canal, u);
                    }
                }
                return 0.0f;
                };
            caderaDer = interpolarPose(&KeyframeJugador::caderaDer); rodillaDer = interpolarPose(&KeyframeJugador::rodillaDer); tobilloDer = interpolarPose(&KeyframeJugador::tobilloDer);
            caderaIzq = interpolarPose(&KeyframeJugador::caderaIzq); rodillaIzq = interpolarPose(&KeyframeJugador::rodillaIzq); tobilloIzq = interpolarPose(&KeyframeJugador::tobilloIzq); brazos = interpolarPose(&KeyframeJugador::brazos);
        }
        else if (jugadorEnMovimiento) {
            float paso = sin((float)ahora * 9.0f) * 24.0f; caderaDer = paso; caderaIzq = -paso;
            rodillaDer = glm::max(0.0f, sin((float)ahora * 9.0f + 1.4f)) * 25.0f; rodillaIzq = glm::max(0.0f, sin((float)ahora * 9.0f + PI + 1.4f)) * 25.0f;
            tobilloDer = -rodillaDer * 0.25f; tobilloIzq = -rodillaIzq * 0.25f;
            brazos = -paso * 0.45f;
        }
        const glm::vec3 hombroDer(-0.94f, 2.55f, 0), hombroIzq(-1.56f, 2.55f, 0);
        const glm::vec3 codoDer(-0.73f, 2.13f, 0), codoIzq(-1.77f, 2.13f, 0);
        const glm::vec3 caderaJugadorDer(-1.12f, 1.70f, 0), caderaJugadorIzq(-1.38f, 1.70f, 0);
        const glm::vec3 rodillaJugadorDer(-1.06f, 1.05f, 0.015f), rodillaJugadorIzq(-1.44f, 1.05f, 0.015f);
        const glm::vec3 tobilloDerJugador(-1.05f, 0.42f, 0.015f), tobilloIzqJugador(-1.45f, 0.42f, 0.015f);
        glm::mat4 hombroDerM = giroArticulacion(hombroDer, brazos, glm::vec3(0, 1, 0));
        glm::mat4 hombroIzqM = giroArticulacion(hombroIzq, -brazos, glm::vec3(0, 1, 0));
        float codoAnimado = animando ? -18.0f : (jugadorEnMovimiento ? 12.0f : 0.0f);
        glm::mat4 codoDerM = giroArticulacion(codoDer, codoAnimado, glm::vec3(0, 1, 0));
        glm::mat4 codoIzqM = giroArticulacion(codoIzq, codoAnimado, glm::vec3(0, 1, 0));
        glm::mat4 caderaDerM = giroArticulacion(caderaJugadorDer, caderaDer, glm::vec3(0, 0, 1));
        glm::mat4 caderaIzqM = giroArticulacion(caderaJugadorIzq, caderaIzq, glm::vec3(0, 0, 1));
        glm::mat4 rodillaDerM = giroArticulacion(rodillaJugadorDer, rodillaDer, glm::vec3(0, 0, 1));
        glm::mat4 rodillaIzqM = giroArticulacion(rodillaJugadorIzq, rodillaIzq, glm::vec3(0, 0, 1));
        glm::mat4 tobilloDerM = giroArticulacion(tobilloDerJugador, tobilloDer, glm::vec3(0, 0, 1));
        glm::mat4 tobilloIzqM = giroArticulacion(tobilloIzqJugador, tobilloIzq, glm::vec3(0, 0, 1));
        // Interpola el ciclo de carrera y aplica angulos a hombros, caderas y rodillas.
        auto dibujaJugadorCorriendo = [&](const glm::mat4& raiz, const glm::vec3& uniforme, GLuint texturaUniforme, float fase, float impulso) {
            modelos[CUERPO_JUGADOR].dibujarConMatriz(programaModelos, vista, proy, raiz, reflectores, uniforme, true, texturaUniforme);
            const int cantidadPoses = (int)(sizeof(keyframesCarrera) / sizeof(keyframesCarrera[0]));
            // El modulo convierte el tiempo continuo en una fase ciclica entre 0 y 1.
            // En juego libre uso el reloj real para que tambien se animen los
            // jugadores que corren a interceptar; en la jugada uso su propio tiempo.
            float relojCarrera = (balonLibre && balonEnMovimiento) ? (float)ahora : tiempoAnimacion;
            float faseCiclo = std::fmod(relojCarrera * 1.35f + fase / (2.0f * PI), 1.0f);
            if (faseCiclo < 0.0f) faseCiclo += 1.0f;
            float muestra = faseCiclo * cantidadPoses;
            int tramo = (int)std::floor(muestra) % cantidadPoses;
            float u = muestra - std::floor(muestra);
            // Catmull-Rom suaviza el paso entre poses usando puntos vecinos.
            auto catmullRom = [](float a, float b, float c, float d, float t) {
                float t2 = t * t, t3 = t2 * t;
                return 0.5f * ((2.0f * b) + (-a + c) * t +
                    (2.0f * a - 5.0f * b + 4.0f * c - d) * t2 +
                    (-a + 3.0f * b - 3.0f * c + d) * t3);
                };
            // Evalua un canal de la pose (rodilla, paso o brazo) en esta fase.
            auto poseCarrera = [&](float KeyframeCarrera::* canal) {
                int i0 = (tramo + cantidadPoses - 1) % cantidadPoses;
                int i1 = tramo;
                int i2 = (tramo + 1) % cantidadPoses;
                int i3 = (tramo + 2) % cantidadPoses;
                return catmullRom(keyframesCarrera[i0].*canal, keyframesCarrera[i1].*canal,
                    keyframesCarrera[i2].*canal, keyframesCarrera[i3].*canal, u);
                };
            float paso = poseCarrera(&KeyframeCarrera::paso);
            float rodillaD = glm::clamp(poseCarrera(&KeyframeCarrera::rodillaDer), 0.0f, 30.0f);
            float rodillaI = glm::clamp(poseCarrera(&KeyframeCarrera::rodillaIzq), 0.0f, 30.0f);
            float brazosCarrera = glm::clamp(poseCarrera(&KeyframeCarrera::brazos), -12.0f, 12.0f);
            glm::mat4 caderaD = giroArticulacion(caderaJugadorDer, -paso - impulso, glm::vec3(1, 0, 0));
            glm::mat4 caderaI = giroArticulacion(caderaJugadorIzq, paso, glm::vec3(1, 0, 0));
            glm::mat4 rodillaDerM = giroArticulacion(rodillaJugadorDer, rodillaD, glm::vec3(1, 0, 0));
            glm::mat4 rodillaIzqM = giroArticulacion(rodillaJugadorIzq, rodillaI, glm::vec3(1, 0, 0));
            glm::mat4 brazoD = giroArticulacion(hombroDer, brazosCarrera, glm::vec3(1, 0, 0));
            glm::mat4 brazoI = giroArticulacion(hombroIzq, -brazosCarrera, glm::vec3(1, 0, 0));
            glm::mat4 codoD = giroArticulacion(codoDer, -24.0f, glm::vec3(1, 0, 0));
            glm::mat4 codoI = giroArticulacion(codoIzq, -24.0f, glm::vec3(1, 0, 0));
            modelos[BRAZO_SUP_DER].dibujarConMatriz(programaModelos, vista, proy, raiz * brazoD, reflectores, uniforme, true, texturaUniforme);
            modelos[BRAZO_INF_DER].dibujarConMatriz(programaModelos, vista, proy, raiz * brazoD * codoD, reflectores, uniforme, true, texturaUniforme);
            modelos[BRAZO_SUP_IZQ].dibujarConMatriz(programaModelos, vista, proy, raiz * brazoI, reflectores, uniforme, true, texturaUniforme);
            modelos[BRAZO_INF_IZQ].dibujarConMatriz(programaModelos, vista, proy, raiz * brazoI * codoI, reflectores, uniforme, true, texturaUniforme);
            modelos[MUSLO_DER].dibujarConMatriz(programaModelos, vista, proy, raiz * caderaD, reflectores, uniforme, true, texturaUniforme);
            modelos[PANTORRILLA_DER].dibujarConMatriz(programaModelos, vista, proy, raiz * caderaD * rodillaDerM, reflectores, uniforme, true, texturaUniforme);
            modelos[PIE_DER].dibujarConMatriz(programaModelos, vista, proy, raiz * caderaD * rodillaDerM, reflectores, uniforme, true, texturaUniforme);
            modelos[MUSLO_IZQ].dibujarConMatriz(programaModelos, vista, proy, raiz * caderaI, reflectores, uniforme, true, texturaUniforme);
            modelos[PANTORRILLA_IZQ].dibujarConMatriz(programaModelos, vista, proy, raiz * caderaI * rodillaIzqM, reflectores, uniforme, true, texturaUniforme);
            modelos[PIE_IZQ].dibujarConMatriz(programaModelos, vista, proy, raiz * caderaI * rodillaIzqM, reflectores, uniforme, true, texturaUniforme);
            };
        // El portero reacciona con un paso corto, inclinacion de torso y brazos
        // extendidos sin desplazar el cuerpo fuera de la porteria.
        auto dibujaPorteroAtajada = [&](const glm::mat4& raiz, const glm::vec3& uniforme, GLuint texturaUniforme, float ladoLocal, float intensidad) {
            // El portero se mantiene de pie, con una inclinacion corta y los brazos extendidos.
            glm::mat4 torso = raiz * giroArticulacion(glm::vec3(-1.25f, 1.75f, 0.0f), ladoLocal * 9.0f * intensidad, glm::vec3(0, 0, 1));
            modelos[CUERPO_JUGADOR].dibujarConMatriz(programaModelos, vista, proy, torso, reflectores, uniforme, true, texturaUniforme);
            glm::mat4 brazoD = giroArticulacion(hombroDer, ladoLocal * 58.0f * intensidad, glm::vec3(0, 0, 1));
            glm::mat4 brazoI = giroArticulacion(hombroIzq, ladoLocal * 58.0f * intensidad, glm::vec3(0, 0, 1));
            glm::mat4 codoD = giroArticulacion(codoDer, -12.0f * intensidad, glm::vec3(0, 0, 1));
            glm::mat4 codoI = giroArticulacion(codoIzq, 12.0f * intensidad, glm::vec3(0, 0, 1));
            modelos[BRAZO_SUP_DER].dibujarConMatriz(programaModelos, vista, proy, torso * brazoD, reflectores, uniforme, true, texturaUniforme);
            modelos[BRAZO_INF_DER].dibujarConMatriz(programaModelos, vista, proy, torso * brazoD * codoD, reflectores, uniforme, true, texturaUniforme);
            modelos[BRAZO_SUP_IZQ].dibujarConMatriz(programaModelos, vista, proy, torso * brazoI, reflectores, uniforme, true, texturaUniforme);
            modelos[BRAZO_INF_IZQ].dibujarConMatriz(programaModelos, vista, proy, torso * brazoI * codoI, reflectores, uniforme, true, texturaUniforme);
            // Piernas rectas y pies sobre el suelo: la atajada no convierte al portero en una zambullida.
            modelos[MUSLO_DER].dibujarConMatriz(programaModelos, vista, proy, raiz, reflectores, uniforme, true, texturaUniforme);
            modelos[PANTORRILLA_DER].dibujarConMatriz(programaModelos, vista, proy, raiz, reflectores, uniforme, true, texturaUniforme);
            modelos[PIE_DER].dibujarConMatriz(programaModelos, vista, proy, raiz, reflectores, uniforme, true, texturaUniforme);
            modelos[MUSLO_IZQ].dibujarConMatriz(programaModelos, vista, proy, raiz, reflectores, uniforme, true, texturaUniforme);
            modelos[PANTORRILLA_IZQ].dibujarConMatriz(programaModelos, vista, proy, raiz, reflectores, uniforme, true, texturaUniforme);
            modelos[PIE_IZQ].dibujarConMatriz(programaModelos, vista, proy, raiz, reflectores, uniforme, true, texturaUniforme);
            };
        // Rig adicional para suplentes sentados: muslos horizontales y pantorrillas hacia abajo.
        // Pose de suplente sentado: torso erguido y piernas flexionadas desde cadera/rodilla.
        auto dibujaJugadorSentado = [&](const glm::mat4& raiz, const glm::vec3& uniforme, GLuint texturaUniforme, float anguloCadera) {
            modelos[CUERPO_JUGADOR].dibujarConMatriz(programaModelos, vista, proy, raiz, reflectores, uniforme, true, texturaUniforme);
            glm::mat4 hombroD = giroArticulacion(hombroDer, 10.0f, glm::vec3(0, 0, 1));
            glm::mat4 hombroI = giroArticulacion(hombroIzq, -10.0f, glm::vec3(0, 0, 1));
            glm::mat4 codoD = giroArticulacion(codoDer, 28.0f, glm::vec3(0, 0, 1));
            glm::mat4 codoI = giroArticulacion(codoIzq, -28.0f, glm::vec3(0, 0, 1));
            modelos[BRAZO_SUP_DER].dibujarConMatriz(programaModelos, vista, proy, raiz * hombroD, reflectores, uniforme, true, texturaUniforme);
            modelos[BRAZO_INF_DER].dibujarConMatriz(programaModelos, vista, proy, raiz * hombroD * codoD, reflectores, uniforme, true, texturaUniforme);
            modelos[BRAZO_SUP_IZQ].dibujarConMatriz(programaModelos, vista, proy, raiz * hombroI, reflectores, uniforme, true, texturaUniforme);
            modelos[BRAZO_INF_IZQ].dibujarConMatriz(programaModelos, vista, proy, raiz * hombroI * codoI, reflectores, uniforme, true, texturaUniforme);
            // El eje X flexiona muslos y pantorrillas hacia delante (eje Z local),
            // siguiendo la orientación del torso; la rodilla compensa el ángulo de cadera
            // para dejar las pantorrillas apuntando hacia abajo.
            glm::mat4 caderaD = giroArticulacion(caderaJugadorDer, anguloCadera, glm::vec3(1, 0, 0));
            glm::mat4 caderaI = giroArticulacion(caderaJugadorIzq, anguloCadera, glm::vec3(1, 0, 0));
            glm::mat4 rodillaD = giroArticulacion(rodillaJugadorDer, -anguloCadera, glm::vec3(1, 0, 0));
            glm::mat4 rodillaI = giroArticulacion(rodillaJugadorIzq, -anguloCadera, glm::vec3(1, 0, 0));
            glm::mat4 tobilloD = giroArticulacion(tobilloDerJugador, 0.0f, glm::vec3(0, 0, 1));
            glm::mat4 tobilloI = giroArticulacion(tobilloIzqJugador, 0.0f, glm::vec3(0, 0, 1));
            modelos[MUSLO_DER].dibujarConMatriz(programaModelos, vista, proy, raiz * caderaD, reflectores, uniforme, true, texturaUniforme);
            modelos[PANTORRILLA_DER].dibujarConMatriz(programaModelos, vista, proy, raiz * caderaD * rodillaD, reflectores, uniforme, true, texturaUniforme);
            modelos[PIE_DER].dibujarConMatriz(programaModelos, vista, proy, raiz * caderaD * rodillaD * tobilloD, reflectores, uniforme, true, texturaUniforme);
            modelos[MUSLO_IZQ].dibujarConMatriz(programaModelos, vista, proy, raiz * caderaI, reflectores, uniforme, true, texturaUniforme);
            modelos[PANTORRILLA_IZQ].dibujarConMatriz(programaModelos, vista, proy, raiz * caderaI * rodillaI, reflectores, uniforme, true, texturaUniforme);
            modelos[PIE_IZQ].dibujarConMatriz(programaModelos, vista, proy, raiz * caderaI * rodillaI * tobilloI, reflectores, uniforme, true, texturaUniforme);
            };
        if (!primeraPersona) {
            modelos[CUERPO_JUGADOR].dibujarConMatriz(programaModelos, vista, proy, raizJugador, reflectores, colorAtlas, true, texturaAtlas);
            modelos[BRAZO_SUP_DER].dibujarConMatriz(programaModelos, vista, proy, raizJugador * hombroDerM, reflectores, colorAtlas, true, texturaAtlas);
            modelos[BRAZO_INF_DER].dibujarConMatriz(programaModelos, vista, proy, raizJugador * hombroDerM * codoDerM, reflectores, colorAtlas, true, texturaAtlas);
            modelos[BRAZO_SUP_IZQ].dibujarConMatriz(programaModelos, vista, proy, raizJugador * hombroIzqM, reflectores, colorAtlas, true, texturaAtlas);
            modelos[BRAZO_INF_IZQ].dibujarConMatriz(programaModelos, vista, proy, raizJugador * hombroIzqM * codoIzqM, reflectores, colorAtlas, true, texturaAtlas);
            modelos[MUSLO_DER].dibujarConMatriz(programaModelos, vista, proy, raizJugador * caderaDerM, reflectores, colorAtlas, true, texturaAtlas);
            modelos[PANTORRILLA_DER].dibujarConMatriz(programaModelos, vista, proy, raizJugador * caderaDerM * rodillaDerM, reflectores, colorAtlas, true, texturaAtlas);
            modelos[PIE_DER].dibujarConMatriz(programaModelos, vista, proy, raizJugador * caderaDerM * rodillaDerM * tobilloDerM, reflectores, colorAtlas, true, texturaAtlas);
            modelos[MUSLO_IZQ].dibujarConMatriz(programaModelos, vista, proy, raizJugador * caderaIzqM, reflectores, colorAtlas, true, texturaAtlas);
            modelos[PANTORRILLA_IZQ].dibujarConMatriz(programaModelos, vista, proy, raizJugador * caderaIzqM * rodillaIzqM, reflectores, colorAtlas, true, texturaAtlas);
            modelos[PIE_IZQ].dibujarConMatriz(programaModelos, vista, proy, raizJugador * caderaIzqM * rodillaIzqM * tobilloIzqM, reflectores, colorAtlas, true, texturaAtlas);
        }
        // Posiciones base de los equipos en el plano XZ. El personaje manejado
        // por el usuario se suma a estos diez jugadores de campo.
        const glm::vec2 equipoRojo[10] = { glm::vec2(30,0),glm::vec2(18,-18),glm::vec2(18,-6),glm::vec2(18,6),glm::vec2(18,18),glm::vec2(5,-13),glm::vec2(5,0),glm::vec2(5,13),glm::vec2(-8,-13),glm::vec2(-8,13) };
        const glm::vec2 equipoBlanco[11] = { glm::vec2(-30,0),glm::vec2(-18,-18),glm::vec2(-18,-6),glm::vec2(-18,6),glm::vec2(-18,18),glm::vec2(-5,-13),glm::vec2(-5,0),glm::vec2(-5,13),glm::vec2(8,-13),glm::vec2(8,0),glm::vec2(8,13) };
        // Conservo posiciones propias para los jugadores de campo y asi moverlos
        // progresivamente hacia el balon sin teletransportarlos en cada cuadro.
        static glm::vec3 posicionesIArojo[10], posicionesIAblanco[11];
        static bool posicionesIAInicializadas = false;
        if (!posicionesIAInicializadas) {
            for (int i = 0; i < 10; ++i) posicionesIArojo[i] = glm::vec3(equipoRojo[i].x, 0.0f, equipoRojo[i].y);
            for (int i = 0; i < 11; ++i) posicionesIAblanco[i] = glm::vec3(equipoBlanco[i].x, 0.0f, equipoBlanco[i].y);
            posicionesIAInicializadas = true;
        }
        // Los jugadores miran un poco por delante de la pelota para anticipar
        // su recorrido y converger desde distintos carriles.
        glm::vec3 objetivoIntercepcion = posicionBalonManual;
        if (balonEnMovimiento) objetivoIntercepcion += velocidadBalonManual * 0.22f;
        objetivoIntercepcion.x = glm::clamp(objetivoIntercepcion.x, -31.0f, 31.0f);
        objetivoIntercepcion.z = glm::clamp(objetivoIntercepcion.z, -19.0f, 19.0f);
        auto rangoCercania = [&](int indice, glm::vec3* posiciones, int cantidad) {
            glm::vec2 objetivoXZ(objetivoIntercepcion.x, objetivoIntercepcion.z);
            glm::vec2 jugadorXZ(posiciones[indice].x, posiciones[indice].z);
            float distancia = glm::length(jugadorXZ - objetivoXZ);
            int masCercanos = 0;
            for (int j = 1; j < cantidad; ++j) {
                if (j == indice) continue;
                glm::vec2 otroXZ(posiciones[j].x, posiciones[j].z);
                if (glm::length(otroXZ - objetivoXZ) < distancia) ++masCercanos;
            }
            return masCercanos;
            };
        auto avanzarIA = [&](glm::vec3& actual, glm::vec3 destino, float velocidad) {
            glm::vec2 diferencia(destino.x - actual.x, destino.z - actual.z);
            float distancia = glm::length(diferencia);
            if (distancia < 0.08f) return false;
            float paso = glm::min(distancia, velocidad * dt);
            actual.x += diferencia.x / distancia * paso;
            actual.z += diferencia.y / distancia * paso;
            return true;
            };
        // Suaviza el inicio y final de cada desplazamiento para evitar cambios instantaneos.
        auto progresoJugada = [](float a, float b, float t) { float u = glm::clamp((t - a) / (b - a), 0.0f, 1.0f); return u * u * (3.0f - 2.0f * u); };
        bool escenaJugada = animando || jugadaTerminada;
        float tiempoJugada = tiempoAnimacion;
        // Durante un tiro manual, uso su velocidad real para orientar al portero;
        // en la jugada predefinida uso la direccion calculada hacia la porteria.
        glm::vec3 direccionAtaqueActual = (balonLibre && balonEnMovimiento)
            ? glm::normalize(glm::vec3(velocidadBalonManual.x, 0.0f, velocidadBalonManual.z))
            : direccionTiro;
        float rumboAtaque = glm::degrees(std::atan2(direccionAtaqueActual.x, direccionAtaqueActual.z));
        glm::vec3 inicioCompaA(equipoRojo[8].x, 0, equipoRojo[8].y), inicioCompaB(equipoRojo[9].x, 0, equipoRojo[9].y);
        // Puntos de recepcion y remate calculados en relacion con la direccion
        // del ataque, para que la misma jugada funcione hacia cualquiera de las porterias.
        glm::vec3 recibeA = inicioJugada + direccionTiro * (distanciaJugada * 0.20f) + lateralJugada * (8.0f);
        glm::vec3 recibeB = inicioJugada + direccionTiro * (distanciaJugada * 0.48f) + lateralJugada * (-4.5f);
        glm::vec3 remate = inicioJugada + direccionTiro * (distanciaJugada * 0.86f);
        // Se queda dentro de la boca del arco y da un paso corto hacia la trayectoria del balon.
        glm::vec3 porteroReacciona = objetivoJugada - direccionTiro * 1.7f - lateralJugada * 1.0f;
        // Actualizo primero el equipo atacante: apoyos, receptores y delantero.
        for (size_t i = 0;i < 10;++i) {
            glm::vec3 posicion(equipoRojo[i].x, 0, equipoRojo[i].y); bool corre = false, porteroAtaja = false; float fase = 0.0f, impulso = 0.0f, intensidadAtajada = 0.0f;
            // Solo los dos jugadores de campo mas cercanos corren a interceptar.
            // Los demas se mantienen en su formacion y no se amontonan alrededor del balon.
            if (!escenaJugada && i > 0 && balonLibre) {
                int rango = rangoCercania((int)i, posicionesIArojo, 10);
                if (rango < 2) {
                    glm::vec3 destino = objetivoIntercepcion;
                    destino.z += (i % 2 == 0) ? -2.6f : 2.6f;
                    destino.z = glm::clamp(destino.z, -20.0f, 20.0f);
                    corre = avanzarIA(posicionesIArojo[i], destino, 9.0f);
                }
                else corre = avanzarIA(posicionesIArojo[i], posicion, 5.5f);
                posicion = posicionesIArojo[i]; fase = (i % 2 == 0) ? 0.0f : PI;
            }
            else if (!escenaJugada && i > 0) {
                corre = avanzarIA(posicionesIArojo[i], posicion, 5.5f);
                posicion = posicionesIArojo[i]; fase = (i % 2 == 0) ? 0.0f : PI;
            }
            if (escenaJugada && (i == 5 || i == 6 || i == 7)) {
                glm::vec3 inicioApoyo(equipoRojo[i].x, 0, equipoRojo[i].y);
                float avanceApoyo = (i == 6 ? 5.0f : 3.5f);
                float carril = (i == 5 ? -2.5f : (i == 7 ? 2.5f : 0.0f));
                glm::vec3 destinoApoyo = inicioApoyo + direccionTiro * avanceApoyo + lateralJugada * carril;
                posicion = glm::mix(inicioApoyo, destinoApoyo, progresoJugada(0.75f, 3.0f, tiempoJugada));
                corre = tiempoJugada > 0.75f && tiempoJugada < 3.0f; fase = (i == 6 ? 0.0f : PI);
            }
            if (escenaJugada && i == 8) {
                posicion = glm::mix(inicioCompaA, recibeA, progresoJugada(0.20f, 1.20f, tiempoJugada));
                if (tiempoJugada > 1.55f) posicion = glm::mix(recibeA, recibeA + direccionTiro * 2.0f, progresoJugada(1.55f, 2.65f, tiempoJugada));
                corre = (tiempoJugada > 0.20f && tiempoJugada < 1.20f) || (tiempoJugada > 1.55f && tiempoJugada < 2.65f); fase = 0.0f;
                impulso = 38.0f * progresoJugada(1.48f, 1.58f, tiempoJugada) * (1.0f - progresoJugada(1.58f, 1.92f, tiempoJugada));
            }
            if (escenaJugada && i == 9) {
                posicion = glm::mix(inicioCompaB, recibeB, progresoJugada(0.75f, 2.60f, tiempoJugada));
                if (tiempoJugada > 2.80f) posicion = glm::mix(recibeB, remate, progresoJugada(2.80f, 4.65f, tiempoJugada));
                corre = (tiempoJugada > 0.75f && tiempoJugada < 2.60f) || (tiempoJugada > 2.80f && tiempoJugada < 4.65f); fase = PI;
                impulso = 42.0f * progresoJugada(4.62f, 4.78f, tiempoJugada) * (1.0f - progresoJugada(4.78f, 5.08f, tiempoJugada));
            }
            if (escenaJugada && objetivoJugada.x > 0.0f && i == 0) {
                float avanceAtajada = progresoJugada(4.10f, 4.78f, tiempoJugada);
                float recuperacion = progresoJugada(5.35f, 6.05f, tiempoJugada);
                posicion = glm::mix(posicion, porteroReacciona, 0.78f * avanceAtajada * (1.0f - recuperacion));
                intensidadAtajada = progresoJugada(4.12f, 4.82f, tiempoJugada) * (1.0f - progresoJugada(5.35f, 6.00f, tiempoJugada));
                porteroAtaja = tiempoJugada >= 4.10f && tiempoJugada < 6.05f;
                corre = porteroAtaja; fase = PI * 0.5f;
            }
            // El portero de la porteria positiva se desplaza hacia el tiro manual.
            if (i == 0 && ((balonEnMovimiento && velocidadBalonManual.x > 0.0f && posicionBalonManual.x > 21.0f) ||
                (tiempoReaccionPortero > 0.0f && ladoPorteroReacciona > 0))) {
                float reaccion = tiempoReaccionPortero > 0.0f ? 1.0f : glm::clamp((posicionBalonManual.x - 21.0f) / 10.0f, 0.0f, 1.0f);
                posicion.z = glm::mix(posicion.z, glm::clamp(posicionBalonManual.z, -2.2f, 2.2f), reaccion);
                intensidadAtajada = reaccion;
                porteroAtaja = true;
            }
            float orientacion = ((escenaJugada && (corre || i == 8 || i == 9)) || (balonLibre && corre)) ? rumboAtaque : -90.0f;
            if (porteroAtaja) orientacion = rumboAtaque + 180.0f;
            glm::mat4 raiz = crearRaizJugador(posicion, orientacion);
            if (porteroAtaja) dibujaPorteroAtajada(raiz, colorAtlas, texturaAtlas, 1.0f, intensidadAtajada);
            else if (corre) dibujaJugadorCorriendo(raiz, colorAtlas, texturaAtlas, fase, impulso);
            else dibujaJugadorCompleto(raiz, colorAtlas, texturaAtlas);
        }
        // El equipo defensor cubre carriles; su portero activa la pose de atajada.
        for (size_t i = 0;i < 11;++i) {
            glm::vec3 posicion(equipoBlanco[i].x, 0, equipoBlanco[i].y); bool corre = false, porteroAtaja = false; float intensidadAtajada = 0.0f;
            // Del equipo rival tambien salen solo los dos mas cercanos al punto
            // previsto. El resto conserva su posicion para mantener la formacion.
            if (!escenaJugada && i > 0 && balonLibre) {
                int rango = rangoCercania((int)i, posicionesIAblanco, 11);
                if (rango < 2) {
                    glm::vec3 destino = objetivoIntercepcion;
                    destino.z += (i % 2 == 0) ? -2.6f : 2.6f;
                    destino.z = glm::clamp(destino.z, -20.0f, 20.0f);
                    corre = avanzarIA(posicionesIAblanco[i], destino, 9.5f);
                }
                else corre = avanzarIA(posicionesIAblanco[i], posicion, 5.5f);
                posicion = posicionesIAblanco[i];
            }
            else if (!escenaJugada && i > 0) {
                corre = avanzarIA(posicionesIAblanco[i], posicion, 5.5f);
                posicion = posicionesIAblanco[i];
            }
            if (escenaJugada && (i == 2 || i == 3 || i == 4)) {
                glm::vec3 inicioDefensa(equipoBlanco[i].x, 0, equipoBlanco[i].y);
                float coberturaLateral = (i == 2 ? -2.0f : (i == 3 ? -3.5f : 2.0f));
                glm::vec3 objetivoDefensa = inicioDefensa + direccionTiro * 2.5f + lateralJugada * coberturaLateral;
                posicion = glm::mix(inicioDefensa, objetivoDefensa, progresoJugada(1.05f, 3.35f, tiempoJugada));
                corre = tiempoJugada > 1.05f && tiempoJugada < 3.35f;
            }
            if (escenaJugada && objetivoJugada.x < 0.0f && i == 0) {
                float avanceAtajada = progresoJugada(4.10f, 4.78f, tiempoJugada);
                float recuperacion = progresoJugada(5.35f, 6.05f, tiempoJugada);
                posicion = glm::mix(posicion, porteroReacciona, 0.78f * avanceAtajada * (1.0f - recuperacion));
                intensidadAtajada = progresoJugada(4.12f, 4.82f, tiempoJugada) * (1.0f - progresoJugada(5.35f, 6.00f, tiempoJugada));
                porteroAtaja = tiempoJugada >= 4.10f && tiempoJugada < 6.05f;
                corre = porteroAtaja;
            }
            // El portero de la porteria negativa responde de la misma manera.
            if (i == 0 && ((balonEnMovimiento && velocidadBalonManual.x < 0.0f && posicionBalonManual.x < -21.0f) ||
                (tiempoReaccionPortero > 0.0f && ladoPorteroReacciona < 0))) {
                float reaccion = tiempoReaccionPortero > 0.0f ? 1.0f : glm::clamp((-posicionBalonManual.x - 21.0f) / 10.0f, 0.0f, 1.0f);
                posicion.z = glm::mix(posicion.z, glm::clamp(posicionBalonManual.z, -2.2f, 2.2f), reaccion);
                intensidadAtajada = reaccion;
                porteroAtaja = true;
            }
            float orientacion = (corre || (escenaJugada && i == 0)) ? rumboAtaque : 90.0f;
            if (porteroAtaja) orientacion = rumboAtaque + 180.0f;
            glm::mat4 raiz = crearRaizJugador(posicion, orientacion);
            if (porteroAtaja) dibujaPorteroAtajada(raiz, colorCruzAzul, texturaCruzAzul, 1.0f, intensidadAtajada);
            else if (corre) dibujaJugadorCorriendo(raiz, colorCruzAzul, texturaCruzAzul, PI * 0.5f, 0.0f);
            else dibujaJugadorCompleto(raiz, colorCruzAzul, texturaCruzAzul);
        }
        // Los cuatro suplentes quedan dentro del largo util de cada banca.
        // Distribucion simetrica de cuatro suplentes por cada banca.
        const float asientosX[4] = { -2.4f,-0.8f,0.8f,2.4f };
        for (float x : asientosX) {
            dibujaJugadorSentado(crearRaizJugador(glm::vec3(x, -0.25f, -27.0f), 0.0f), colorAtlas, texturaAtlas, -65.0f);
            dibujaJugadorSentado(crearRaizJugador(glm::vec3(x, -0.25f, 27.0f), 180.0f), colorCruzAzul, texturaCruzAzul, -65.0f);
        }
        // El técnico se coloca delante del grupo, entre la banca y la cancha, y mira al terreno.
        // Se usa la geometría original tintada; no se agregan cubos por encima del personaje.
        const glm::vec3 colorTecnico(0.10f, 0.13f, 0.20f);
        auto dibujaEntrenador = [&](const glm::mat4& raiz) {
            dibujaJugadorCompleto(raiz, colorTecnico, texturaAtlas);
            };
        dibujaEntrenador(crearRaizJugador(glm::vec3(0.0f, 0.0f, -25.2f), 0.0f));
        dibujaEntrenador(crearRaizJugador(glm::vec3(0.0f, 0.0f, 25.2f), 180.0f));
        // El arbitro se conserva como modelo completo en el centro de la escena.
        modelos[10].dibujar(programaModelos, vista, proy, arbitro, 3.2f, 180.0f, reflectores);
        glUseProgram(programa);
        // Marcador de siete segmentos: G suma local, H suma visita, R reinicia.
        const int segmentos[10][7] = { {1,1,1,1,1,1,0},{0,1,1,0,0,0,0},{1,1,0,1,1,0,1},{1,1,1,1,0,0,1},{0,1,1,0,0,1,1},{1,0,1,1,0,1,1},{1,0,1,1,1,1,1},{1,1,1,0,0,0,0},{1,1,1,1,1,1,1},{1,1,1,1,0,1,1} };
        // En el marcador fisico cada digito se construye con hasta siete barras.
        auto digito = [&](int valor, float z) {
            const float xx[7] = { 0.0f,0.78f,0.78f,0.0f,-0.78f,-0.78f,0.0f };
            const float yy[7] = { 1.0f,0.5f,-0.5f,-1.0f,-0.5f,0.5f,0.0f };
            for (int i = 0;i < 7;++i) if (segmentos[valor % 10][i]) {
                bool horizontal = (i == 0 || i == 3 || i == 6);
                dibujaParte(glm::vec3(-61.15f, 18.0f + yy[i], z + xx[i]), horizontal ? glm::vec3(0.18f, 0.18f, 1.40f) : glm::vec3(0.18f, 0.90f, 0.18f), glm::vec3(1.0f, 0.16f, 0.06f), glm::mat4(1));
            }
            };
        digito(golesLocal, -2.6f); digito(golesVisita, 2.6f);
        dibujaParte(glm::vec3(-61.15f, 18.0f, 0), glm::vec3(0.18f, 0.20f, 0.18f), AMARILLO, glm::mat4(1));
        // Busco el par de keyframes que contiene el tiempo actual e interpolo
        // avance, altura y desvio para obtener la posicion continua del balon.
        float avance = keyframesBalon[0].avance, altura = keyframesBalon[0].altura, lateral = keyframesBalon[0].lateral;
        if (animando || jugadaTerminada) for (size_t k = 0;k + 1 < sizeof(keyframesBalon) / sizeof(keyframesBalon[0]);++k) if (tiempoAnimacion >= keyframesBalon[k].tiempo && tiempoAnimacion <= keyframesBalon[k + 1].tiempo) {
            float u = (tiempoAnimacion - keyframesBalon[k].tiempo) / (keyframesBalon[k + 1].tiempo - keyframesBalon[k].tiempo);
            u = u * u * (3.0f - 2.0f * u);
            avance = glm::mix(keyframesBalon[k].avance, keyframesBalon[k + 1].avance, u);
            altura = glm::mix(keyframesBalon[k].altura, keyframesBalon[k + 1].altura, u);
            lateral = glm::mix(keyframesBalon[k].lateral, keyframesBalon[k + 1].lateral, u); break;
        }
        // Antes de iniciar, el balon acompana al jugador; durante la secuencia
        // parte del punto guardado y sigue la trayectoria interpolada.
        glm::vec3 origenBalon = (animando || jugadaTerminada) ? inicioJugada : jugador;
        glm::vec3 balon = origenBalon + direccionTiro * (avance * distanciaJugada) + glm::vec3(0, altura, 0) + lateralJugada * lateral;
        // En modo de juego libre, el balon usa su posicion fisica independiente;
        // si se recupera, vuelve a acompañar al jugador.
        if (balonLibre && !animando) balon = posicionBalonManual;
        float rumboBalon = glm::degrees(std::atan2(direccionTiro.x, direccionTiro.z));
        float giroBalon = tiempoAnimacion < 5.22f ? tiempoAnimacion * 900.0f : 4698.0f + 90.0f * (1.0f - std::exp(-(tiempoAnimacion - 5.22f) * 5.0f));
        if (modelos[1].listo()) { glUseProgram(programaModelos); modelos[1].dibujar(programaModelos, vista, proy, balon, 0.48f, giroBalon + rumboBalon, reflectores); glUseProgram(programa); glBindVertexArray(cuboVao); }
        else dibujaParte(balon, glm::vec3(0.42f), glm::vec3(0.94f, 0.94f, 0.88f), glm::mat4(1));
        // Fuegos artificiales lanzados desde tres puntos sobre las gradas.
        // Cada ciclo dibuja una estela ascendente y una explosion esferica de chispas.
        std::vector<float> verticesFuegos;
        auto agregaVerticeFuego = [&](glm::vec3 p, glm::vec3 color) {
            verticesFuegos.insert(verticesFuegos.end(), { p.x,p.y,p.z,color.r,color.g,color.b });
            };
        auto agregaRayoFuego = [&](glm::vec3 a, glm::vec3 b, glm::vec3 colorA, glm::vec3 colorB) {
            agregaVerticeFuego(a, colorA); agregaVerticeFuego(b, colorB);
            };
        const glm::vec3 salidasFuegos[3] = {
            glm::vec3(-39.0f, 30.0f, -25.0f), glm::vec3(0.0f, 32.0f, 0.0f), glm::vec3(39.0f, 30.0f, 25.0f)
        };
        const glm::vec3 coloresFuegos[3] = {
            glm::vec3(1.0f,0.24f,0.06f), glm::vec3(0.10f,0.58f,1.0f), glm::vec3(0.84f,0.18f,1.0f)
        };
        // Cada cohete repite un ciclo independiente con desfasamiento temporal.
        const float tiempoFuegos = (float)glfwGetTime(), cicloFuegos = 6.0f, subidaFuego = 1.25f, duracionExplosion = 2.15f;
        for (int f = 0; f < 3; ++f) {
            float faseFuego = std::fmod(tiempoFuegos + f * 2.0f, cicloFuegos);
            glm::vec3 origen = salidasFuegos[f], color = coloresFuegos[f];
            glm::vec3 blancoCaliente = glm::mix(color, glm::vec3(1.0f, 0.96f, 0.78f), 0.62f);
            glm::vec3 centroExplosion = origen + glm::vec3(0, 18.0f, 0);
            if (faseFuego < subidaFuego) {
                float u = faseFuego / subidaFuego;
                float ascenso = u * (2.0f - u);
                glm::vec3 punta = origen + glm::vec3(0, 18.0f * ascenso, 0);
                agregaRayoFuego(punta, punta - glm::vec3(0, 2.3f, 0), blancoCaliente, color * 0.72f);
                for (int chispa = 0; chispa < 4; ++chispa) {
                    float angulo = faseFuego * 14.0f + chispa * (PI * 0.5f);
                    glm::vec3 inicio = punta + glm::vec3(cos(angulo) * 0.18f, -0.35f * chispa, sin(angulo) * 0.18f);
                    glm::vec3 fin = inicio + glm::vec3(cos(angulo) * 0.8f, -0.65f, sin(angulo) * 0.8f);
                    agregaRayoFuego(inicio, fin, blancoCaliente, color * 0.45f);
                }
            }
            else if (faseFuego < subidaFuego + duracionExplosion) {
                float edad = faseFuego - subidaFuego;
                float progreso = edad / duracionExplosion;
                float radio = 12.0f * (1.0f - (1.0f - progreso) * (1.0f - progreso));
                centroExplosion.y -= 2.5f * progreso * progreso;
                glm::vec3 brillo = glm::mix(blancoCaliente, color, 0.35f + progreso * 0.35f);
                for (int chispa = 0; chispa < 48; ++chispa) {
                    float yDir = 1.0f - 2.0f * ((float)chispa + 0.5f) / 48.0f;
                    float radioXZ = sqrtf(1.0f - yDir * yDir);
                    float angulo = chispa * 2.399963f + f * 0.7f;
                    glm::vec3 direccion(cos(angulo) * radioXZ, yDir, sin(angulo) * radioXZ);
                    glm::vec3 punta = centroExplosion + direccion * radio;
                    float largoCola = 0.45f + 1.25f * (1.0f - progreso);
                    glm::vec3 cola = punta - direccion * largoCola;
                    agregaRayoFuego(cola, punta, color * 0.68f, brillo);
                }
            }
        }
        if (!verticesFuegos.empty()) {
            // Activo el programa de geometria y restauro matrices/reflectores
            // para dibujar particulas en las mismas coordenadas que el estadio.
            glUseProgram(programa); cargarSpots(programa);
            glUniformMatrix4fv(locV, 1, GL_FALSE, glm::value_ptr(vista)); glUniformMatrix4fv(locP, 1, GL_FALSE, glm::value_ptr(proy));
            glUniformMatrix4fv(locM, 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));
            glUniform1i(locField, 2); glUniform1i(locUI, 0); glUniform1i(locLamps, reflectores ? 1 : 0);
            glBindBuffer(GL_ARRAY_BUFFER, fuegosVbo);
            // La lista cambia cada cuadro, por eso el buffer se actualiza dinamicamente.
            glBufferData(GL_ARRAY_BUFFER, verticesFuegos.size() * sizeof(float), verticesFuegos.data(), GL_DYNAMIC_DRAW);
            glBindVertexArray(fuegosVao); glLineWidth(2.0f);
            // Cada par de vertices representa una linea de estela o una chispa.
            glDrawArrays(GL_LINES, 0, (GLsizei)(verticesFuegos.size() / 6));
            glBindVertexArray(cuboVao); glUniform1i(locField, 0);
        }
        // Marcador HUD fijo para que G/H siempre produzcan una respuesta visible.
        glUseProgram(programa); glDisable(GL_DEPTH_TEST);
        // La proyeccion ortografica usa pixeles de ventana para fijar el marcador
        // a la pantalla, sin que la camara 3D cambie su tamano o posicion.
        glm::mat4 vistaHUD(1.0f), proyeccionHUD = glm::ortho(0.0f, (float)ww, 0.0f, (float)hh, -100.0f, 100.0f);
        glUniformMatrix4fv(locV, 1, GL_FALSE, glm::value_ptr(vistaHUD)); glUniformMatrix4fv(locP, 1, GL_FALSE, glm::value_ptr(proyeccionHUD));
        glUniform1i(locField, 0); glUniform1i(locLamps, 0); glUniform1i(locUI, 1);
        // Centro y altura del panel se calculan respecto al tamano actual de ventana.
        float centroHUD = ww * 0.5f, altoHUD = (float)hh - 48.0f;
        dibujaParte(glm::vec3(centroHUD, altoHUD, 0), glm::vec3(260, 72, 2), glm::vec3(0.025f, 0.045f, 0.085f), glm::mat4(1));
        // Dibuja un caracter con siete segmentos; el patron indica que segmentos encender.
        auto dibujaPatronHUD = [&](const int* patron, float cx, float cy, glm::vec3 color, float ancho, float alto, float grosor) {
            // Desplazamientos locales para los segmentos superior, laterales,
            // inferior y central, escalados segun el ancho y alto del digito.
            const float sx[7] = { 0,ancho / 2,ancho / 2,0,-ancho / 2,-ancho / 2,0 };
            const float sy[7] = { alto / 2,alto / 4,-alto / 4,-alto / 2,-alto / 4,alto / 4,0 };
            for (int i = 0;i < 7;++i) if (patron[i]) {
                bool horizontal = (i == 0 || i == 3 || i == 6);
                glm::vec3 escala = horizontal ? glm::vec3(ancho, grosor, 1) : glm::vec3(grosor, alto / 2, grosor);
                dibujaParte(glm::vec3(cx + sx[i], cy + sy[i], 0), escala, color, glm::mat4(1));
            }
            };
        // Patrones de siete segmentos usados para identificar los dos marcadores.
        const int letraG[7] = { 1,0,1,1,1,1,1 }, letraH[7] = { 0,1,1,0,1,1,1 };
        const glm::vec3 colorLocal(1.0f, 0.24f, 0.20f), colorVisita(0.78f, 0.90f, 1.0f), colorDigito(0.98f, 0.82f, 0.36f);
        // Coloco letras y digitos a cada lado del centro para formar el HUD local-visita.
        dibujaPatronHUD(letraG, centroHUD - 91, altoHUD, colorLocal, 18, 40, 4);
        dibujaPatronHUD(segmentos[golesLocal / 10], centroHUD - 47, altoHUD, colorDigito, 27, 48, 5);
        dibujaPatronHUD(segmentos[golesLocal % 10], centroHUD - 17, altoHUD, colorDigito, 27, 48, 5);
        dibujaPatronHUD(segmentos[golesVisita / 10], centroHUD + 17, altoHUD, colorDigito, 27, 48, 5);
        dibujaPatronHUD(segmentos[golesVisita % 10], centroHUD + 47, altoHUD, colorDigito, 27, 48, 5);
        dibujaPatronHUD(letraH, centroHUD + 91, altoHUD, colorVisita, 18, 40, 4);
        // Restauro el modo 3D y la prueba de profundidad para el siguiente cuadro.
        glUniform1i(locUI, 0); glEnable(GL_DEPTH_TEST);
        // Presento el cuadro ya dibujado; GLFW intercambia el buffer frontal y trasero.
        glfwSwapBuffers(window);
    }
    // Libero primero recursos pertenecientes a cada modelo cargado desde OBJ.
    for (size_t i = 0;i < modelos.size();++i) modelos[i].liberar();
    glDeleteTextures(1, &texturaCesped); if (texturaAtlas) glDeleteTextures(1, &texturaAtlas); if (texturaCruzAzul) glDeleteTextures(1, &texturaCruzAzul);
    if (texturaTifoCruz) glDeleteTextures(1, &texturaTifoCruz); if (texturaTifoPumas) glDeleteTextures(1, &texturaTifoPumas);
    // Elimino texturas, buffers, arreglos de vertices y shaders creados en OpenGL.
    glDeleteVertexArrays(1, &vao); glDeleteBuffers(1, &vbo); glDeleteVertexArrays(1, &cuboVao); glDeleteBuffers(1, &cuboVbo); glDeleteVertexArrays(1, &conosVao); glDeleteBuffers(1, &conosVbo); glDeleteVertexArrays(1, &tifoVao); glDeleteBuffers(1, &tifoVbo); glDeleteVertexArrays(1, &fuegosVao); glDeleteBuffers(1, &fuegosVbo); glDeleteProgram(programa); glDeleteProgram(programaModelos);
    glfwDestroyWindow(window); glfwTerminate(); return 0;
}
