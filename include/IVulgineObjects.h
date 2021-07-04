//
// Created by Бушев Дмитрий on 04.07.2021.
//

#ifndef TEST_EXE_IVULGINEOBJECTS_H
#define TEST_EXE_IVULGINEOBJECTS_H

#include <vector>
#include <cstdint>
#include <glm/vec3.hpp>

namespace Vulgine{

    class Creatable{
        bool created = false;
    protected:
        virtual void createImpl() = 0;
        virtual void destroyImpl() = 0;
    public:
        void create() {
            if(created)
                return;
            createImpl();
            created = true;
        };

        void destroy() {
            if(!created)
                return;
            destroyImpl();
            created = false;
        }

        bool isCreated() const{
            return created;
        }


        virtual ~Creatable() = default;
    };


    struct Camera: public Creatable{
        glm::vec3 position;
        glm::vec3 rotation;
    };


    struct RenderTarget: public Creatable{

        enum { COLOR, DEPTH_STENCIL} attachmentType;
        enum { SCREEN, OFFSCREEN} renderType;


    };

    struct Material: public Creatable{

    };

    struct Scene;

    class Light: public Creatable{
    protected:
        Scene* parent_;
    public:
        explicit Light(Scene* parent): parent_(parent){}
        glm::vec3 direction;
        Scene* parent() const{ return parent_;};
    };


    struct RenderTask{
        Scene* scene;
        Camera* camera;
        std::vector<RenderTarget*> renderTargets;
    };

    class Mesh: public Creatable{
    protected:
        Scene* parent_;
    public:

        explicit Mesh(Scene* parent): parent_(parent){}
        /**
         *
         *  RGBA32SF -->  glm::vec4
         *  RGB32SF  -->  glm::vec3
         *  RG32SF   -->  glm::vec2
         *  R32SF    -->  glm::vec1(32-bit float)
         *
         * */

        enum class AttributeFormat{RGBA32SF, RGB32SF, RG32SF, R32SF};

        struct VertexFormat{
            std::vector<AttributeFormat> perVertexAttributes;
            std::vector<AttributeFormat> perInstanceAttributes;
        } vertexFormat;

        struct {
            /** non-owning pointer to valid vertex data */
            void *pData = nullptr;
            uint32_t count = 0;
        } vertices;

        std::vector<uint32_t> indices{0};

        struct {
            /** non-owning pointer to valid instance data */
            void *pData = nullptr;
            uint32_t count = 0;
        } instances;

        struct Primitive{
            Material* material = nullptr;
            uint32_t startIdx = 0;
            uint32_t endIdx = 0;
        };

        std::vector<Primitive> primitives{};

        /**  Creates buffers and other resources holding vertex and instance information for a mesh
         *   Then pushes them to video memory
         * */

        // inherits 'void create()' from Creatable

        /** Updates vertex and index buffer and pushes them to video memory*/

        virtual void updateVertexData() = 0;

        /** Updates instance buffer and pushes it to video memory*/

        virtual void updateInstanceData() = 0;

        Scene* parent() const{ return parent_;};



    };
}
#endif //TEST_EXE_IVULGINEOBJECTS_H
