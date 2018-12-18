#pragma once

#include <memory>
#include "Object.hpp"

class CommandList;

class Renderer : public Object {
public:
	Renderer() : Object(L"") {}
	Renderer(jwstring name) : Object(name) {}
	~Renderer() {}

	virtual void Draw(std::shared_ptr<CommandList> commandList) = 0;
};