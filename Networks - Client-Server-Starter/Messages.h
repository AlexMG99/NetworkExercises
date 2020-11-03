#pragma once

// Add as many messages as you need depending on the
// functionalities that you decide to implement.

enum class ClientMessage
{
	Hello,
	Welcome,
	Help,
	Ban,
	Kick,
	Whisper,
	Goodbye
};

enum class ServerMessage
{
	Welcome
};

enum class MessageType
{
	Default,
	Info,
	Message,
	Warning,
	Error
};

