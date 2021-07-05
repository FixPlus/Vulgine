//
// Created by Бушев Дмитрий on 04.07.2021.
//

#ifndef TEST_EXE_IVULGINEOBJECTS_H
#define TEST_EXE_IVULGINEOBJECTS_H

#include <vector>
#include <cstdint>
#include <glm/vec3.hpp>
#include <string>

namespace Vulgine{

    class Creatable{
        bool created = false;
    protected:
        virtual void createImpl() = 0;
        virtual void destroyImpl() = 0;
    public:
        Creatable() = default;
        Creatable(Creatable const& another) = delete;
        Creatable const& operator=(Creatable const& another) = delete;
        Creatable(Creatable&& another) = default;
        Creatable& operator=(Creatable&& another) = default;
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

    struct Scene;

    struct Camera: public Creatable{
    protected:
        Scene* parent_;
        uint32_t id_;
    public:

        Camera(Scene* parent, uint32_t id): parent_(parent), id_(id){}
        glm::vec3 position;
        glm::vec3 rotation;

        Scene* parent() const{ return parent_;};
        uint32_t id() const { return id_;}

    };


    struct RenderTarget{

        enum { COLOR, DEPTH_STENCIL} attachmentType;
        enum { SCREEN, OFFSCREEN} renderType;


    };

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
    };


    class Material: public Creatable{
        uint32_t id_;
    public:

        VertexFormat vertexFormat;

        std::string vertexShader;

        explicit Material(uint32_t id): id_(id){}
        uint32_t id() const { return id_;}
    };



    class Light: public Creatable{
    protected:
        Scene* parent_;
        uint32_t id_;
    public:
        Light(Scene* parent, uint32_t id): parent_(parent), id_(id){}
        glm::vec3 direction;
        Scene* parent() const{ return parent_;};
        uint32_t id() const { return id_;}
    };


    struct RenderTask{
        Scene* scene;
        Camera* camera;
        std::vector<RenderTarget> renderTargets;
    };

    class Mesh: public Creatable{
    protected:
        Scene* parent_;
        uint32_t id_;
    public:

        Mesh(Scene* parent, uint32_t id): parent_(parent), id_(id){}

        VertexFormat vertexFormat;

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
        uint32_t id() const { return id_;}



    };
}
#endif //TEST_EXE_IVULGINEOBJECTS_H
