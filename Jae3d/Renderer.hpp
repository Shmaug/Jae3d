#pragma once

#include <memory>
#include "Object.hpp"
#include <DirectXCollision.h>

class CommandList;

class Renderer : public Object {
public:
	Renderer() : Object(L""), mVisible(true) {}
	Renderer(jwstring name) : Object(name), mVisible(true) {}
	~Renderer() {}

	virtual void Draw(std::shared_ptr<CommandList> commandList) = 0;
	bool mVisible;
};