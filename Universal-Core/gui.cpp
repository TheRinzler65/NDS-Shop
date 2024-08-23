
#include "gui.hpp"
#include "screenCommon.hpp"

#include <3ds.h>
#include <stack>
#include <unistd.h>
#include <vector>

C3D_RenderTarget *Top, *TopRight, *Bottom;

C2D_TextBuf TextBuf;
C2D_Font Font;
std::unique_ptr<Screen> usedScreen, tempScreen;
std::stack<std::unique_ptr<Screen>> screens;
bool currentScreen = false;
bool fadeout = false, fadein = false, fadeout2 = false, fadein2 = false;
int fadealpha = 0;
int fadecolor = 0;
CFG_Region loadedSystemFont = (CFG_Region)-1;

void Gui::clearTextBufs(void) { C2D_TextBufClear(TextBuf); };


void Gui::DrawSprite(C2D_SpriteSheet sheet, size_t imgindex, int x, int y, float ScaleX, float ScaleY) {
	if (sheet) {
		if (C2D_SpriteSheetCount(sheet) >= imgindex) {
			C2D_DrawImageAt(C2D_SpriteSheetGetImage(sheet, imgindex), x, y, 0.5f, nullptr, ScaleX, ScaleY);
		}
	}
}


Result Gui::init(CFG_Region fontRegion) {
	C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
	C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
	C2D_Prepare();

	/* Create Screen Targets. */
	Top = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
	TopRight = C2D_CreateScreenTarget(GFX_TOP, GFX_RIGHT);
	Bottom = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);

	/* Load Textbuffer. */
	TextBuf = C2D_TextBufNew(4096);
	loadSystemFont(fontRegion);
	return 0;
}

/
void Gui::loadSystemFont(CFG_Region fontRegion) {
	if(loadedSystemFont != fontRegion) {
		Font = C2D_FontLoadSystem(fontRegion);
		loadedSystemFont = fontRegion;
	}
}


Result Gui::loadFont(C2D_Font &fnt, const char *Path) {
	if (access(Path, F_OK) == 0) fnt = C2D_FontLoad(Path); // Only load if found.

	return 0;
}


Result Gui::unloadFont(C2D_Font &fnt) {
	if (fnt) C2D_FontFree(fnt); // Make sure to only unload if not nullptr.

	return 0;
}


Result Gui::loadSheet(const char *Path, C2D_SpriteSheet &sheet) {
	if (access(Path, F_OK) == 0) sheet = C2D_SpriteSheetLoad(Path); // Only load if found.

	return 0;
}


Result Gui::unloadSheet(C2D_SpriteSheet &sheet) {
	if (sheet) C2D_SpriteSheetFree(sheet); // Make sure to only unload if not nullptr.

	return 0;
}


void Gui::exit(void) {
	C2D_TextBufDelete(TextBuf);
	C2D_Fini();
	C3D_Fini();
	if (usedScreen) usedScreen = nullptr;
}


Result Gui::reinit(CFG_Region fontRegion) {
	C2D_TextBufDelete(TextBuf);
	C2D_Fini();
	C3D_Fini();

	return Gui::init(fontRegion);
}


void Gui::DrawStringCentered(float x, float y, float size, u32 color, const std::string &Text, int maxWidth, int maxHeight, C2D_Font fnt, int flags) {
	Gui::DrawString(x +(currentScreen ? 200 : 160), y, size, color, Text, maxWidth, maxHeight, fnt, flags | C2D_AlignCenter);
}


