#include "FaceMatchingSimilarityTree.h"

FaceMatchingSimilarityTree::FaceMatchingSimilarityTree(std::vector<double> data, double similarityThreshold) {
	this->root=NULL;
	this->similarityThreshold = similarityThreshold;
	buildTree(data);
}

void FaceMatchingSimilarityTree::buildTree(std::vector<double> data){
	//root Node first level 0
	SimilarityNode* node = new SimilarityNode;
	node->similarity = data.at(0);
	node->confidence = 1.0;
	root = node;

	//level 1
	root->left = new SimilarityNode;
	root->left->similarity = data.at(1);
	root->left->confidence = 0.5;

	root->right = new SimilarityNode;
	root->right->similarity = data.at(2);
	root->right->confidence = 0.5;

	//level 2
	root->left->left = new SimilarityNode;
	root->left->left->similarity = data.at(3);
	root->left->left->confidence = 0.25;

	root->left->right = new SimilarityNode;
	root->left->right->similarity = data.at(4);
	root->left->right->confidence = 0.25;

	root->right->left = new SimilarityNode;
	root->right->left->similarity = data.at(5);
	root->right->left->confidence = 0.25;

	root->right->right = new SimilarityNode;
	root->right->right->similarity = data.at(6);
	root->right->right->confidence = 0.25;

	//level 3
	root->left->left->left = new SimilarityNode;
	root->left->left->left->similarity = data.at(7);
	root->left->left->left->confidence = 0.125;
	root->left->left->left->left = NULL;
	root->left->left->left->right = NULL;


	root->left->left->right = new SimilarityNode;
	root->left->left->right->similarity = data.at(9);
	root->left->left->right->confidence = 0.125;
	root->left->left->right->left = NULL;
	root->left->left->right->right = NULL;

	root->left->right->left = new SimilarityNode;
	root->left->right->left->similarity = data.at(8);
	root->left->right->left->confidence = 0.125;
	root->left->right->left->left = NULL;
	root->left->right->left->right = NULL;

	root->left->right->right = new SimilarityNode;
	root->left->right->right->similarity = data.at(10);
	root->left->right->right->confidence = 0.125;
	root->left->right->right->left = NULL;
	root->left->right->right->right = NULL;

	root->right->left->left = new SimilarityNode;
	root->right->left->left->similarity = data.at(11);
	root->right->left->left->confidence = 0.125;
	root->right->left->left->left = NULL;
	root->right->left->left->right = NULL;

	root->right->left->right = new SimilarityNode;
	root->right->left->right->similarity = data.at(13);
	root->right->left->right->confidence = 0.125;
	root->right->left->right->left = NULL;
	root->right->left->right->right = NULL;

	root->right->right->left = new SimilarityNode;
	root->right->right->left->similarity = data.at(12);
	root->right->right->left->confidence = 0.125;
	root->right->right->left->left = NULL;
	root->right->right->left->right = NULL;

	root->right->right->right = new SimilarityNode;
	root->right->right->right->similarity = data.at(14);
	root->right->right->right->confidence = 0.125;
	root->right->right->right->left = NULL;
	root->right->right->right->right = NULL;
}

void FaceMatchingSimilarityTree::computeSimilarityAndConfidence(double &similarity, double &confidence){
	computeSimilarityAndConfidence(root, similarity, confidence);
}

void FaceMatchingSimilarityTree::computeSimilarityAndConfidence(SimilarityNode* node, double &similarity, double &confidence){
	if (node->left == NULL || node->right == NULL){
		similarity = node->similarity;
		confidence = node->confidence;
		return;
    }

	if (node->similarity > similarityThreshold){
		similarity = node->similarity;
		confidence = node->confidence;
		return;
	} else {
		double leftSimilarity, leftConfidence;
		double rightSimilarity, rightConfidence;
		computeSimilarityAndConfidence(node->left, leftSimilarity, leftConfidence);
		computeSimilarityAndConfidence(node->right, rightSimilarity, rightConfidence);

		if (leftSimilarity < similarityThreshold){
			leftSimilarity = 0.0;
			leftConfidence = 0.0;
		}

		if (rightSimilarity < similarityThreshold){
			rightSimilarity = 0.0;
			rightConfidence = 0.0;
		}
		
		double sumOfConfidence = leftConfidence + rightConfidence;

		if (sumOfConfidence < 0.00001){ // sumOfConfidence == 0
			similarity = 0;
			confidence = 0;
		} else {
			similarity = leftSimilarity * (leftConfidence / sumOfConfidence) + rightSimilarity * (rightConfidence / sumOfConfidence);
			confidence = leftConfidence + rightConfidence;
		}
		return;
	}
}
