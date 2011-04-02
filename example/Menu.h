#ifndef __EXAMPLE_MENU_H__
#define __EXAMPLE_MENU_H__

#ifdef __cplusplus
extern "C" {
#endif

#define MenuItem_MaxTextLength 64

typedef struct MenuItem
{
	char text[MenuItem_MaxTextLength];
	float* valuePtr;
	float upperBound;
	float lowerBound;
	float increment;
	int* optionPtr;
	const char** options;
	int optionCount;
	struct MenuItem* _next;
	struct MenuItem* _prev;
	
} MenuItem;

MenuItem* MenuItem_new(float* valuePtr);
void MenuItem_free(MenuItem* self);

void MenuItem_setText(MenuItem* item, const char* text);
void MenuItem_setBounds(MenuItem* item, float lowerBound, float upperBound, float increment);
void MenuItem_setValue(MenuItem* item, float value);
float MenuItem_getValue(MenuItem* item);
void MenuItem_increaseValue(MenuItem* item);
void MenuItem_decreaseValue(MenuItem* item);

typedef struct Menu
{
	MenuItem* firstItem;
	MenuItem* currentItem;
	int windowWidth;
	int windowHeight;
} Menu;

Menu* Menu_new();
void Menu_free(Menu* menu);

void Menu_debugItems(Menu* menu);
void Menu_addItem(Menu* menu, MenuItem* item);
void Menu_selectNextItem(Menu* menu);
void Menu_selectPrevItem(Menu* menu);

typedef void (*Menu_DrawItemCallback)(float, float, int, const char*);

void Menu_draw(Menu* menu, float x, float y, int numItems, Menu_DrawItemCallback drawItem);

#ifdef __cplusplus
}
#endif

#endif	// __EXAMPLE_MENU_H__