void Gui::DrawString(float x, float y, float size, u32 color, const std::string &Text, int maxWidth, int maxHeight, C2D_Font fnt, int flags) {
	C2D_Text c2d_text;

	if (fnt) C2D_TextFontParse(&c2d_text, fnt, TextBuf, Text.c_str());
	else C2D_TextFontParse(&c2d_text, Font, TextBuf, Text.c_str());

	C2D_TextOptimize(&c2d_text);

	if(!fnt) {
		switch(loadedSystemFont) {
			case CFG_REGION_CHN:
			case CFG_REGION_KOR:
			case CFG_REGION_TWN:
				y += 3.0f * size;
				break;
			default:
				break;
		}
	}

	float heightScale;

	if (maxHeight == 0) {
		heightScale = size;

	} else {
		if (fnt) heightScale = std::min(size, size*(maxHeight/Gui::GetStringHeight(size, Text, fnt)));
		else heightScale = std::min(size, size*(maxHeight/Gui::GetStringHeight(size, Text)));
	}

	if (maxWidth == 0) {
		C2D_DrawText(&c2d_text, C2D_WithColor | flags, x, y, 0.5f, size, heightScale, color);
	} else if (flags & C2D_WordWrap) {
		C2D_DrawText(&c2d_text, C2D_WithColor | flags, x, y, 0.5f, size, heightScale, color, (float)maxWidth);
	} else {
		if (fnt) C2D_DrawText(&c2d_text, C2D_WithColor | flags, x, y, 0.5f, std::min(size, size*(maxWidth/Gui::GetStringWidth(size, Text, fnt))), heightScale, color);
		else C2D_DrawText(&c2d_text, C2D_WithColor | flags, x, y, 0.5f, std::min(size, size*(maxWidth/Gui::GetStringWidth(size, Text))), heightScale, color);
	}
}


float Gui::GetStringWidth(float size, const std::string &Text, C2D_Font fnt) {
	float width = 0;

	if (fnt) GetStringSize(size, &width, NULL, Text, fnt);
	else GetStringSize(size, &width, NULL, Text);

	return width;
}


void Gui::GetStringSize(float size, float *width, float *height, const std::string &Text, C2D_Font fnt) {
	C2D_Text c2d_text;

	if (fnt) C2D_TextFontParse(&c2d_text, fnt, TextBuf, Text.c_str());
	else C2D_TextFontParse(&c2d_text, Font, TextBuf, Text.c_str());

	C2D_TextGetDimensions(&c2d_text, size, size, width, height);
}



float Gui::GetStringHeight(float size, const std::string &Text, C2D_Font fnt) {
	float height = 0;

	if (fnt) GetStringSize(size, NULL, &height, Text, fnt);
	else GetStringSize(size, NULL, &height, Text);

	return height;
}


bool Gui::Draw_Rect(float x, float y, float w, float h, u32 color) {
	return C2D_DrawRectSolid(x, y, 0.5f, w, h, color);
}


void Gui::DrawScreen(bool stack) {
	if (!stack) {
		if (usedScreen) usedScreen->Draw();

	} else {
		if (!screens.empty()) screens.top()->Draw();
	}
}


#ifdef UC_KEY_REPEAT
void Gui::ScreenLogic(u32 hDown, u32 hDownRepeat, u32 hHeld, touchPosition touch, bool waitFade, bool stack) {
	if (waitFade) {
		if (!fadein && !fadeout && !fadein2 && !fadeout2) {
			if (!stack) {
				if (usedScreen)	usedScreen->Logic(hDown, hDownRepeat, hHeld, touch);

			} else {
				if (!screens.empty()) screens.top()->Logic(hDown, hDownRepeat, hHeld, touch);
			}
		}

	} else {
		if (!stack) {
			if (usedScreen)	usedScreen->Logic(hDown, hDownRepeat, hHeld, touch);

		} else {
			if (!screens.empty()) screens.top()->Logic(hDown, hDownRepeat, hHeld, touch);
		}
	}
}
#else
void Gui::ScreenLogic(u32 hDown, u32 hHeld, touchPosition touch, bool waitFade, bool stack) {
	if (waitFade) {
		if (!fadein && !fadeout && !fadein2 && !fadeout2) {
			if (!stack) {
				if (usedScreen)	usedScreen->Logic(hDown, hHeld, touch);

			} else {
				if (!screens.empty()) screens.top()->Logic(hDown, hHeld, touch);
			}
		}

	} else {
		if (!stack) {
			if (usedScreen)	usedScreen->Logic(hDown, hHeld, touch);

		} else {
			if (!screens.empty()) screens.top()->Logic(hDown, hHeld, touch);
		}
	}
}
#endif

