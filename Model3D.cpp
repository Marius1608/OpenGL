#include "Model3D.hpp"

namespace gps {

	void Model3D::LoadModel(std::string fileName) {

        std::string basePath = fileName.substr(0, fileName.find_last_of('/')) + "/";
		ReadOBJ(fileName, basePath);
	}

    void Model3D::LoadModel(std::string fileName, std::string basePath)	{

		ReadOBJ(fileName, basePath);
	}

	void Model3D::Draw(gps::Shader shaderProgram) {

		for (int i = 0; i < meshes.size(); i++)
			meshes[i].Draw(shaderProgram);
	}

    void Model3D::ReadOBJ(std::string fileName, std::string basePath) {
        std::cout << "Loading : " << fileName << std::endl;
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        int materialId;

        std::string err;
        bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, fileName.c_str(), basePath.c_str(), GL_TRUE);

        if (!err.empty()) {
            std::cerr << err << std::endl;
        }

        if (!ret) {
            exit(1);
        }

        std::cout << "# of shapes    : " << shapes.size() << std::endl;
        std::cout << "# of materials : " << materials.size() << std::endl;

        for (size_t s = 0; s < shapes.size(); s++) {
            std::vector<gps::Vertex> vertices;
            std::vector<GLuint> indices;
            std::vector<gps::Texture> textures;

            size_t index_offset = 0;
            for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
                int fv = shapes[s].mesh.num_face_vertices[f];

                for (size_t v = 0; v < fv; v++) {
                   
                    tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];

                    float vx = attrib.vertices[3 * idx.vertex_index + 0];
                    float vy = attrib.vertices[3 * idx.vertex_index + 1];
                    float vz = attrib.vertices[3 * idx.vertex_index + 2];
                    float nx = attrib.normals[3 * idx.normal_index + 0];
                    float ny = attrib.normals[3 * idx.normal_index + 1];
                    float nz = attrib.normals[3 * idx.normal_index + 2];
                    float tx = 0.0f;
                    float ty = 0.0f;

                    if (idx.texcoord_index != -1) {
                        tx = attrib.texcoords[2 * idx.texcoord_index + 0];
                        ty = attrib.texcoords[2 * idx.texcoord_index + 1];
                    }

                    glm::vec3 vertexPosition(vx, vy, vz);
                    glm::vec3 vertexNormal(nx, ny, nz);
                    glm::vec2 vertexTexCoords(tx, ty);

                    gps::Vertex currentVertex;
                    currentVertex.Position = vertexPosition;
                    currentVertex.Normal = vertexNormal;
                    currentVertex.TexCoords = vertexTexCoords;

                    vertices.push_back(currentVertex);
                    indices.push_back((GLuint)(index_offset + v));
                }

