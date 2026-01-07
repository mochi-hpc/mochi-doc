/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include <bedrock/AbstractComponent.hpp>
#include <iostream>

struct MyProvider {};

class MyComponent : public bedrock::AbstractComponent {

    std::unique_ptr<MyProvider> m_provider;

    public:

    MyComponent()
    : m_provider{std::make_unique<MyProvider>()} {}

    static std::vector<bedrock::Dependency>
        GetDependencies(const bedrock::ComponentArgs& args) {
        (void)args;
        std::vector<bedrock::Dependency> deps = {
            { "pool", "pool", true, false, true },
            { "kv_store", "yokan", true, true, false }
        };
        return deps;
    }

    static std::shared_ptr<bedrock::AbstractComponent>
        Register(const bedrock::ComponentArgs& args) {
            std::cout << "Registering a MyComponent" << std::endl;
            std::cout << " -> mid = " << (void*)args.engine.get_margo_instance() << std::endl;
            std::cout << " -> provider id = " << args.provider_id << std::endl;
            std::cout << " -> config = " << args.config << std::endl;
            std::cout << " -> name = " << args.name << std::endl;
            std::cout << " -> tags = ";
            for(auto& t : args.tags) std::cout << t << " ";
            std::cout << std::endl;
            auto pool_it = args.dependencies.find("pool");
            auto pool = pool_it->second[0]->getHandle<thallium::pool>();
            return std::make_shared<MyComponent>();
    }

    void* getHandle() override {
        return static_cast<void*>(m_provider.get());
    }

    /* optional */
    std::string getConfig() override {
        return "{}";
    }

    /* optional */
    void changeDependency(
            const std::string& dep_name,
            const bedrock::NamedDependencyList& dependencies) override {
        throw bedrock::Exception{"Operation not supported"};
    }

    /* optional */
    void migrate(
            const std::string& dest_addr,
            uint16_t dest_component_id,
            const std::string& options_json,
            bool remove_source) override {
        throw bedrock::Exception{"Operation not supported"};
    }

    /* optional */
    void snapshot(
            const std::string& dest_path,
            const std::string& options_json,
            bool remove_source) override {
        throw bedrock::Exception{"Operation not supported"};
    }

    /* optional */
    void restore(
            const std::string& src_path,
            const char* options_json) override {
        throw bedrock::Exception{"Operation not supported"};
    }
};

BEDROCK_REGISTER_COMPONENT_TYPE(my_module, MyComponent)
