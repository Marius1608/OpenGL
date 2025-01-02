#ifndef Camera_hpp
#define Camera_hpp

#include <C:\Users\pante\Desktop\An3_sem1\PG\Biblioteci\glm/glm.hpp>
#include <C:\Users\pante\Desktop\An3_sem1\PG\Biblioteci\glm/gtx/transform.hpp>


namespace gps {

    enum MOVE_DIRECTION { MOVE_FORWARD, MOVE_BACKWARD, MOVE_RIGHT, MOVE_LEFT };

    class Camera {
    public:
        Camera(glm::vec3 cameraPosition = glm::vec3(0.0f, 0.0f, 3.0f),
            glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, -1.0f),
            glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f));

        glm::mat4 getViewMatrix();
        void move(MOVE_DIRECTION direction, float speed);
        void rotate(float pitch, float yaw);

    private:
        glm::vec3 cameraPosition;
        glm::vec3 cameraTarget;
        glm::vec3 cameraFrontDirection;
        glm::vec3 cameraRightDirection;
        glm::vec3 cameraUpDirection;
        const float SENSITIVITY = 0.1f;
    };
}

#endif /* Camera_hpp */
