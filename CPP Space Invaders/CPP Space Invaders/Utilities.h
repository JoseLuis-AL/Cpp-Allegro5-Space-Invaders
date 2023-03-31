#pragma once
#include <iostream>

namespace Utilities
{
	/**
	 * @brief Ensure 'test' is initialized.
	 * @param test
	 * @param description
	*/
	void MustInit(bool test, const char* description)
	{
		if (test) return;

		std::cout << "ERROR: Couldn't initialize " << description << "\n";
		exit(1);
	}

	/**
	 * @brief Check the collision between two rectangles (Bounding box).
	 * @param ax1
	 * @param ay1
	 * @param ax2
	 * @param ay2
	 * @param bx1
	 * @param by1
	 * @param bx2
	 * @param by2
	 * @return True if boxes collide, false otherwise.
	*/
	bool CheckCollision(int ax1, int ay1, int ax2, int ay2, int bx1, int by1, int bx2, int by2)
	{
		if (ax1 > bx2) return false;
		if (ax2 < bx1) return false;
		if (ay1 > by2) return false;
		if (ay2 < by1) return false;

		return true;
	}
}