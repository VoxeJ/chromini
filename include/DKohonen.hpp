#ifndef KOHONEN_HPP
#define KOHONEN_HPP

#include <vector>
#include <math.h>
#include <stdexcept>
#include <exception>
#include <chrono>
#include <random>
#include <algorithm>
#include <functional>
#include <thread>
#include <atomic>
#include <limits>

template<typename T>
class DKohonen {
private:
	std::function<double(const T&, const T&)> _metric;

	std::vector<T> _weights;

	double _eucDist(const std::vector<double>& x1, const std::vector<double>& x2) {
		if (x1.size() != x2.size()) {
			throw std::runtime_error("Invalid dimensions");
		}
		double sum = 0;
		for (size_t i = 0; i < x1.size(); i++) {
			sum += pow(x1[i] - x2[i], 2);
		}
		return sqrt(sum);
	}
	
	size_t _closestNodeInd(const T& data, double& minDist) {
		if (_weights.size() == 0) {
			throw std::runtime_error("Use of untrained network");
		}
		size_t minInd = 0;
		minDist = _metric(data, _weights[0]);
		for(int i = 1; i < _weights.size(); i++){
			double dist = _metric(data, _weights[i]);
			if(dist < minDist){
				minInd = i;
				minDist = dist;
			}
		}
		return minInd;
	}

	size_t _closestNodeInd(const T& data){
		double dist = 0;
		return _closestNodeInd(data, dist);
	}

	/*
	size_t _closestNodeInd(const std::vector<double>& data, double& dist) {
		size_t minInd = _closestNodeInd(data);
		dist = _metric(data, _weights[minInd]);
		return minInd;
	}*/

	T _closestNode(const T& data, double dist) {
		return _weights[_closestNodeInd(data, dist)];
	}

	T _closestNode(const T& data) {
		return _weights[_closestNodeInd(data)];
	}
	
	/*
	std::vector<double> _multiplyVector(std::vector<double> data, const double& multiplier) {
		for (double& elem : data) {
			elem *= multiplier;
		}
		return data;
	}

	std::vector<double> _sumVectors(std::vector<double> x1, const std::vector<double>& x2) {
		for (size_t i = 0; i < x1.size(); i++) {
			x1[i] += x2[i];
		}
		return x1;
	}
	*/

	void _removeOneRedundant(const double& minDist){
		for(size_t i = 0; i < _weights.size() - 1; i++){
			for(size_t j = i + 1; j < _weights.size(); j++){
				double interDist = _metric(_weights[i], _weights[j]);
				if(interDist < minDist){
					_weights[i] = (_weights[i]+_weights[j])/2;
					_weights.erase(_weights.begin() + j);
					return;
				}
			}
		}
	}

public:
	DKohonen() = default;
	DKohonen(std::function<double(const T&, const T&)> metric) : _metric(metric) {};

	void trainStep(const T& dataPiece, const size_t& maxClusters, const double& maxDistance, const double& minDist, const double& learningRate){
		if (_weights.size() == 0) {
				_weights.push_back(dataPiece);
				return;
			}
			double dist = 0;
			size_t clusterInd = _closestNodeInd(dataPiece, dist);
			if((_weights.size() == maxClusters) && (dist > maxDistance)){
				_removeOneRedundant(minDist);
			}
			if((_weights.size() == maxClusters) || (dist <= maxDistance)) {
				_weights[clusterInd] = _weights[clusterInd]*(1 - learningRate)+dataPiece*learningRate;
			}
			else if(dist >= minDist){
				_weights.push_back(dataPiece);
			}
	}

	void train(std::vector<T> trainData, const size_t& maxClusters, const double& maxDistance, const double& minDistance, const double& learningRate, std::mt19937& randEng) {
		std::shuffle(trainData.begin(), trainData.end(), randEng);
		for (const T& dataPiece : trainData) {
			trainStep(dataPiece, maxClusters, maxDistance, minDistance, learningRate);
		}
	}

	std::vector<unsigned char> getResultVec(const T& data) {
		if (_weights.size() == 0) {
			throw std::runtime_error("Use of untrained network");
		}
		if (data.size() != _weights[0].size()) {
			throw std::runtime_error("Invalid dimensions");
		}
		std::vector<unsigned char> ans(_weights.size(), 0);
		ans[_closestNodeInd(data)] = 1;
		return ans;
	}

	std::vector<double> getDistVec(const T& data){
		std::vector<double> dists;
		for(const T& weight : _weights){
			dists.push_back(_metric(data,weight));
		}
		return dists;
	}

	T getClosestGroup(const T& data) {
		return _closestNode(data);
	}

	std::vector<T> getGroups(){
		return _weights;
	}

	size_t closestGroupInd(const T& data){
		return _closestNodeInd(data);
	}
};

#endif