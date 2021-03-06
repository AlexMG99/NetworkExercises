#pragma once

enum class ClientMessage : uint8
{
	Hello,
	Input,
	Ping,   // NOTE(jesus): Use this message type in the virtual connection lab session
	Replication,
	StartGame,
	Respawn,
	LostGame,
	Disconnected,
	LostHP
};

enum class ServerMessage : uint8
{
	Welcome,
	Unwelcome,
	Input,
	Ping,   // NOTE(jesus): Use this message type in the virtual connection lab session
	Replication,
	LostHP,
	StartGame,
	FinishGame,
	Death
};
