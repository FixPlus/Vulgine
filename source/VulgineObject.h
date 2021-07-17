//
// Created by Бушев Дмитрий on 17.07.2021.
//

#ifndef TEST_EXE_VULGINEOBJECT_H
#define TEST_EXE_VULGINEOBJECT_H

#include "IVulgineObjects.h"

namespace Vulgine{
    class ObjectImpl: public virtual Object{
    private:
        bool created = false;

        static std::unordered_map<Type, uint32_t> countMap;
        static std::unordered_map<Type, std::string> typeNames;

        Type typeId_;
        uint32_t id_;


    protected:
        virtual void createImpl() = 0;
        virtual void destroyImpl() = 0;
        std::optional<std::string> name;
    public:
        ObjectImpl(uint32_t id, Type typeId);

        ObjectImpl(ObjectImpl&& another) = default;
        ObjectImpl& operator=(ObjectImpl&& another) = default;


        void create() override {
            if(created)
                return;
            createImpl();
            created = true;
        };

        void destroy() override {
            if(!created)
                return;
            destroyImpl();
            created = false;
        }

        bool isCreated() const override{
            return created;
        }


        static void fillTypeNameTable();

        void setName(std::string const& newName) override { name.reset(); name.emplace(newName);};

        std::string objectLabel() const override;

        std::string typeName() const;

        uint32_t id() const override{ return id_;}
        static uint32_t count(Type type);
        ~ObjectImpl() override;
    };

}
#endif //TEST_EXE_VULGINEOBJECT_H
