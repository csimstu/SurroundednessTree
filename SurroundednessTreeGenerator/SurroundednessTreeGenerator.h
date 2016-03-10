#ifndef SURROUDEDNESS_TREE_GENERATOR_H
#define SURROUDEDNESS_TREE_GENERATOR_H

#include <string>
#include <memory>
#include <vector>
#include <set>
#include <fstream>
#include <sstream>
using namespace std;

class ConnectedComponent;
typedef shared_ptr<ConnectedComponent> comp_ptr;

struct Color {
	int r, g, b;
	static Color randomColor() {
		// Use light color
		return Color{rand() % 128 + 128, rand() % 128 + 128, rand() % 128 + 128};
	}
};

struct RenderInfo {
	RenderInfo(Color _color, int _min_x, int _min_y, int _max_x, int _max_y)
		:color(_color), min_x(_min_x), min_y(_min_y), max_x(_max_x), max_y(_max_y) {}
	Color color;
	int min_x, min_y, max_x, max_y;
};

class ConnectedComponent : public enable_shared_from_this<ConnectedComponent> {
public:
	ConnectedComponent(int val);
	int getVal() const { return val; }
	void addChild(const comp_ptr &child);
	const comp_ptr &getParent() const { return parent; }
	void setParent(const comp_ptr &parent);
	void removeFromTree();
	void extendAt(int i, int j);
	void absorb(const comp_ptr &other);
	bool isAncestorOf(const comp_ptr &other) const;
	void print(ofstream &fout, vector<RenderInfo> &render_info, stringstream &latex_pre, stringstream &latex_tree) const;
private:
	int val; // 0: white, 1: black
	int area;
	int64_t sum_x, sum_y;
	int min_x, min_y, max_x, max_y;
	comp_ptr parent; // enclosed by
	set<comp_ptr> children; // enclosing
	string name;
};

class SurroundednessTreeGenerator {
public:
	SurroundednessTreeGenerator(int nrow, int ncol);
	void process(int val);
	void printTree(ofstream &fout, vector<RenderInfo> &render_info, stringstream &latex_pre, stringstream &latex_tree);

private:
	void merge(const comp_ptr &comp1,
		const comp_ptr &comp2);
	void mergeInto(const comp_ptr &src, const comp_ptr &des);
	int nrow, ncol;
	int cur_row, cur_col;
	vector<comp_ptr> cut;
	comp_ptr root;
};

#endif