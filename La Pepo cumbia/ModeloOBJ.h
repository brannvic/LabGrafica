#pragma once

#include <algorithm>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "SOIL2/SOIL2.h"

struct VerticeOBJ {
    glm::vec3 posicion;
    glm::vec3 normal;
    glm::vec2 uv;
};

struct ParteOBJ {
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint textura = 0;
    GLsizei vertices = 0;
    glm::vec3 color = glm::vec3(0.8f);
};

// Carga Wavefront OBJ/MTL sin Assimp para mantener el ejecutable compatible
// con las bibliotecas de 32 y 64 bits que ya contiene el proyecto.
class ModeloOBJ {
public:
    ModeloOBJ() : centro(0.0f), extensionMaxima(1.0f), cargado(false) {}

    bool cargar(const std::string& ruta) {
        std::ifstream archivo(ruta.c_str());
        if (!archivo.is_open()) {
            std::cerr << "No se pudo abrir el OBJ: " << ruta << std::endl;
            return false;
        }

        const std::string carpeta = directorio(ruta);
        std::vector<glm::vec3> posiciones, normales;
        std::vector<glm::vec2> coordenadas;
        std::map<std::string, std::vector<VerticeOBJ> > grupos;
        std::vector<std::string> ordenGrupos;
        std::map<std::string, size_t> indicesGrupo;
        std::string materialActual = "default", archivoMaterial;
        glm::vec3 minimo(1.0e30f), maximo(-1.0e30f);

        auto grupoActual = [&]() -> std::vector<VerticeOBJ>& {
            if (indicesGrupo.find(materialActual) == indicesGrupo.end()) {
                indicesGrupo[materialActual] = ordenGrupos.size();
                ordenGrupos.push_back(materialActual);
                grupos[materialActual] = std::vector<VerticeOBJ>();
            }
            return grupos[materialActual];
        };

        std::string linea;
        while (std::getline(archivo, linea)) {
            std::istringstream stream(linea);
            std::string tipo;
            stream >> tipo;
            if (tipo == "v") {
                glm::vec3 p; stream >> p.x >> p.y >> p.z; posiciones.push_back(p);
            } else if (tipo == "vt") {
                glm::vec2 uv; stream >> uv.x >> uv.y; coordenadas.push_back(uv);
            } else if (tipo == "vn") {
                glm::vec3 n; stream >> n.x >> n.y >> n.z; normales.push_back(n);
            } else if (tipo == "mtllib") {
                std::getline(stream, archivoMaterial);
                limpiar(archivoMaterial);
            } else if (tipo == "usemtl") {
                std::getline(stream, materialActual);
                limpiar(materialActual);
            } else if (tipo == "f") {
                std::vector<VerticeOBJ> cara;
                std::string token;
                while (stream >> token) {
                    int vi = 0, ti = 0, ni = 0;
                    leerIndices(token, vi, ti, ni);
                    VerticeOBJ v;
                    v.posicion = posiciones.at(indiceOBJ(vi, posiciones.size()));
                    v.uv = (ti != 0 && !coordenadas.empty()) ? coordenadas.at(indiceOBJ(ti, coordenadas.size())) : glm::vec2(0.0f);
                    v.normal = (ni != 0 && !normales.empty()) ? normales.at(indiceOBJ(ni, normales.size())) : glm::vec3(0.0f);
                    cara.push_back(v);
                }
                if (cara.size() < 3) continue;
                std::vector<VerticeOBJ>& vertices = grupoActual();
                for (size_t i = 1; i + 1 < cara.size(); ++i) {
                    VerticeOBJ triangulo[3] = { cara[0], cara[i], cara[i + 1] };
                    glm::vec3 normal = glm::normalize(glm::cross(triangulo[1].posicion - triangulo[0].posicion,
                                                                  triangulo[2].posicion - triangulo[0].posicion));
                    for (int k = 0; k < 3; ++k) {
                        if (glm::length(triangulo[k].normal) < 0.001f) triangulo[k].normal = normal;
                        vertices.push_back(triangulo[k]);
                        minimo = glm::min(minimo, triangulo[k].posicion);
                        maximo = glm::max(maximo, triangulo[k].posicion);
                    }
                }
            }
        }
        archivo.close();

        if (ordenGrupos.empty()) {
            std::cerr << "El OBJ no contiene caras: " << ruta << std::endl;
            return false;
        }
        centro = (minimo + maximo) * 0.5f;
        glm::vec3 tamano = maximo - minimo;
        extensionMaxima = glm::max(tamano.x, glm::max(tamano.y, tamano.z));
        if (extensionMaxima < 0.0001f) extensionMaxima = 1.0f;

        std::map<std::string, glm::vec3> colores;
        std::map<std::string, std::string> mapas;
        if (!archivoMaterial.empty()) cargarMaterial(carpeta + archivoMaterial, colores, mapas);

        for (size_t i = 0; i < ordenGrupos.size(); ++i) {
            const std::string& nombre = ordenGrupos[i];
            const std::vector<VerticeOBJ>& vertices = grupos[nombre];
            if (vertices.empty()) continue;
            ParteOBJ parte;
            parte.vertices = (GLsizei)vertices.size();
            if (colores.find(nombre) != colores.end()) parte.color = colores[nombre];
            if (mapas.find(nombre) != mapas.end()) {
                std::string archivoTextura = carpeta + mapas[nombre];
                parte.textura = SOIL_load_OGL_texture(archivoTextura.c_str(), SOIL_LOAD_AUTO,
                    SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y | SOIL_FLAG_TEXTURE_REPEATS);
                if (parte.textura == 0) std::cerr << "Textura no disponible: " << archivoTextura << std::endl;
            }
            glGenVertexArrays(1, &parte.vao);
            glGenBuffers(1, &parte.vbo);
            glBindVertexArray(parte.vao);
            glBindBuffer(GL_ARRAY_BUFFER, parte.vbo);
            glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(VerticeOBJ), vertices.data(), GL_STATIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VerticeOBJ), (void*)offsetof(VerticeOBJ, posicion)); glEnableVertexAttribArray(0);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VerticeOBJ), (void*)offsetof(VerticeOBJ, normal)); glEnableVertexAttribArray(1);
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VerticeOBJ), (void*)offsetof(VerticeOBJ, uv)); glEnableVertexAttribArray(2);
            glBindVertexArray(0);
            partes.push_back(parte);
        }
        cargado = !partes.empty();
        return cargado;
    }

    void dibujar(GLuint programa, const glm::mat4& vista, const glm::mat4& proyeccion,
                 const glm::vec3& posicion, float tamanoObjetivo, float rotacionY, bool iluminacion) const {
        if (!cargado) return;
        float escala = tamanoObjetivo / extensionMaxima;
        glm::mat4 modelo(1.0f);
        modelo = glm::translate(modelo, posicion);
        modelo = glm::rotate(modelo, glm::radians(rotacionY), glm::vec3(0, 1, 0));
        modelo = glm::scale(modelo, glm::vec3(escala));
        modelo = glm::translate(modelo, -centro);
        glUniformMatrix4fv(glGetUniformLocation(programa, "model"), 1, GL_FALSE, glm::value_ptr(modelo));
        glUniformMatrix4fv(glGetUniformLocation(programa, "view"), 1, GL_FALSE, glm::value_ptr(vista));
        glUniformMatrix4fv(glGetUniformLocation(programa, "projection"), 1, GL_FALSE, glm::value_ptr(proyeccion));
        glUniform3fv(glGetUniformLocation(programa, "lightDirection"), 1, glm::value_ptr(glm::normalize(glm::vec3(-0.45f, 1.0f, 0.35f))));
        glUniform1f(glGetUniformLocation(programa, "lightStrength"), iluminacion ? 1.0f : 0.55f);
        for (size_t i = 0; i < partes.size(); ++i) {
            const ParteOBJ& parte = partes[i];
            glUniform3fv(glGetUniformLocation(programa, "materialColor"), 1, glm::value_ptr(parte.color));
            glUniform1i(glGetUniformLocation(programa, "hasTexture"), parte.textura != 0 ? 1 : 0);
            glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, parte.textura);
            glBindVertexArray(parte.vao); glDrawArrays(GL_TRIANGLES, 0, parte.vertices);
        }
        glBindVertexArray(0);
    }

    void liberar() {
        for (size_t i = 0; i < partes.size(); ++i) {
            if (partes[i].textura) glDeleteTextures(1, &partes[i].textura);
            if (partes[i].vbo) glDeleteBuffers(1, &partes[i].vbo);
            if (partes[i].vao) glDeleteVertexArrays(1, &partes[i].vao);
        }
        partes.clear(); cargado = false;
    }

    bool listo() const { return cargado; }

