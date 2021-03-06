//
// Created by Бушев Дмитрий on 04.07.2021.
//

#ifndef TEST_EXE_IVULGINEOBJECTS_H
#define TEST_EXE_IVULGINEOBJECTS_H

#include <vector>
#include <cstdint>
#include <unordered_map>
#include <memory>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtx/transform.hpp>

#include <string>

namespace Vulgine{

    template<typename T>
    using SharedRef = std::shared_ptr<T>;
    template<typename T>
    using WeakRef = std::weak_ptr<T>;
#define TYPEDEF_REF(X) using X ## Ref = SharedRef<X>;
#define TYPEDEF_WEAK_REF(X) using X ## WeakRef = WeakRef<X>;

    /**
    * @brief General interface for most Vulgine Objects.
    *
    * Vulgine objects sharing this interface have common use pattern:
    *
    * @fill  Fill up all public fields (create info).
    * @create    call create() member function.
    * @use   Use this object.
    * @destroy   Destroy object:
    *        a) call destroy() member function - you can later reuse this object.
    *        Or
    *        b) invalidate this object using appropriate deleteObject(object) function - you cannot reuse this object.
    *        in this case.
    *
    *  \Remember that if resource owning this object gets freed (by calling destroy() in particular) or invalidated,
    *  the object gets implicitly invalidated. Any use of invalidated object is forbidden.
    *
    *  Vulgine objects are not copiable as they encapsulate unique data structures they are to allocated and free
    *  when destructed. Every object in Vulgine has its own unique ID - unsigned 32 bit integer number. Optionally
    *  you can name object and Vulgine will display it in corresponding debug sections of UI instead of default
    *  "%Type% #ID" output. Vulgine keeps track of count of each object type entities allocated. This information
    *  can be retrieved by corresponding Object::count(%Type%) static function.
    *
    */

    class Object{
    protected:
        bool created = false;
    public:

        /** @brief list of types Vulgine is using. Some of them are fully internal and not accessible by API */

        enum class Type{
            MATERIAL,
            SCENE,
            IMAGE,
            MESH,
            UBO,
            CAMERA,
            LIGHT,
            RENDER_PASS,
            FRAME_BUFFER,
            PIPELINE,           // internal only
            SHADER_MODULE,      // internal only (yet)
            DESCRIPTOR_SET,     // internal only (maybe public in future)
            NONE,
            SAMPLER,
            GEOMETRY,
            UNKNOWN             // other uncategorized internal only objects
        };

        Object() = default;
        Object(Object const& another) = delete;
        Object& operator=(Object const& another) = delete;
        Object& operator=(Object&& another) = default;
        Object(Object&& another) = default;

        /** @brief allocates and prepares all internal resources of an object based on interface part state
         *
         *  Before calling this function, object is in "not-ready" state, so using or/and referencing it is not allowed
         *
         * */

        virtual void create() = 0;

        /** @brief frees all the resources allocated by create() function thus returning object in "not-ready" state */

        virtual void destroy() = 0;

        /** @brief query whether the object is now in "ready" state or not */

        bool isCreated() const { return created; };

        /** @brief gives object a name(not necessarily unique) */

        virtual void setName(std::string const& newName) = 0;

        /** @brief gets string version of object id info */

        virtual std::string objectLabel() const = 0;

        /** @brief ID info */

        virtual uint32_t id() const = 0;

        /** @brief returns count of total allocated objects of certain type */

        static uint32_t count(Type type);

        /** @brief returns pointer to an object with specified id. If id is invalid, returns null pointer*/

        static Object* get(uint32_t id);

        virtual ~Object() = default;

    };


    /**
     *
     * @brief abstraction of any image object
     *
     * @use Can be used as texture target by material, render target by render pass
     *
     * @import Store data to it using load() and loadFromFile() functions
     *
     * @formats Supported importing of images: png, jpeg, bmp, TODO: ktx
     *
     *
     */

    struct Image: virtual public Object{

        enum FileFormat{FILE_FORMAT_PNG, FILE_FORMAT_JPEG, FILE_FORMAT_KTX};
        enum Format{FORMAT_R8G8B8A8_UNORM, FORMAT_R8G8B8A8_SRGB};

        virtual bool loadFromFile(const char* filename, FileFormat fileFormat) = 0;
        virtual bool load(const unsigned char* data, uint32_t len, FileFormat fileFormat) = 0;


    };


    /**
     *
     * @brief uniform buffer object (ubo)
     *
     * @param dynamic tells Vulgine whether this buffer will be updated frequently
     *
     * @param pData valid pointer to data. Note: if dynamic = false is set, pointer only must be valid throughout create()
     * call, otherwise it must remain valid for whole lifetime of ubo.
     *
     * @param size size in bytes
     *
     * @note size once set cannot be changed.
     *
     *
     */

    struct UniformBuffer: virtual public Object{

        void* pData = nullptr;
        uint32_t size = 0;

        bool dynamic = true;


