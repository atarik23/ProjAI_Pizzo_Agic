#pragma once

struct RectCordinates {
	float top;
	float bottom;
	float left;
	float right;
	RectCordinates() :top(0), bottom(0), left(0), right(0) {}
	RectCordinates(float t, float b, float l, float r) : top(t), bottom(b), left(l), right(r) {}
};