private:
    std::vector<ParteOBJ> partes;
    glm::vec3 centro;
    float extensionMaxima;
    bool cargado;

    static std::string directorio(const std::string& ruta) {
        size_t p = ruta.find_last_of("/\\");
        return p == std::string::npos ? std::string() : ruta.substr(0, p + 1);
    }
    static void limpiar(std::string& texto) {
        size_t a = texto.find_first_not_of(" \t\r\n"), b = texto.find_last_not_of(" \t\r\n");
        texto = (a == std::string::npos) ? std::string() : texto.substr(a, b - a + 1);
    }
    static int indiceOBJ(int indice, size_t cuenta) {
        return indice > 0 ? indice - 1 : (int)cuenta + indice;
    }
    static void leerIndices(const std::string& token, int& vi, int& ti, int& ni) {
        std::istringstream s(token); std::string part;
        if (std::getline(s, part, '/')) vi = part.empty() ? 0 : std::stoi(part);
        if (std::getline(s, part, '/')) ti = part.empty() ? 0 : std::stoi(part);
        if (std::getline(s, part, '/')) ni = part.empty() ? 0 : std::stoi(part);
    }
    static void cargarMaterial(const std::string& ruta, std::map<std::string, glm::vec3>& colores,
                               std::map<std::string, std::string>& mapas) {
        std::ifstream archivo(ruta.c_str());
        if (!archivo.is_open()) return;
        std::string linea, actual;
        while (std::getline(archivo, linea)) {
            std::istringstream s(linea); std::string tipo; s >> tipo;
            if (tipo == "newmtl") { std::getline(s, actual); limpiar(actual); colores[actual] = glm::vec3(0.8f); }
            else if (tipo == "Kd" && !actual.empty()) s >> colores[actual].r >> colores[actual].g >> colores[actual].b;
            else if (tipo == "map_Kd" && !actual.empty()) { std::getline(s, mapas[actual]); limpiar(mapas[actual]); }
        }
    }
};
