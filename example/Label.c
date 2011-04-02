#include "Label.windows.h"
#include "../lib/crender/Texture.h"

Label* Label_alloc()
{
	Label* self;
	CrAllocWithImpl(self, Label, LabelImpl);
	return self;
}

void Label_free(Label* self)
{
	if(nullptr != self->impl->text) {
		free(self->impl->text);
	}

	crTextureFree(self->texture);
	DeleteObject(self->impl->hbmp);
	DeleteDC(self->impl->hdc);
	free(self);
}

void Label_init(Label* self, size_t width, size_t height)
{
	self->impl->width = width;
	self->impl->height = height;
	self->impl->hdc = CreateCompatibleDC(nullptr);
	self->impl->hbmp = CreateBitmap(width, height, 1, 32, nullptr);
	self->texture = crTextureAlloc();
	crTextureInit(self->texture, width, height, 1, 1, CrGpuFormat_UnormR8);
	SelectObject(self->impl->hdc, self->impl->hbmp);
}

void Label_setText(Label* self, const char* text)
{
	size_t len;
	if(nullptr != self->impl->text) {
		free(self->impl->text);
	}

	len = (size_t)MultiByteToWideChar(CP_UTF8, 0, text, -1, nullptr, 0);
	self->impl->text = malloc(len * sizeof(wchar_t));
	MultiByteToWideChar(CP_UTF8, 0, text, -1, self->impl->text, len);
}

void Label_setFont(Label* self, const char* font)
{
}

 int GetBytesPerPixel(int depth)
{
    return (depth==32 ? 4 : 3);
}

int GetBytesPerRow(int width, int depth)
{
    int bytesPerPixel = GetBytesPerPixel(depth);
    int bytesPerRow = ((width * bytesPerPixel + 3) & ~3);

    return bytesPerRow;
}

int GetBitmapBytes(int width, int height, int depth)
{
    return height * GetBytesPerRow(width, depth);
}

int GetBitmapBytesS(const BITMAPINFOHEADER *bmih)
{
    return GetBitmapBytes(bmih->biWidth, bmih->biHeight, bmih->biBitCount);
}


void Label_commit(Label* self)
{
	RECT rect;
	HDC hdc = self->impl->hdc;

	if(nullptr == self->impl->text)
		return;

	SetRect(&rect, 0, 0, self->impl->width, self->impl->height);
	FillRect(hdc, &rect, GetStockObject(BLACK_BRUSH));
	SetTextColor(hdc, RGB(255,255,255));
	SetBkMode(hdc, TRANSPARENT);
    DrawTextW(hdc, self->impl->text, -1, &rect, DT_NOCLIP);

	{
		unsigned char* bits;
		BITMAPINFO* bi;

		bi = malloc(sizeof(BITMAPINFOHEADER)+ 256*sizeof(RGBQUAD));
		memset(&bi->bmiHeader, 0, sizeof(BITMAPINFOHEADER));
		bi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);

		GetDIBits(hdc, self->impl->hbmp, 0, self->impl->height, nullptr, bi, DIB_RGB_COLORS);

		bits = malloc(GetBitmapBytesS(&bi->bmiHeader));

		if(GetDIBits(hdc, self->impl->hbmp, 0, self->impl->height, bits, bi, DIB_RGB_COLORS) == self->impl->height) {
			
			unsigned char* srcPixel = bits;
			unsigned char* dstPixel = self->texture->data;
			size_t i;
			size_t pixelCnt = (size_t)(bi->bmiHeader.biWidth * bi->bmiHeader.biHeight);

			for(i = 0; i < pixelCnt; ++i) {
				*dstPixel = *srcPixel;
				dstPixel += 1;
				srcPixel += 4;
			}
			
			crTextureCommit(self->texture);
		}
		free(bits);
		free(bi);
	}
}