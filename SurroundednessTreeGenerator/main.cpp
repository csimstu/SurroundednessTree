#include "bitmap_utils.h"
#include "SurroundednessTreeGenerator.h"
#include "FreeImage.h"

const string IMG_FILE = "seth.jpg";

void draw_rectangle(FIBITMAP *dib, int min_x, int min_y, int max_x, int max_y, Color color) {
	RGBQUAD quad;
	quad.rgbRed = color.r;
	quad.rgbBlue = color.b;
	quad.rgbGreen = color.g;
	min_x -= 2;
	min_y -= 2;
	for (int y = min_y; y <= max_y; y++) {
		FreeImage_SetPixelColor(dib, min_x, y, &quad);
		FreeImage_SetPixelColor(dib, max_x, y, &quad);
	}
	for (int x = min_x; x <= max_x; x++) {
		FreeImage_SetPixelColor(dib, x, min_y, &quad);
		FreeImage_SetPixelColor(dib, x, max_y, &quad);
	}
}


int main() {
	FreeImage_Initialise();
	FIBITMAP *dib;
	auto img = bitmap_convert(bitmap_wrap(read_bitmap(IMG_FILE.c_str(), &dib)));
	int n = (int)img.size(), m = (int)img[0].size();
	SurroundednessTreeGenerator gen(n, m);
	for (int i = 0; i < n; i++)
		for (int j = 0; j < m; j++) {
			//cout << "(" + to_string(i) + "," + to_string(j) + ")\n";
			gen.process(img[i][j]);
			//gen.printTree();
		}
	ofstream flog("log.txt");
	vector<RenderInfo> renderInfo;
	stringstream latex_pre, latex_tree;
	gen.printTree(flog, renderInfo, latex_pre, latex_tree, n * m);
	for (auto info : renderInfo) {
		draw_rectangle(dib, info.min_x, info.min_y, info.max_x, info.max_y, info.color);
	}
	flog << latex_pre.str() << endl;
	flog << latex_tree.str() << endl;
	FreeImage_Save(FIF_PNG, dib, "result.png");
	FreeImage_Unload(dib);
	FreeImage_DeInitialise();
}