        virtual void update() = 0;
    };

    TYPEDEF_REF(Image)
    TYPEDEF_REF(UniformBuffer)

    struct Scene;

    /**
     *
     * @brief encapsulates view matrix calculation based on user-defined position, rotation and projection
     *
     */

    struct Camera: virtual public Object{
    protected:
        Scene* parent_;
    public:

        Camera(Scene* parent): parent_(parent) { projection[1][1] *= -1; }
        glm::vec3 position = glm::vec3{0.0f};
        glm::vec3 rotation = glm::vec3{0.0f, 0.0f, 0.0f};
        glm::mat4 projection = glm::perspective(glm::radians(60.0f), 16.0f / 9.0f, 0.1f, 100.0f);

        virtual void update() = 0;
        Scene* parent() const{ return parent_;};

    };

    TYPEDEF_REF(Camera)

    struct Sampler: virtual public Object{
        enum class Filtering{NONE, LINEAR} filtering = Filtering::LINEAR;
    };

    TYPEDEF_REF(Sampler)

    struct Descriptor{
        UniformBufferRef ubo = nullptr;
        SamplerRef sampler = nullptr;
        ImageRef image = nullptr;
    };

    struct DescriptorInfo{
        enum class Type{ UNIFORM_BUFFER, COMBINED_IMAGE_SAMPLER} type;
        int binding;
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


    /**
     *
     * @brief contains information about surface rendering (generally -  fragment shader stage info)
     *
     *
     */
    struct Material: virtual public Object{


        bool alpha_transparent = false;

        struct{
            ::Vulgine::SamplerRef sampler = nullptr;
            ::Vulgine::ImageRef colorMap = nullptr;
            ::Vulgine::ImageRef normalMap = nullptr;
        } texture;

        glm::vec4 baseColor = glm::vec4(1.0f); // used when colorMap == nullptr
        float specular = 1.0f;

        virtual void update() = 0;
        bool custom = false;

        struct {

            std::vector<AttributeFormat> inputAttributes = {};
            std::vector<std::pair<DescriptorInfo, Descriptor>> descriptors = {};
            std::string fragmentShader = "frag_default";

        } customMaterialInfo;

    };

    TYPEDEF_REF(Material)
    TYPEDEF_WEAK_REF(Material)

    /**
     *
     * @brief encapsulates light source implementation
     *
     */

    class Light: virtual public Object{
    protected:
        Scene* parent_;
    public:



        virtual void update() = 0;

        Light(Scene* parent): parent_(parent){}

        glm::vec3 color = glm::vec3{1.0f};
        glm::vec2 direction = glm::vec2{0.0f, 0.0f};

        Scene* parent() const{ return parent_;};
    };

    TYPEDEF_REF(Light)

    struct Geometry: virtual public Object{
        VertexFormat vertexFormat;
        std::string vertexShader = "vert_default";

        enum CullMode {NONE, CLOCKWISE, COUNTER_CLOCKWISE} cullMode = COUNTER_CLOCKWISE;
        enum FillMode {FILL, LINE, POINT} fillMode = FILL;

        std::vector<DescriptorInfo> descriptors{};
    };

    TYPEDEF_REF(Geometry)
    TYPEDEF_WEAK_REF(Geometry)
    /**
     *
     * @brief rendered object
     *
     */
    class Mesh: virtual public Object{
    public:


        GeometryRef geometry{};

        std::vector<Descriptor> descriptors{0};

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

        std::vector<uint32_t> indices{};

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
            MaterialRef material{};
            uint32_t startIdx = 0;
            uint32_t indexCount = 0;
        };

        std::vector<Primitive> primitives{};

        /**  Creates buffers and other resources holding vertex and instance information for a mesh
         *   Then pushes them to video memory
         * */

        // inherits 'void create()' from Object

        /** Updates vertex and index buffer and pushes them to video memory*/

        virtual void updateVertexBuffer() = 0;

        virtual void updateIndexBuffer() = 0;

        /** Updates instance buffer and pushes it to video memory*/

        virtual void updateInstanceBuffer() = 0;


    };

    TYPEDEF_REF(Mesh)


    struct RenderPass;

    TYPEDEF_WEAK_REF(RenderPass)
    TYPEDEF_REF(Scene)

    struct RenderPass: virtual public Object{

        enum class AttachmentType { COLOR, DEPTH_STENCIL};

        struct {
            uint32_t width, height;
        } framebufferExtents;

        bool onscreen = true;

        SceneRef scene = nullptr;
        CameraRef camera = nullptr;

        virtual ImageRef addAttachment(AttachmentType type = AttachmentType::COLOR) = 0;

        virtual uint32_t attachmentCount() = 0;

        virtual ImageRef getAttachment(uint32_t binding) = 0;


        std::vector<RenderPassWeakRef> dependencies;

    };


    TYPEDEF_REF(RenderPass)

}

#undef TYPEDEF_REF
#endif //TEST_EXE_IVULGINEOBJECTS_H
