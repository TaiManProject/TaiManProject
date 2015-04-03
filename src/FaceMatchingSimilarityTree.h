#pragma once

#include "commonTool.h"

struct SimilarityNode {
  double similarity;
  double confidence;
  SimilarityNode *left;
  SimilarityNode *right;
};

class FaceMatchingSimilarityTree {
public:
    FaceMatchingSimilarityTree(std::vector<double> data, double similarityThreshold);
    void buildTree(std::vector<double> data);
	void computeSimilarityAndConfidence(double &similarity, double &confidence);

private:
    SimilarityNode *root;
	double similarityThreshold;

	void destroyTree(SimilarityNode *leaf);
	void computeSimilarityAndConfidence(SimilarityNode* node, double &similarity, double &confidence);

};

