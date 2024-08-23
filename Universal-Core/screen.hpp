
#ifndef _UNIVERSAL_CORE_SCREEN_HPP
#define _UNIVERSAL_CORE_SCREEN_HPP

#include <3ds.h>
#include <memory>

class Screen {
public:
	virtual ~Screen() {}
#ifdef UC_KEY_REPEAT
	virtual void Logic(u32 hDown, u32 hDownRepeat, u32 hHeld, touchPosition touch) = 0;
#else
	virtual void Logic(u32 hDown, u32 hHeld, touchPosition touch) = 0;
#endif
	virtual void Draw() const = 0;
};

#endif