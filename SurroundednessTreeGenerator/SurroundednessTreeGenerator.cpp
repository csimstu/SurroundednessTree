#include "SurroundednessTreeGenerator.h"
#include <algorithm>
#include <cassert>
#include <iostream>
#include <fstream>

int id = 0;

ConnectedComponent::ConnectedComponent(int val)
	:val(val), area(0), sum_x(0), sum_y(0) {
	min_x = min_y = INT_MAX;
	max_x = max_y = INT_MIN;
	if (val == 0)
		name = to_string(id++) + "(0)";
	else
		name = to_string(id++) + "(1)";
}

void ConnectedComponent::extendAt(int i, int j) {
	if (j % 2 == 0) return;
	j /= 2;
	area++;
	sum_x += j;
	sum_y += i;
	min_x = min(min_x, j);
	min_y = min(min_y, i);
	max_x = max(max_x, j);
	max_y = max(max_y, i);
}

void ConnectedComponent::addChild(const comp_ptr &child) {
	child->setParent(shared_from_this());
	children.insert(child);
}

void ConnectedComponent::setParent(const comp_ptr &parent) {
	this->parent = parent;
}

void ConnectedComponent::removeFromTree() {
	parent->children.erase(parent->children.find(shared_from_this()));
	parent = nullptr;
}

void ConnectedComponent::absorb(const comp_ptr &other) {
	for (auto x : other->children)
		addChild(x);
	area += other->area;
	sum_x += other->sum_x;
	sum_y += other->sum_y;
	min_x = min(min_x, other->min_x);
	min_y = min(min_y, other->min_y);
	max_x = max(max_x, other->max_x);
	max_y = max(max_y, other->max_y);
}

bool ConnectedComponent::isAncestorOf(const comp_ptr &other) const {
	comp_ptr cur = other;
	while (cur->parent) {
		if (cur->parent.get() == this)
			return true;
		cur = cur->parent;
	}
	return false;
}

string createNewColorName() {
	static int id = 0;
	return "tmpcolor" + to_string(id++);
}

#define AREA_THRESHOLD 520

void ConnectedComponent::print(ofstream &fout, vector<RenderInfo> &render_info,
	stringstream &latex_pre, stringstream &latex_tree, int totalArea, int level) const {
	if (area < 10) return;
	fout << name + " with area " + to_string(area) + ", center of gravity (" + to_string((double)sum_x / area) + "," + to_string((double)sum_y / area) + "),";
	fout << "enclosed by ";
	fout << "(" << min_x << "," << min_y << ") -- (" << max_x << "," << max_y << ")\n";

	Color newColor = Color::randomColor();
	render_info.emplace_back(newColor, min_x, min_y, max_x, max_y);
	string color_name = createNewColorName();
	if (area > AREA_THRESHOLD - level * 160) {
		latex_pre << "\\definecolor{" << color_name << "}{RGB}{" << newColor.r << "," << newColor.g << "," << newColor.b << "}\n";
		latex_tree << "node [arn_r," << color_name << ", scale=" << 1.5 * pow((float)area / totalArea, 0.2);
		if (level & 1) {
			latex_tree << ", pattern=north east lines";
		}
		latex_tree << "] {}";
	}

	if (children.size() == 0) {
		return;
	}
	for (auto x : children) {
		fout << " " + x->name;
	}
	fout << "\n";
	for (auto x : children) {
		if (x->area >= 10 && x->area > AREA_THRESHOLD - (level+1) * 160)
			latex_tree << " child{";
		x->print(fout, render_info, latex_pre, latex_tree, totalArea, level + 1);
		if (x->area >= 10 && x->area > AREA_THRESHOLD - (level+1) * 160)
			latex_tree << "} ";
	}
}

SurroundednessTreeGenerator::SurroundednessTreeGenerator(int nrow, int ncol)
	:nrow(nrow), ncol(ncol) {
	root.reset(new ConnectedComponent(0));
	for (int i = 0; i < ncol; i++)
		cut.push_back(root);
	cur_row = cur_col = 0;
}

void SurroundednessTreeGenerator::process(int val) {
	comp_ptr left_comp = cur_col == 0 ? root : cut[cur_col - 1];
	comp_ptr up_comp = cut[cur_col];

	if (left_comp->getVal() == up_comp->getVal()) {
		if (val != left_comp->getVal()) {
			// new component
			comp_ptr new_comp(new ConnectedComponent(val));
			new_comp->extendAt(cur_row, cur_col);
			left_comp->addChild(new_comp);
			cut[cur_col] = new_comp;
		}
		else {
			left_comp->extendAt(cur_row, cur_col);
			if (left_comp == up_comp) {
			}
			else {
				merge(left_comp, up_comp);
			}
		}
	}
	else {
		if (left_comp->getVal() == val) {
			left_comp->extendAt(cur_row, cur_col);
			cut[cur_col] = left_comp;
		}
		else {
			up_comp->extendAt(cur_row, cur_col);
			cut[cur_col] = up_comp;
		}
	}

	cur_col++;
	if (cur_col == ncol) {
		cur_col = 0;
		cur_row++;
	}
}

void SurroundednessTreeGenerator::merge(const comp_ptr &comp1,
	const comp_ptr &comp2) {
	assert(comp1 != comp2);
	if (comp1->isAncestorOf(comp2)) {
		mergeInto(comp2, comp1);
	}
	else {
		assert(comp1->getParent() == comp2->getParent() || comp2->isAncestorOf(comp1));
		mergeInto(comp1, comp2);
	}
}

void SurroundednessTreeGenerator::mergeInto(const comp_ptr &src,
	const comp_ptr &des) {
	src->removeFromTree();
	des->absorb(src);
	for (auto &x : cut)
		if (x == src)
			x = des;
}

void SurroundednessTreeGenerator::printTree(ofstream &fout, vector<RenderInfo> &render_info, stringstream &latex_pre, stringstream &latex_tree, int totalArea) {
	latex_tree << "\\";
	root->print(fout, render_info, latex_pre, latex_tree, totalArea, 0);
	latex_tree << ";";
}