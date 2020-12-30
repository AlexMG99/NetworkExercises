#include "Networks.h"
#include "Behaviours.h"
#include "ModuleNetworking.h"

void Laser::start()
{
	gameObject->networkInterpolationEnabled = true;

	App->modSound->playAudioClip(App->modResources->audioClipLaser);
}

void Laser::update()
{
	secondsSinceCreation += Time.deltaTime;

	const float pixelsPerSecond = 1000.0f;
	gameObject->position += vec2FromDegrees(gameObject->angle) * pixelsPerSecond * Time.deltaTime;

	if (isServer)
	{
		const float neutralTimeSeconds = 0.1f;
		if (secondsSinceCreation > neutralTimeSeconds && gameObject->collider == nullptr) {
			gameObject->collider = App->modCollision->addCollider(ColliderType::Laser, gameObject);
		}

		const float lifetimeSeconds = 2.0f;
		if (secondsSinceCreation >= lifetimeSeconds) {
			NetworkDestroy(gameObject);
		}
	}
}


void Spaceship::start()
{
	gameObject->tag = (uint32)(Random.next() * UINT_MAX);

	if (App->modGameObject->spaceship01 == nullptr)
	{
		App->modGameObject->spaceship01 = gameObject;
	}
	else if (App->modGameObject->spaceship02 == nullptr)
	{
		App->modGameObject->spaceship02 = gameObject;
	}

	lifebar = Instantiate();
	lifebar->sprite = App->modRender->addSprite(lifebar);
	lifebar->sprite->pivot = vec2{ 0.0f, 0.5f };
	lifebar->sprite->order = 5;
}

void Spaceship::onInput(const InputController &input)
{
	if (input.horizontalAxis != 0.0f)
	{
		gameObject->angle += input.horizontalAxis * rotateSpeed * Time.deltaTime;

		if (isServer)
		{
			NetworkUpdate(gameObject);
		}
	}

	if (input.actionDown == ButtonState::Pressed)
	{
		gameObject->position += vec2FromDegrees(gameObject->angle) * advanceSpeed * Time.deltaTime;

		if (isServer)
		{
			NetworkUpdate(gameObject);
		}
	}

	if (input.actionLeft == ButtonState::Press && (timeSinceShot > shootCoolDown))
	{
		if (isServer)
		{
			GameObject *laser = NetworkInstantiate();

			laser->position = gameObject->position;
			laser->angle = gameObject->angle;
			laser->size = { 20, 60 };

			laser->sprite = App->modRender->addSprite(laser);
			laser->sprite->order = 3;
			laser->sprite->texture = App->modResources->laser;

			Laser *laserBehaviour = App->modBehaviour->addLaser(laser);
			laserBehaviour->isServer = isServer;

			laser->tag = gameObject->tag;

			timeSinceShot = 0.0f;
		}
	}
}

void Spaceship::update()
{
	secondsSinceHit += Time.deltaTime;
	timeSinceShot += Time.deltaTime;

	static const vec4 colorAlive = vec4{ 0.2f, 1.0f, 0.1f, 0.5f };
	static const vec4 colorDead = vec4{ 1.0f, 0.2f, 0.1f, 0.5f };
	const float lifeRatio = max(0.01f, (float)(hitPoints) / (MAX_HIT_POINTS));
	lifebar->position = gameObject->position + vec2{ -50.0f, -50.0f };
	lifebar->size = vec2{ lifeRatio * 80.0f, 5.0f };
	lifebar->sprite->color = lerp(colorDead, colorAlive, lifeRatio);

	
}

void Spaceship::destroy()
{
	if (App->modGameObject->spaceship01 != nullptr && App->modGameObject->spaceship01->networkId == gameObject->networkId)
		App->modGameObject->spaceship01 = nullptr;
	else if (App->modGameObject->spaceship02 != nullptr && App->modGameObject->spaceship02->networkId == gameObject->networkId)
		App->modGameObject->spaceship02 = nullptr;

	Destroy(lifebar);
}

