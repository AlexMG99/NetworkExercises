#pragma once

#include "imgui/imgui.h"

#define MAX_COLORS 7

ImVec4 White	= ImVec4(1, 1, 1, 1);
ImVec4 Blue		= ImVec4(0, 0, 1, 1);
ImVec4 Red		= ImVec4(1, 0, 0, 1);
ImVec4 FirebrickRed = ImVec4(0.7f, 0.13f, 0.13f, 1);
ImVec4 Green	= ImVec4(0, 1, 0, 1);
ImVec4 LimeGreen = ImVec4(0.19f, 0.8f, 0.19f, 1);
ImVec4 Yellow	= ImVec4(1, 1, 0, 1);
ImVec4 Orange	= ImVec4(1, 0.64f, 0, 1);
ImVec4 Purple	= ImVec4(0.62f, 0.13f, 0.94f, 1);
ImVec4 Pink		= ImVec4(1, 0.75f, 0, 1);

ImVec4 colors[MAX_COLORS] = { Blue, Red, Green, Yellow, Orange, Purple, Pink };