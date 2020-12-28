#pragma once

#include "Behaviours.h"

class ModuleBehaviour : public Module
{
public:

	bool update() override;

	Behaviour * addBehaviour(BehaviourType behaviourType, GameObject *parentGameObject);
	Spaceship * addSpaceship(GameObject *parentGameObject);
	Laser     * addLaser(GameObject *parentGameObject);
	Meteorite *	addMeteorite(GameObject* parentGameObject);

private:

	void handleBehaviourLifeCycle(Behaviour * behaviour);

	Spaceship spaceships[MAX_CLIENTS];
	Laser lasers[MAX_GAME_OBJECTS];
	Meteorite meteorite[MAX_GAME_OBJECTS];
};

