#include "menu.h"
#include "../crender/Platform.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

MenuItem* MenuItem_new(float* valuePtr)
{
	MenuItem* self = (MenuItem*)malloc(sizeof(MenuItem));
	memset(self, 0, sizeof(MenuItem));
	self->valuePtr = valuePtr;
	return self;
}

void MenuItem_free(MenuItem* item)
{
	free(item);
}

void MenuItem_setText(MenuItem* item, const char* text)
{
	if(nullptr == item)
		return;
	
	if(strlen(text) >= MenuItem_MaxTextLength)
		return;
	
	strcpy(item->text, text);
}

void MenuItem_setBounds(MenuItem* item, float lowerBound, float upperBound, float increment)
{
	if(nullptr == item)
		return;
	
	// check if the bounds are valid
	if(lowerBound > upperBound)
	{
		printf("MenuItem_setBounds: %s lowerBound > upperBound!!\n", item->text);
		return;
	}
	
	item->lowerBound = lowerBound;
	item->upperBound = upperBound;
	item->increment = increment;
	
	// check the value
	if(nullptr == item->valuePtr)
		return;
	
	if(*item->valuePtr < lowerBound)
		*item->valuePtr = lowerBound;
	
	if(*item->valuePtr > upperBound)
		*item->valuePtr = upperBound;
}

void MenuItem_setValue(MenuItem* item, float value)
{
	if(nullptr == item)
		return;
	
	if(nullptr == item->valuePtr)
	{
		printf("MenuItem_setValue: %s valuePtr == nullptr!!\n", item->text);
		return;
	}
	
	if(value > item->upperBound || value < item->lowerBound)
	{
		printf("MenuItem_setValue: %s value out of bounds!!\n", item->text);
		return;
	}
	
	*item->valuePtr = value;
}

float MenuItem_getValue(MenuItem* item)
{
	if(nullptr == item)
		return 0;
	
	if(nullptr == item->valuePtr)
		return 0;
	
	return *item->valuePtr;
}

void MenuItem_increaseValue(MenuItem* item)
{
	if(nullptr == item)
		return;
	
	if(nullptr != item->valuePtr)
	{
		//printf("MenuItem_increaseValue: %s valuePtr == nullptr!!\n", item->text);
		*item->valuePtr += item->increment;
		
		if(*item->valuePtr < item->lowerBound)
			*item->valuePtr = item->lowerBound;
		
		if(*item->valuePtr > item->upperBound)
			*item->valuePtr = item->upperBound;
	}

	if(nullptr != item->optionPtr && nullptr != item->options)
	{
		++(*item->optionPtr);
		if((*item->optionPtr) >= item->optionCount)
			(*item->optionPtr) = 0;
	}
}

void MenuItem_decreaseValue(MenuItem* item)
{
	if(nullptr == item)
		return;
	
	if(nullptr != item->valuePtr)
	{
		//printf("MenuItem_decreaseValue: %s valuePtr == nullptr!!\n", item->text);
		*item->valuePtr -= item->increment;
		
		if(*item->valuePtr < item->lowerBound)
			*item->valuePtr = item->lowerBound;
		
		if(*item->valuePtr > item->upperBound)
			*item->valuePtr = item->upperBound;
	}

	if(nullptr != item->optionPtr && nullptr != item->options)
	{
		--(*item->optionPtr);
		if((*item->optionPtr) < 0)
			(*item->optionPtr) = item->optionCount-1;
	}
}

Menu* Menu_new()
{
	Menu* self = (Menu*)malloc(sizeof(Menu));
	memset(self, 0, sizeof(Menu));
	return self;
}

void Menu_free(Menu* self)
{
	MenuItem* curr = self->firstItem;
	MenuItem* next;
	while(nullptr != curr)
	{
		next = curr->_next;
		MenuItem_free(curr);
		curr = next;
	}
	
	free(self);
}

void Menu_debugItems(Menu* menu)
{
	MenuItem* curr = menu->firstItem;
	printf("Menu_debugItems:\n");
	while(nullptr != curr)
	{
		if(curr == menu->currentItem)
			printf("[%s], ", curr->text);
		else
			printf("%s, ", curr->text);
		
		curr = curr->_next;
	}
	printf("\n");
}

MenuItem* Menu_lastItem(Menu* menu)
{
	MenuItem* curr;

	if(nullptr == menu)
		return nullptr;
	
	if(nullptr == menu->firstItem)
		return nullptr;
	
	curr = menu->firstItem;
	
	while(nullptr != curr->_next)
		curr = curr->_next;
	
	return curr;
}

void Menu_addItem(Menu* menu, MenuItem* item)
{
	MenuItem* last;

	if(nullptr == menu)
		return;

	if(nullptr == item)
		return;

	last = Menu_lastItem(menu);
	if(nullptr == last)
	{
		// menu is empty add item as firstItem
		menu->firstItem = item;
	}
	else
	{
		// append to the last item
		last->_next = item;
		item->_prev = last;
	}
}

void Menu_selectNextItem(Menu* menu)
{
	if(nullptr == menu)
		return;
	
	if(nullptr == menu->currentItem)
	{	// no selection yet, select the first item and return
		menu->currentItem = menu->firstItem;
		return;
	}
	
	menu->currentItem = menu->currentItem->_next;
	
	// check if we have reached the end of the list
	if(nullptr == menu->currentItem)
		menu->currentItem = menu->firstItem;
}

void Menu_selectPrevItem(Menu* menu)
{
	if(nullptr == menu)
		return;
	
	if(nullptr == menu->currentItem)
	{	// no selection yet, select the first item and return
		menu->currentItem = Menu_lastItem(menu);
		return;
	}
	
	menu->currentItem = menu->currentItem->_prev;
	
	// check if we have reached the beginning of the list
	if(nullptr == menu->currentItem)
		menu->currentItem = Menu_lastItem(menu);
}

void Menu_draw(Menu* menu, float x, float y, int numItems, Menu_DrawItemCallback drawItem)
{
	MenuItem* curr;
	char buf[128];
	float y_inc;

	if(nullptr == menu)
		return;

	if(nullptr == drawItem)
		return;
	
	curr = menu->firstItem;
	
	y_inc = -25.0f / menu->windowHeight;
	y += y_inc;
	
	for(; nullptr != curr; curr = curr->_next)
	{
		if(nullptr != curr->valuePtr)
			sprintf(buf, "%s:\t%3.2f", curr->text, *curr->valuePtr);
		else if(nullptr != curr->options && nullptr != curr->optionPtr)
			sprintf(buf, "%s:\t%s", curr->text, curr->options[(*curr->optionPtr)]);

		drawItem(x, y, (curr == menu->currentItem) ? 1 : 0, buf);
		y += y_inc;
	}

}
