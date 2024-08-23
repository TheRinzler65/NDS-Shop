
#ifndef _UNIVERSAL_CORE_STRUCTS_HPP
#define _UNIVERSAL_CORE_STRUCTS_HPP

#include <string>
#include <3ds/types.h>
#include <3ds/services/hid.h>

class Structs {
public:
	struct ButtonPos {
		int x;
		int y;
		int w;
		int h;

		bool Touched(const touchPosition &T) const {
			return (T.px >= this->x && T.px <= (this->x + this->w)) && (T.py >= this->y && T.py <= (this->y + this->h));
		};
	};

	struct Key {
		std::string character;
		int x;
		int y;
		int w;
	};
};

#endif