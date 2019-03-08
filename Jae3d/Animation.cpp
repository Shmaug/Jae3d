#include "Animation.hpp"

Animation::Animation(const jwstring& name) : Asset(name) {}
Animation::Animation(const jwstring& name, MemoryStream &ms) : Asset(name, ms) {
	
}
Animation::~Animation() {}

uint64_t Animation::TypeId() {
	return (uint64_t)ASSET_TYPE_ANIMATION;
}
void Animation::WriteData(MemoryStream &ms) {

}