void Spaceship::onCollisionTriggered(Collider &c1, Collider &c2)
{
	const float neutralHitTime = 1.5f;
	if (secondsSinceHit < neutralHitTime)
		return;

	if (c2.type == ColliderType::Laser && c2.gameObject->tag != gameObject->tag)
	{
		if (isServer)
		{
			NetworkDestroy(c2.gameObject); // Destroy the laser
		
			if (hitPoints > 0)
			{
				hitPoints--;
				NetworkUpdate(gameObject);
			}

			float size = gameObject->size.x + 50.0f * Random.next();
			vec2 position = gameObject->position + 50.0f * vec2{Random.next() - 0.5f, Random.next() - 0.5f};

			if (hitPoints <= 0)
			{
				// Centered big explosion
				size = 250.0f + 100.0f * Random.next();
				position = gameObject->position;

				gameObject->position = { 2000,2000 };
				gameObject->hasTeleported = true;

				isDead = true;
				//NetworkDestroy(gameObject);
			}

			CreateExplotion(position, size);

			// NOTE(jesus): Only played in the server right now...
			// You need to somehow make this happen in clients
			App->modSound->playAudioClip(App->modResources->audioClipExplosion);

			secondsSinceHit = 0.0f;
		}
	}
	else if(c2.type == ColliderType::Meteorite && c2.gameObject->tag != gameObject->tag)
	{
		if (isServer)
		{
			NetworkDestroy(c2.gameObject); // Destroy the laser

			if (hitPoints > 0)
			{
				hitPoints--;
				NetworkUpdate(gameObject); 

				score += 100;

				Meteorite* meteoriteBehaviour = (Meteorite*)c2.gameObject->behaviour;

				if (meteoriteBehaviour->GetCurrentLevel() < meteoriteBehaviour->GetMaxLevel())
				{
					for (int i = 0; i < meteoriteBehaviour->GetDivision(); ++i)
					{
						float wtf = (float)(1.0f / meteoriteBehaviour->GetDivision());
						float angle = 20 + i * wtf * 360.0f;
						meteoriteBehaviour->create(c2.gameObject->position, c2.gameObject->size.x * 0.75f, angle, meteoriteBehaviour->GetSpeed() * 1.25);
					}
				}

				App->modNetServer->totalMeteorites--;

				NetworkDestroy(c2.gameObject);
			}

			float size = gameObject->size.x + 50.0f * Random.next();
			vec2 position = gameObject->position + 50.0f * vec2{ Random.next() - 0.5f, Random.next() - 0.5f };

			if (hitPoints <= 0)
			{
				// Centered big explosion
				size = 250.0f + 100.0f * Random.next();
				position = gameObject->position;

				gameObject->position = { 2000,2000 };
				gameObject->hasTeleported = true;

				isDead = true;

				//NetworkDestroy(gameObject);
			}

			CreateExplotion(position, size);

			// NOTE(jesus): Only played in the server right now...
			// You need to somehow make this happen in clients
			App->modSound->playAudioClip(App->modResources->audioClipExplosion);

			secondsSinceHit = 0.0f;
		}
	}
}

void Spaceship::write(OutputMemoryStream & packet)
{
	packet << hitPoints;
	packet << advanceSpeed;
	packet << rotateSpeed;
	packet << MAX_HIT_POINTS;
	packet << score;
	packet << isDead;
}

void Spaceship::read(const InputMemoryStream & packet)
{
	packet >> hitPoints;
	packet >> advanceSpeed;
	packet >> rotateSpeed;
	packet >> MAX_HIT_POINTS;
	packet >> score;
	packet >> isDead;
}

void Spaceship::Respawn()
{
	if (isServer)
	{
		gameObject->position = { 0,0 };
		gameObject->collider->type = ColliderType::Player;
		gameObject->hasTeleported = false;
		//gameObject->networkInterpolationEnabled = true;
		ResetHealth();

		isDead = false;

		secondsSinceHit = 0.0f;

		NetworkUpdate(gameObject);
	}
}

void Meteorite::create(vec2 spawnPos, float size, float ang, float speed)
{
	GameObject* meteor = NetworkInstantiate();

	meteor->position = spawnPos;
	meteor->angle = ang;
	meteor->size = { size, size };

	meteor->sprite = App->modRender->addSprite(meteor);
	meteor->sprite->order = 5;
	meteor->sprite->texture = App->modResources->asteroid1;

	Meteorite* meteroriteBehaviour = App->modBehaviour->addMeteorite(meteor);
	meteroriteBehaviour->isServer = isServer;
	meteroriteBehaviour->speed = speed;
	meteroriteBehaviour->currentLevel = currentLevel + 1;

	// Create collider
	meteor->collider = App->modCollision->addCollider(ColliderType::Meteorite, meteor);
	meteor->collider->isTrigger = true;


}

void Meteorite::start()
{
	gameObject->networkInterpolationEnabled = false;
}

