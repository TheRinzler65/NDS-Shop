
#ifndef _UNIVERSAL_CORE_GUI_HPP
#define _UNIVERSAL_CORE_GUI_HPP

#include "screen.hpp"

#include <3ds.h>
#include <citro2d.h>
#include <citro3d.h>

namespace Gui {
	
	void clearTextBufs(void);

	void DrawSprite(C2D_SpriteSheet sheet, size_t imgindex, int x, int y, float ScaleX = 1, float ScaleY = 1);

	Result init(CFG_Region fontRegion = CFG_REGION_USA);

	void loadSystemFont(CFG_Region fontRegion = CFG_REGION_USA);

	Result loadFont(C2D_Font &fnt, const char *Path = "");

	Result unloadFont(C2D_Font &fnt);

	Result loadSheet(const char *Path, C2D_SpriteSheet &sheet);

	Result unloadSheet(C2D_SpriteSheet &sheet);

	void exit(void);

	Result reinit(CFG_Region fontRegion = CFG_REGION_USA);

	void DrawStringCentered(float x, float y, float size, u32 color, const std::string &Text, int maxWidth = 0, int maxHeight = 0, C2D_Font fnt = nullptr, int flags = 0);

	void DrawString(float x, float y, float size, u32 color, const std::string &Text, int maxWidth = 0, int maxHeight = 0, C2D_Font fnt = nullptr, int flags = 0);

	float GetStringWidth(float size, const std::string &Text, C2D_Font fnt = nullptr);

	void GetStringSize(float size, float *width, float *height, const std::string &Text, C2D_Font fnt = nullptr);

	float GetStringHeight(float size, const std::string &Text, C2D_Font fnt = nullptr);

	bool Draw_Rect(float x, float y, float w, float h, u32 color);

	void DrawScreen(bool stack = false);

	#ifdef UC_KEY_REPEAT
	void ScreenLogic(u32 hDown, u32 hDownRepeat, u32 hHeld, touchPosition touch, bool waitFade = true, bool stack = false);
	#else
	void ScreenLogic(u32 hDown, u32 hHeld, touchPosition touch, bool waitFade = true, bool stack = false);
	#endif

	void transferScreen(bool stack = false);

	void setScreen(std::unique_ptr<Screen> screen, bool fade = false, bool stack = false);

	void fadeEffects(int fadeoutFrames = 6, int fadeinFrames = 6, bool stack = false);

	void screenBack(bool fade = false);
	void screenBack2();

	void ScreenDraw(C3D_RenderTarget * screen);

	void drawGrid(float xPos, float yPos, float Width, float Height, u32 color, u32 bgColor = C2D_Color32(0, 0, 0, 0));

	void drawAnimatedSelector(float xPos, float yPos, float Width, float Height, float speed, u32 SelectorColor, u32 bgColor = C2D_Color32(0, 0, 0, 0));
};

#endif