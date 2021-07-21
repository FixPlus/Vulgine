//
// Created by Бушев Дмитрий on 17.07.2021.
//

#ifndef TEST_EXE_VULGINEOBJECT_H
#define TEST_EXE_VULGINEOBJECT_H

#include "IVulgineObjects.h"
#include <stack>
#include <functional>

namespace Vulgine{
    class ObjectImpl: public virtual Object{
    private:

        static std::stack<uint32_t> freeIds;
        static uint32_t claimedIdsCount;
        static std::unordered_map<uint32_t, ObjectImpl*> objMap;

        static std::unordered_map<Type, uint32_t> countMap;          // TODO: make this full-static map (constexpr)
        static std::unordered_map<Type, std::string> typeNames;      // TODO: make this full static map (constexpr)

        Type typeId_;
        uint32_t id_;


        static void invalidateId(uint32_t id);


    protected:
        virtual void createImpl() = 0;
        virtual void destroyImpl() = 0;
        std::optional<std::string> name;
    public:

        static uint32_t claimId();

        explicit ObjectImpl(Type typeId, uint32_t id);
        ObjectImpl& operator=(ObjectImpl&& another) noexcept;
        ObjectImpl(ObjectImpl&& another) noexcept ;

        void create() final {
            if(created)
                return;
            createImpl();
            created = true;
        };

        void destroy() final {
            if(!created)
                return;
            destroyImpl();
            created = false;
        }


        static void fillTypeNameTable();

        void setName(std::string const& newName) final { name.reset(); name.emplace(newName);};

        std::string objectLabel() const final;

        std::string typeName() const;

        Type type() const { return typeId_; };

        uint32_t id() const final{ return id_;}
        static uint32_t count(Type type);
        static ObjectImpl* get(uint32_t id);

        static void for_each(std::function<void(ObjectImpl*)> action);

        ~ObjectImpl() override;
    };

    bool checkDeviceLimits(ObjectImpl* object);

#define SELF_CHECK_DEVICE_LIMITS() assert(checkDeviceLimits(this) && "object exceeds device limits");
#define CHECK_DEVICE_LIMITS(OBJ) assert(checkDeviceLimits(OBJ) && "object exceeds device limits");

    struct ObjectImplNoMove: public ObjectImpl{
        ObjectImplNoMove(Type typeId, uint32_t id): ObjectImpl(typeId, id){};
        ObjectImplNoMove& operator=(ObjectImplNoMove&& another) = delete;
        ObjectImplNoMove(ObjectImplNoMove&& another) = delete;
    };

}
#endif //TEST_EXE_VULGINEOBJECT_H
