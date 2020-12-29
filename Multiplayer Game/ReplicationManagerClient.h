#pragma once

// TODO(you): World state replication lab session

class ReplicationManagerClient
{
public:
	void read(const InputMemoryStream& packet);

	void createObject(const InputMemoryStream& packet, GameObject* go);

	//More shit...
};