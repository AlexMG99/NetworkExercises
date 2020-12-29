#pragma once


enum class BehaviourType : uint8;

struct Behaviour
{
	GameObject *gameObject = nullptr;
	bool isServer = false;
	bool isLocalPlayer = false;

	virtual BehaviourType type() const = 0;

	virtual void start() { }

	virtual void onInput(const InputController &input) { }

	virtual void update() { }

	virtual void destroy() { }

	virtual void onCollisionTriggered(Collider &c1, Collider &c2) { }

	virtual void write(OutputMemoryStream &packet) { }

	virtual void read(const InputMemoryStream &packet) { }
};


enum class BehaviourType : uint8
{
	None,
	Spaceship,
	Laser,
	Meteorite
};


struct Laser : public Behaviour
{
	float secondsSinceCreation = 0.0f;

	BehaviourType type() const override { return BehaviourType::Laser; }

	void start() override;

	void update() override;
};

struct Meteorite : public Behaviour
{
	float secondsSinceCreation = 0.0f;

	BehaviourType type() const override { return BehaviourType::Meteorite; }

	void create(vec2 spawnPos, float size, float ang, float speed);

	void start() override;

	void update() override;

	void write(OutputMemoryStream& packet) override;

	void read(const InputMemoryStream& packet) override;

	void onCollisionTriggered(Collider& c1, Collider& c2) override;

private:
	int maxHitPoints = 1;
	int currentHitPoints = 0;

	int division = 3;
	int currentLevel = 0;
	int maxLevel = 3;
	float speed = 100.0f;
};


struct Spaceship : public Behaviour
{
	static const uint8 MAX_HIT_POINTS = 5;
	uint8 hitPoints = MAX_HIT_POINTS;

	GameObject *lifebar = nullptr;

	BehaviourType type() const override { return BehaviourType::Spaceship; }

	void start() override;

	void onInput(const InputController &input) override;

	void update() override;

	void destroy() override;

	void onCollisionTriggered(Collider &c1, Collider &c2) override;

	void write(OutputMemoryStream &packet) override;

	void read(const InputMemoryStream &packet) override;
};
