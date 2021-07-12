//
// Created by Бушев Дмитрий on 04.07.2021.
//

#ifndef TEST_EXE_IVULGINEOBJECTS_H
#define TEST_EXE_IVULGINEOBJECTS_H

#include <vector>
#include <cstdint>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtx/transform.hpp>

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

    struct Image{
        uint32_t id_;
    public:
        enum FileFormat{FILE_FORMAT_PNG, FILE_FORMAT_JPEG, FILE_FORMAT_KTX};
        enum Format{FORMAT_R8G8B8A8_UNORM, FORMAT_R8G8B8A8_SRGB};

        virtual bool loadFromFile(const char* filename, FileFormat fileFormat) = 0;
        virtual bool load(const unsigned char* data, uint32_t len, FileFormat fileFormat) = 0;
        explicit Image(uint32_t id): id_(id){}
        uint32_t id() const { return id_;}
        virtual ~Image() = default;
    };

    struct Scene;

    struct Camera: public Creatable{
    protected:
        Scene* parent_;
        uint32_t id_;
    public:

        Camera(Scene* parent, uint32_t id): parent_(parent), id_(id){ projection[1][1] *= -1; }
        glm::vec3 position = glm::vec3{0.0f};
        glm::vec3 rotation = glm::vec3{0.0f, 0.0f, 0.0f};
        glm::mat4 projection = glm::perspective(glm::radians(60.0f), 16.0f / 9.0f, 0.1f, 100.0f);

        virtual void update() = 0;
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
 *  MAT4F    -->  glm::mat4
 *
 * */

    enum class AttributeFormat{RGBA32SF, RGB32SF, RG32SF, R32SF, MAT4F};

    struct VertexFormat{
        std::vector<AttributeFormat> perVertexAttributes;
        std::vector<AttributeFormat> perInstanceAttributes;

        uint32_t perVertexSize() const;
        uint32_t perInstanceSize() const;
    };


    class Material: public Creatable{
        uint32_t id_;
    public:

        struct{
            ::Vulgine::Image* colorMap = nullptr;
            ::Vulgine::Image* normalMap = nullptr;
        } texture;

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

        /**
         * @brief vertices
         *
         * Contains information about mesh vertex structure
         *
         * @param pData
         * Contains pointer to valid vertex data allocated and initialized by application.
         * @param count
         * count of vertices
         * @param dynamic
         * notification for Vulgine that application intends to frequently update vertex data
         *
         */
        struct {
            void *pData = nullptr;
            uint32_t count = 0;
            bool dynamic = false;
        } vertices;

        std::vector<uint32_t> indices{0};

        /**
         * @brief instances
         *
         * Contains information about mesh vertex structure
         *
         * @param pData
         * Contains pointer to valid instance data allocated and initialized by application.
         * @param count
         * count of instances
         * @param dynamic
         * notification for Vulgine that application intends to frequently update instance data
         *
         */
        struct {
            void *pData = nullptr;
            uint32_t count = 0;
            bool dynamic = true;
        } instances;

        /**
         * @brief primitive
         *
         * Contains information about single mesh primitive
         *
         * @param material
         * Contains pointer to valid Material object allocated by Vulgine. Primitive will be rendered using this material.
         * @param startIdx
         * indicates index inside of vertex array to start primitive with
         * @param indexCount
         * number of indexes in primitive
         *
         */
        struct Primitive{
            Material* material = nullptr;
            uint32_t startIdx = 0;
            uint32_t indexCount = 0;
        };

        std::vector<Primitive> primitives{};

        /**  Creates buffers and other resources holding vertex and instance information for a mesh
         *   Then pushes them to video memory
         * */

        // inherits 'void create()' from Creatable

        /** Updates vertex and index buffer and pushes them to video memory*/

        virtual void updateVertexBuffer() = 0;

        virtual void updateIndexBuffer() = 0;

        /** Updates instance buffer and pushes it to video memory*/

        virtual void updateInstanceBuffer() = 0;

        Scene* parent() const{ return parent_;};
        uint32_t id() const { return id_;}



    };
}
#endif //TEST_EXE_IVULGINEOBJECTS_H