void Gui::transferScreen(bool stack) {
	if (!stack) {
		if (tempScreen) usedScreen = std::move(tempScreen);

	} else {
		if (tempScreen) screens.push(std::move(tempScreen));
	}
}

void Gui::setScreen(std::unique_ptr<Screen> screen, bool fade, bool stack) {
	tempScreen = std::move(screen);

	/* Switch screen without fade. */
	if (!fade) {
		Gui::transferScreen(stack);

	} else {
		/* Fade, then switch. */
		fadeout = true;
	}
}

void Gui::fadeEffects(int fadeoutFrames, int fadeinFrames, bool stack) {
	if (fadein) {
		fadealpha -= fadeinFrames;

		if (fadealpha < 0) {
			fadealpha = 0;
			fadecolor = 255;
			fadein = false;
		}
	}

	if (stack) {
		if (fadein2) {
			fadealpha -= fadeinFrames;

			if (fadealpha < 0) {
				fadealpha = 0;
				fadecolor = 255;
				fadein2 = false;
			}
		}
	}

	if (fadeout) {
		fadealpha += fadeoutFrames;

		if (fadealpha > 255) {
			fadealpha = 255;
			Gui::transferScreen(stack);
			fadein = true;
			fadeout = false;
		}
	}

	if (stack) {
		if (fadeout2) {
			fadealpha += fadeoutFrames;

			if (fadealpha > 255) {
				fadealpha = 255;
				Gui::screenBack2();
				fadein2 = true;
				fadeout2 = false;
			}
		}
	}
}


void Gui::screenBack(bool fade) {
	if (!fade) {
		if (screens.size() > 0) screens.pop();

	} else {
		if (screens.size() > 0) fadeout2 = true;
	}
}
void Gui::screenBack2() { if (screens.size() > 0) screens.pop(); };


void Gui::ScreenDraw(C3D_RenderTarget *screen) {
	C2D_SceneBegin(screen);
	currentScreen = (screen == Top || screen == TopRight) ? 1 : 0;
}


void Gui::drawGrid(float xPos, float yPos, float Width, float Height, u32 color, u32 bgColor) {
	static constexpr int w	= 1;

	/* BG Color for the Grid. (Transparent by default.) */
	Gui::Draw_Rect(xPos, yPos, Width, Height, bgColor);

	/* Grid part, Top. */
	Gui::Draw_Rect(xPos, yPos, Width, w, color);
	/* Left. */
	Gui::Draw_Rect(xPos, yPos + w, w, Height - 2 * w, color);
	/* Right. */
	Gui::Draw_Rect(xPos + Width - w, yPos + w, w, Height - 2 * w, color);
	/* Bottom. */
	Gui::Draw_Rect(xPos, yPos + Height - w, Width, w, color);
}


void Gui::drawAnimatedSelector(float xPos, float yPos, float Width, float Height, float speed, u32 SelectorColor, u32 bgColor) {
	static constexpr int w		= 2;
	static float timer			= 0.0f;
	float highlight_multiplier  = fmax(0.0, fabs(fmod(timer, 1.0) - 0.5) / 0.5);
	u8 r						= SelectorColor & 0xFF;
	u8 g						= (SelectorColor >> 8) & 0xFF;
	u8 b						= (SelectorColor >> 16) & 0xFF;
	u32 color 					= C2D_Color32(r + (255 - r) * highlight_multiplier, g + (255 - g) * highlight_multiplier, b + (255 - b) * highlight_multiplier, 255);

	/* BG Color for the Selector. */
	Gui::Draw_Rect(xPos, yPos, Width, Height, bgColor);

	/* Selector part, Top. */
	Gui::Draw_Rect(xPos, yPos, Width, w, color);
	/* Left. */
	Gui::Draw_Rect(xPos, yPos + w, w, Height - 2 * w, color);
	/* Right. */
	Gui::Draw_Rect(xPos + Width - w, yPos + w, w, Height - 2 * w, color);
	/* Bottom. */
	Gui::Draw_Rect(xPos, yPos + Height - w, Width, w, color);

	timer += speed;
}