                index_offset += fv;
            }

            size_t a = shapes[s].mesh.material_ids.size();
            if (a > 0 && materials.size() > 0) {
                materialId = shapes[s].mesh.material_ids[0];
                if (materialId != -1) {
                    gps::Material currentMaterial;
                    currentMaterial.ambient = glm::vec3(materials[materialId].ambient[0], materials[materialId].ambient[1], materials[materialId].ambient[2]);
                    currentMaterial.diffuse = glm::vec3(materials[materialId].diffuse[0], materials[materialId].diffuse[1], materials[materialId].diffuse[2]);
                    currentMaterial.specular = glm::vec3(materials[materialId].specular[0], materials[materialId].specular[1], materials[materialId].specular[2]);

                    std::string diffuseTexturePath = materials[materialId].diffuse_texname;
                    if (!diffuseTexturePath.empty()) {
                        std::cout << "Loading diffuse texture: " << basePath + diffuseTexturePath << std::endl;
                        gps::Texture currentTexture;
                        currentTexture = LoadTexture(basePath + diffuseTexturePath, "diffuseTexture");
                        textures.push_back(currentTexture);
                    }

                    // Normal Map
                    std::string normalTexturePath = materials[materialId].bump_texname;
                    if (!normalTexturePath.empty()) {
                        std::cout << "Loading normal map: " << basePath + normalTexturePath << std::endl;
                        gps::Texture currentTexture;
                        currentTexture = LoadTexture(basePath + normalTexturePath, "normalTexture");
                        textures.push_back(currentTexture);
                    }

                    // Metallic Map
                    std::string metallicTexturePath = materials[materialId].specular_texname;
                    if (!metallicTexturePath.empty()) {
                        std::cout << "Loading metallic map: " << basePath + metallicTexturePath << std::endl;
                        gps::Texture currentTexture;
                        currentTexture = LoadTexture(basePath + metallicTexturePath, "metallicTexture");
                        textures.push_back(currentTexture);
                    }

                    // Roughness Map
                    std::string roughnessTexturePath = materials[materialId].specular_highlight_texname;
                    if (!roughnessTexturePath.empty()) {
                        std::cout << "Loading roughness map: " << basePath + roughnessTexturePath << std::endl;
                        gps::Texture currentTexture;
                        currentTexture = LoadTexture(basePath + roughnessTexturePath, "roughnessTexture");
                        textures.push_back(currentTexture);
                    }

                    // Ambient Occlusion Map (optional)
                    std::string ambientTexturePath = materials[materialId].ambient_texname;
                    if (!ambientTexturePath.empty()) {
                        std::cout << "Loading ambient occlusion map: " << basePath + ambientTexturePath << std::endl;
                        gps::Texture currentTexture;
                        currentTexture = LoadTexture(basePath + ambientTexturePath, "ambientTexture");
                        textures.push_back(currentTexture);
                    }
                }
            }

            meshes.push_back(gps::Mesh(vertices, indices, textures));
        }
    }

	gps::Texture Model3D::LoadTexture(std::string path, std::string type) {

			std::cout << "Attempting to load texture: " << path << std::endl;
			for (int i = 0; i < loadedTextures.size(); i++) {

				if (loadedTextures[i].path == path)	{

					return loadedTextures[i];
				}
			}

			gps::Texture currentTexture;
			currentTexture.id = ReadTextureFromFile(path.c_str());
			currentTexture.type = std::string(type);
			currentTexture.path = path;

			loadedTextures.push_back(currentTexture);

			return currentTexture;
		}

	GLuint Model3D::ReadTextureFromFile(const char* file_name) {

		int x, y, n;
		int force_channels = 4;
		unsigned char* image_data = stbi_load(file_name, &x, &y, &n, force_channels);

		if (!image_data) {
			fprintf(stderr, "ERROR: could not load %s\n", file_name);
			return false;
		}
		if ((x & (x - 1)) != 0 || (y & (y - 1)) != 0) {
			fprintf(
				stderr, "WARNING: texture %s is not power-of-2 dimensions\n", file_name
			);
		}
		if (!image_data) {
			std::cout << "Failed to load image: " << file_name << std::endl;
		}
		int width_in_bytes = x * 4;
		unsigned char *top = NULL;
		unsigned char *bottom = NULL;
		unsigned char temp = 0;
		int half_height = y / 2;

		for (int row = 0; row < half_height; row++) {

			top = image_data + row * width_in_bytes;
			bottom = image_data + (y - row - 1) * width_in_bytes;

			for (int col = 0; col < width_in_bytes; col++) {

				temp = *top;
				*top = *bottom;
				*bottom = temp;
				top++;
				bottom++;
			}
		}

		GLuint textureID;
		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_SRGB, //GL_SRGB,//GL_RGBA,
			x,
			y,
			0,
			GL_RGBA,
			GL_UNSIGNED_BYTE,
			image_data
		);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);

		return textureID;
	}

	Model3D::~Model3D() {

        for (size_t i = 0; i < loadedTextures.size(); i++) {

            glDeleteTextures(1, &loadedTextures.at(i).id);
        }

        for (size_t i = 0; i < meshes.size(); i++) {

            GLuint VBO = meshes.at(i).getBuffers().VBO;
            GLuint EBO = meshes.at(i).getBuffers().EBO;
            GLuint VAO = meshes.at(i).getBuffers().VAO;
            glDeleteBuffers(1, &VBO);
            glDeleteBuffers(1, &EBO);
            glDeleteVertexArrays(1, &VAO);
        }
	}
}
