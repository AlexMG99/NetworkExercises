#include "Networks.h"
#include "ModuleGameObject.h"

bool ModuleGameObject::init()
{
	return true;
}

bool ModuleGameObject::preUpdate()
{
	BEGIN_TIMED_BLOCK(GOPreUpdate);

	static const GameObject::State gNextState[] = {
		GameObject::NON_EXISTING, // After NON_EXISTING
		GameObject::STARTING,     // After INSTANTIATE
		GameObject::UPDATING,     // After STARTING
		GameObject::UPDATING,     // After UPDATING
		GameObject::DESTROYING,   // After DESTROY
		GameObject::NON_EXISTING  // After DESTROYING
	};

	for (GameObject &gameObject : gameObjects)
	{
		gameObject.state = gNextState[gameObject.state];
	}

	END_TIMED_BLOCK(GOPreUpdate);

	return true;
}

bool ModuleGameObject::update()
{
	// Delayed destructions
	for (DelayedDestroyEntry &destroyEntry : gameObjectsWithDelayedDestruction)
	{
		if (destroyEntry.object != nullptr)
		{
			destroyEntry.delaySeconds -= Time.deltaTime;
			if (destroyEntry.delaySeconds <= 0.0f)
			{
				Destroy(destroyEntry.object);
				destroyEntry.object = nullptr;
			}
		}
	}

	return true;
}

bool ModuleGameObject::postUpdate()
{
	return true;
}

bool ModuleGameObject::cleanUp()
{
	return true;
}

GameObject * ModuleGameObject::Instantiate()
{
	for (uint32 i = 0; i < MAX_GAME_OBJECTS; ++i)
	{
		GameObject &gameObject = App->modGameObject->gameObjects[i];

		if (gameObject.state == GameObject::NON_EXISTING)
		{
			gameObject = GameObject();
			gameObject.id = i;
			gameObject.state = GameObject::INSTANTIATE;
			return &gameObject;
		}
	}

	ASSERT(0); // NOTE(jesus): You need to increase MAX_GAME_OBJECTS in case this assert crashes
	return nullptr;
}

void ModuleGameObject::Destroy(GameObject * gameObject)
{
	ASSERT(gameObject->networkId == 0); // NOTE(jesus): If it has a network identity, it must be destroyed by the Networking module first

	static const GameObject::State gNextState[] = {
		GameObject::NON_EXISTING, // After NON_EXISTING
		GameObject::DESTROY,      // After INSTANTIATE
		GameObject::DESTROY,      // After STARTING
		GameObject::DESTROY,      // After UPDATING
		GameObject::DESTROY,      // After DESTROY
		GameObject::DESTROYING    // After DESTROYING
	};

	ASSERT(gameObject->state < GameObject::STATE_COUNT);
	gameObject->state = gNextState[gameObject->state];
}

void ModuleGameObject::Destroy(GameObject * gameObject, float delaySeconds)
{
	for (uint32 i = 0; i < MAX_GAME_OBJECTS; ++i)
	{
		if (App->modGameObject->gameObjectsWithDelayedDestruction[i].object == nullptr)
		{
			App->modGameObject->gameObjectsWithDelayedDestruction[i].object = gameObject;
			App->modGameObject->gameObjectsWithDelayedDestruction[i].delaySeconds = delaySeconds;
			break;
		}
	}
}

GameObject * Instantiate()
{
	GameObject *result = ModuleGameObject::Instantiate();
	return result;
}

void Destroy(GameObject * gameObject)
{
	ModuleGameObject::Destroy(gameObject);
}

void Destroy(GameObject * gameObject, float delaySeconds)
{
	ModuleGameObject::Destroy(gameObject, delaySeconds);
}


void GameObject::createRead(const InputMemoryStream& packet)
{
	packet >> hasTeleported;

	// Transform
	packet >> position.x;
	packet >> position.y;
	packet >> size.x;
	packet >> size.y;
	packet >> angle;

	initial_position = final_position = position;
	initial_angle = final_angle = angle;
}

void GameObject::read(const InputMemoryStream& packet)
{
	packet >> hasTeleported;
	// Transform
	if (networkInterpolationEnabled && !hasTeleported)
	{
		packet >> final_position.x;
		packet >> final_position.y;
		packet >> size.x;
		packet >> size.y;
		packet >> final_angle;

		initial_position = position;
		initial_angle = angle;

		secondsElapsed = 0.0f;
	}
	else
	{
		packet >> position.x;
		packet >> position.y;
		packet >> size.x;
		packet >> size.y;
		packet >> angle;
	}

}

void GameObject::write(OutputMemoryStream& packet)
{
	packet << hasTeleported;

	// Transform
	packet << position.x;
	packet << position.y;
	packet << size.x;
	packet << size.y;
	packet << angle;
}

void GameObject::interpolate()
{
	float t = secondsElapsed / MAX_NETWORK_DELAY_SECONDS;

	if (t < 1)
	{
		position = lerp(initial_position, final_position, t);
		angle = lerp(initial_angle, final_angle, t);

		secondsElapsed += Time.deltaTime;
	}
	//LOG("Time: %f", t);
	/*LOG("Posicion del otro player: %f, %f", position.x, position.y);*/
}
