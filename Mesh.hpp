#ifndef Mesh_hpp
#define Mesh_hpp

#if defined (__APPLE__)
    #define GL_SILENCE_DEPRECATION
    #include <OpenGL/gl3.h>
#else
    #define GLEW_STATIC
    #include <C:\Users\pante\Desktop\An3_sem1\PG\Biblioteci\OpenGL dev libs\include\GL/glew.h>
#endif

#include <C:\Users\pante\Desktop\An3_sem1\PG\Biblioteci\glm/glm.hpp>

#include "Shader.hpp"

#include <string>
#include <vector>


namespace gps {

    struct Vertex {

        glm::vec3 Position;
        glm::vec3 Normal;
        glm::vec2 TexCoords;
    };

    struct Texture {

        GLuint id;
        std::string type;
        std::string path;
    };

    struct Material {

        glm::vec3 ambient;
        glm::vec3 diffuse;
        glm::vec3 specular;
    };

    struct Buffers {
        GLuint VAO;
        GLuint VBO;
        GLuint EBO;
    };

    class Mesh {

    public:
        std::vector<Vertex> vertices;
        std::vector<GLuint> indices;
        std::vector<Texture> textures;

	    Mesh(std::vector<Vertex> vertices, std::vector<GLuint> indices, std::vector<Texture> textures);

	    Buffers getBuffers();

	    void Draw(gps::Shader shader);

    private:
        
        Buffers buffers;
	    void setupMesh();

    };

}
#endif /* Mesh_hpp */
