#ifndef BITMAP_UTILS_H
#define BITMAP_UTILS_H

#include <iostream>
#include <fstream>
#include <vector>
#include <cstdio>
#include <algorithm>
#include "FreeImage.h"
#include <cassert>
using namespace std;

typedef vector<vector<int>> grid_t;

grid_t read_bitmap(const char *filename, FIBITMAP **dib_result) {
	FIBITMAP *dib = FreeImage_Load(FIF_PNG, filename, PNG_DEFAULT);
	assert(dib);
	unsigned width = FreeImage_GetWidth(dib);
	unsigned height = FreeImage_GetHeight(dib);
	unsigned pitch = FreeImage_GetPitch(dib);
	grid_t result(height, vector<int>(width));
	FREE_IMAGE_TYPE image_type = FreeImage_GetImageType(dib);
	assert(image_type == FIT_BITMAP);

	if ((image_type == FIT_BITMAP) && (FreeImage_GetBPP(dib) == 24)) {
		BYTE *bits = (BYTE*)FreeImage_GetBits(dib);
		for (unsigned y = 0; y < height; y++) {
			BYTE *pixel = (BYTE*)bits;
			for (unsigned x = 0; x < width; x++) {
				result[y][x] = (pixel[FI_RGBA_RED] == 255 ? 0 : 1);
				pixel += 3;
			}
			// next line
			bits += pitch;
		}
	}
	if (dib_result) {
		*dib_result = dib;
	} else
		FreeImage_Unload(dib);
	return result;
}

void print_bitmap(const grid_t &img) {
	int n = (int)img.size(), m = (int)img[0].size();
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < m; j++)
			cout << img[i][j] << " ";
		cout << "\n";
	}
}


grid_t bitmap_or(const grid_t &A, const grid_t &B) {
	int ma = (int)A[0].size(), mb = (int)B[0].size();
	int n = (int)A.size(), m = max(ma, mb);
	grid_t result(n, vector<int>(m));
	for (int i = 0; i < n; i++)
		for (int j = 0; j < m; j++) {
			result[i][j] =
				(j < m - ma ? 0 : A[i][j - (m - ma)])
				| (j < m - mb ? 0 : B[i][j - (m - mb)]);
		}
	return result;
}

grid_t bitmap_lshift(const grid_t &A, int k) {
	int n = (int)A.size();
	grid_t result = A;
	for (int i = 0; i < n; i++)
		for (int j = 0; j < k; j++) {
			result[i].push_back(0);
		}
	return result;
}

grid_t bitmap_zip(const grid_t &A, const grid_t &B) {
	// result = A[0] B[0] A[1] B[1] ...
	grid_t result(A.size(), vector<int>());
	for (int i = 0; i < (int)A.size(); i++) {
		int pa = 0, pb = 0;
		for (int j = 0; j < (int)A[0].size() + (int)B[0].size(); j++)
			result[i].push_back(j & 1 ? B[i][pb++] : A[i][pa++]);
	}
	return result;
}

grid_t bitmap_convert(const grid_t &img) {
	return bitmap_zip(bitmap_or(bitmap_lshift(img, 1), img), img);
}

grid_t bitmap_wrap(const grid_t &img) {
	grid_t result = img;
	result.insert(result.begin(), vector<int>(img[0].size(), 0));
	result.push_back(vector<int>(img[0].size(), 0));
	for (auto &row : result) {
		row.insert(row.begin(), 0);
		row.push_back(0);
	}
	return result;
}

#endif