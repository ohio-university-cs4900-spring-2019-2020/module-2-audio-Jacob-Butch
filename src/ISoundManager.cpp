#include "ISoundManager.h"

#include <iostream>
#include <irrKlang.h>

using namespace Aftr;
using namespace irrklang;

ISoundEngine* ISoundManager::engine = nullptr;

void ISoundManager::init() {
	engine = createIrrKlangDevice();
	if (!engine) {
		std::cout << "Error starting engine!" << std::endl;
		exit(-1);
	}
	std::cout << "Sound engine initialized successfully!" << std::endl;
}

void ISoundManager::drop() {
	if (engine != nullptr) {
		engine->drop();
	}
}

vec3df ISoundManager::toVec3df(const Vector& vector) {
	return vec3df(vector.x, -1 * vector.y, vector.z);
}


void ISoundManager::setListenerPosition(const Vector& pos, const Vector& lookdir, const Vector& vel, const Vector& updir){
	engine->setListenerPosition(toVec3df(pos), toVec3df(lookdir), toVec3df(vel), toVec3df(updir));
}