void Meteorite::update()
{
	gameObject->position += vec2FromDegrees(gameObject->angle) * speed * Time.deltaTime;

	if (isServer)
	{
		const float neutralTimeSeconds = 0.1f;
		if (secondsSinceCreation > neutralTimeSeconds&& gameObject->collider == nullptr) {
			gameObject->collider = App->modCollision->addCollider(ColliderType::Meteorite, gameObject);
		}

		float mid_w = Window.width * 0.5f;
		float mid_h = Window.height * 0.5f;

		if (gameObject->position.x < -mid_w)
		{
			gameObject->position.x = gameObject->final_position.x = mid_w - 20;
			gameObject->hasTeleported = true;
			NetworkUpdate(gameObject);
		}
		else if (gameObject->position.x > mid_w)
		{
			gameObject->position.x = gameObject->final_position.x = -mid_w + 20;
			gameObject->hasTeleported = true;
			NetworkUpdate(gameObject);
		}
		else if (gameObject->position.y > mid_h)
		{
			gameObject->position.y = gameObject->final_position.y = -mid_h + 20;
			gameObject->hasTeleported = true;
			NetworkUpdate(gameObject);
		}
		else if (gameObject->position.y < -mid_h)
		{
			gameObject->position.y = gameObject->final_position.y = mid_h - 20;
			gameObject->hasTeleported = true;
			NetworkUpdate(gameObject);
		}


	}
}

void Meteorite::write(OutputMemoryStream& packet)
{
	packet << speed;
	packet << currentLevel;
}

void Meteorite::read(const InputMemoryStream& packet)
{
	packet >> speed;
	packet >> currentLevel;
}

void Meteorite::onCollisionTriggered(Collider& c1, Collider& c2)
{
	if (c2.type == ColliderType::Laser)
	{
		if (isServer)
		{
			NetworkDestroy(c2.gameObject); // Destroy the laser

			if (currentHitPoints < maxHitPoints)
			{
				currentHitPoints++;
				NetworkUpdate(gameObject);
			}

			float size = 100 + 50.0f * Random.next();
			vec2 position = gameObject->position + 50.0f * vec2{ Random.next() - 0.5f, Random.next() - 0.5f };

			if (currentHitPoints >= maxHitPoints)
			{
				// Centered big explosion
				CreateExplotion(position, size);

				if (App->modGameObject->spaceship01 && App->modGameObject->spaceship01->tag == c2.gameObject->tag)
				{
					Spaceship* spaceshipBehaviour = (Spaceship*)(App->modGameObject->spaceship01->behaviour);
					spaceshipBehaviour->AddScore(100);
				}
				else if (App->modGameObject->spaceship02 && App->modGameObject->spaceship02->tag == c2.gameObject->tag)
				{
					Spaceship* spaceshipBehaviour = (Spaceship*)(App->modGameObject->spaceship02->behaviour);
					spaceshipBehaviour->AddScore(100);
				}

				if (currentLevel < maxLevel)
				{
					for (int i = 0; i < division; ++i)
					{
						float part = (float)(1.0f / division);
						float angle = 20 + i * part * 360.0f;
						create(gameObject->position, gameObject->size.x * 0.75f, angle, speed * 1.25);
					}
				}
				App->modNetServer->totalMeteorites--;

				NetworkDestroy(gameObject);
			}

			/*GameObject* explosion = NetworkInstantiate();
			explosion->position = position;
			explosion->size = vec2{ size, size };
			explosion->angle = 365.0f * Random.next();

			explosion->sprite = App->modRender->addSprite(explosion);
			explosion->sprite->texture = App->modResources->explosion1;
			explosion->sprite->order = 100;

			explosion->animation = App->modRender->addAnimation(explosion);
			explosion->animation->clip = App->modResources->explosionClip;

			NetworkDestroy(explosion, 2.0f);*/

			// NOTE(jesus): Only played in the server right now...
			// You need to somehow make this happen in clients
			App->modSound->playAudioClip(App->modResources->audioClipExplosion);
		}
	}
}

void Behaviour::CreateExplotion(const vec2& position, float size)
{
	GameObject* explosion = NetworkInstantiate();
	explosion->position = position;
	explosion->size = vec2{ size, size };
	explosion->angle = 365.0f * Random.next();

	explosion->sprite = App->modRender->addSprite(explosion);
	explosion->sprite->texture = App->modResources->explosion1;
	explosion->sprite->order = 100;

	explosion->animation = App->modRender->addAnimation(explosion);
	explosion->animation->clip = App->modResources->explosionClip;

	NetworkDestroy(explosion, 2.0f);
}
