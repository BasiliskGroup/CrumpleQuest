#ifndef INSTANCE_HANDLER_H
#define INSTANCE_HANDLER_H

#include "instance/instancer.h"
#include "render/shader.h"
#include "scene/node.h"

template <typename T>
class InstanceHandler<T> {
    private:
        vector<Instancer<T>> instancers;

        Shader* shader;

    public:
        InstanceHandler(Shader* shader, std::vector<std::string> modelFormat, std::vector<std::string> instanceFormat);
        ~InstanceHandler();

        void add(Node* node);
}

#endif