#include "CatmullClarkSubdivision.h"
#include <unordered_map>

using namespace std;
using namespace glm;

void CatmullClarkSubdivision::subdivide(PolygonMesh &mesh, int nSubdiv)
{
	if (mesh.getVertices().empty() || mesh.getFaceIndices().empty())
	{
		std::cerr << __FUNCTION__ << ": mesh not ready" << std::endl;
		return;
	}

	HalfEdge::Mesh _mesh;
	_mesh.build(mesh);

	for (int iter = 0; iter < nSubdiv; ++iter)
		_mesh = apply(_mesh);

	_mesh.restore(mesh);
	mesh.calcVertexNormals();
}

HalfEdge::Mesh CatmullClarkSubdivision::apply(HalfEdge::Mesh &mesh)
{
	// return mesh;	// TODO: delete this line

	HalfEdge::Mesh newMesh;

	const int nOldVertices = (int)mesh.vertices.size();
	const int nOldFaces = (int)mesh.faces.size();
	const int nOldHalfEdges = (int)mesh.halfEdges.size();

	// Step 0: allocate memory for even (i.e., old) vertices

	newMesh.vertices.reserve(nOldFaces + nOldVertices + nOldHalfEdges);

	for (int vi = 0; vi < nOldVertices; ++vi)
		newMesh.addVertex();

	// Step 1: generate face centroids
	unordered_map<long, HalfEdge::Vertex *> newCentroidDict;
	for (int fi = 0; fi < nOldFaces; ++fi)
	{
		auto oldFace = mesh.faces[fi];
		auto newFaceCentroid = newMesh.addVertex();
		// TODO: calculate the positions of face centroids
		newFaceCentroid->position = oldFace->calcCentroidPosition();
		newCentroidDict[oldFace->id] = newFaceCentroid;
	}

	// Step 2: create odd (i.e., new) vertices by splitting half edges

	unordered_map<long, HalfEdge::Vertex *> newEdgeMidpointDict;
	unordered_map<long, pair<HalfEdge::HalfEdge *, HalfEdge::HalfEdge *>> newHalfEdgeDict;

	for (int hi = 0; hi < nOldHalfEdges; ++hi)
	{
		auto oldHE = mesh.halfEdges[hi];

		vec3 startVertexPosition = oldHE->getStartVertex()->position;
		vec3 endVertexPosition = oldHE->getEndVertex()->position;

		HalfEdge::Vertex *edgeMidpoint = nullptr;

		if (oldHE->pPair == nullptr) // on boundary
		{
			edgeMidpoint = newMesh.addVertex();
			// TODO: calculate the position of edge midpoint
			edgeMidpoint->position = 1.f / 2.f * (startVertexPosition + endVertexPosition);
		}
		else
		{
			auto edgeMidpointIter = newEdgeMidpointDict.find(oldHE->pPair->id); // check if pair has been already registered

			if (edgeMidpointIter == newEdgeMidpointDict.end())
			{
				edgeMidpoint = newMesh.addVertex();
				// TODO: calculate the position of edge midpoint
				edgeMidpoint->position = 1.f / 4.f * (startVertexPosition + endVertexPosition + oldHE->pFace->calcCentroidPosition() + oldHE->pPair->pFace->calcCentroidPosition());
			}
			else
			{
				edgeMidpoint = edgeMidpointIter->second;
			}
		}

		newEdgeMidpointDict[oldHE->id] = edgeMidpoint;

		auto formerHE = newMesh.addHalfEdge();
		auto latterHE = newMesh.addHalfEdge();

		auto evenStartVertex = newMesh.vertices[oldHE->pStartVertex->id];
		auto evenEndVertex = newMesh.vertices[oldHE->pNext->pStartVertex->id];

		formerHE->pStartVertex = evenStartVertex;
		if (evenStartVertex->pHalfEdge == nullptr)
			evenStartVertex->pHalfEdge = formerHE;

		latterHE->pStartVertex = edgeMidpoint;
		if (edgeMidpoint->pHalfEdge == nullptr)
			edgeMidpoint->pHalfEdge = latterHE;

		newHalfEdgeDict[hi] = make_pair(formerHE, latterHE);

		// register pairs

		if (oldHE->pPair != nullptr)
		{
			auto iter = newHalfEdgeDict.find(oldHE->pPair->id);

			if (iter != newHalfEdgeDict.end())
			{
				HalfEdge::HalfEdge *pairFormerHE = iter->second.first;
				HalfEdge::HalfEdge *pairLatterHE = iter->second.second;

				HalfEdge::Helper::SetPair(pairFormerHE, latterHE);
				HalfEdge::Helper::SetPair(pairLatterHE, formerHE);
			}
		}
	}

	// Step 3: update even (i.e., old) vertex positions

	for (int vi = 0; vi < nOldVertices; ++vi)
	{
		auto newVertex = newMesh.vertices[vi];
		const auto oldVertex = mesh.vertices[vi];
		const auto oldVertexPosition = oldVertex->position;

		// TODO: calculate the new vertex position
		// c.f., HalfEdge::Vertex::countValence() in HalfEdgeDataStructure.cpp

		int valence = 0;
		bool onBoundary = false;
		auto he = oldVertex->pHalfEdge;

		vector<HalfEdge::Vertex *> midpoints; // 中点を格納
		vector<HalfEdge::Vertex *> centroids; // 重心を格納

		do
		{
			++valence;

			// 中点を取得
			auto midpoint = newEdgeMidpointDict.find(he->id)->second;
			midpoints.push_back(midpoint);
			// 重心を取得
			auto centroid = newCentroidDict.find(he->pFace->id)->second;
			centroids.push_back(centroid);

			if (he->pPair == nullptr)
			{
				onBoundary = true;
				break;
			}

			he = he->pPair->pNext;
		} while (he != oldVertex->pHalfEdge);

		if (onBoundary)
		{
			he = oldVertex->pHalfEdge->pPrev;

			do
			{
				++valence;

				auto midpoint = newEdgeMidpointDict.find(he->id)->second;
				midpoints.push_back(midpoint);

				auto centroid = newCentroidDict.find(he->pFace->id)->second;
				centroids.push_back(centroid);

				if (he->pPair == nullptr)
				{
					break;
				}

				he = he->pPair->pPrev;
			} while (he != oldVertex->pHalfEdge);
		}

		// even vertexの座標を計算
		// 境界線上にある場合
		if (onBoundary)
		{
			newVertex->position = 3.f / 4.f * oldVertexPosition + 1.f / 8.f * (oldVertex->pHalfEdge->getEndVertex()->position + oldVertex->pHalfEdge->pPrev->pStartVertex->position);
		}
		// 境界線以外にある場合
		else
		{
			vec3 R, S; // R:辺の中点の平均座標 ,S:面の重心の平均座標
			for (size_t i = 0; i < valence; i++)
			{
				R += midpoints[i]->position;
				S += centroids[i]->position;
			}
			R /= (float)valence;
			S /= (float)valence;

			newVertex->position = (valence - 3.f) / valence * oldVertexPosition + 4.f / valence * R - 1.f / valence * S;
		}
	}

	// Step 4: set up new faces

	for (int fi = 0; fi < nOldFaces; ++fi)
	{
		auto oldFace = mesh.faces[fi];
		auto centroidVertex = newMesh.vertices[oldFace->id + nOldVertices];

		// TODO: update the half-edge data structure within each old face
		// HINT: use the following std::vector to store temporal data and process step by step
		vector<HalfEdge::HalfEdge *> tmpToCentroidHalfEdges;
		vector<HalfEdge::Face *> tmpNewFaces;

		// faceの設定
		auto he = oldFace->pHalfEdge;
		int k = 0;
		do
		{
			auto newFace = newMesh.addFace();

			tmpNewFaces.push_back(newFace);

			he = he->pNext;
			k++;
		} while (he != oldFace->pHalfEdge);

		// HalfEdgeの設定(同一面内)(ここが問題)
		he = oldFace->pHalfEdge;
		int i = 0;
		do
		{
			// 中点→重心に向かうhalf edgeの追加
			auto toCentroidHalfEdge = newMesh.addHalfEdge();
			// 重心→中点に向かうhalf edgeの追加
			auto toMidpointHalfEdge = newMesh.addHalfEdge();

			// 始点となるvertexを設定
			// 中心→重心
			// auto midpoint = newEdgeMidpointDict.find(he->pPrev->id)->second;
			auto midpoint = newEdgeMidpointDict.find(he->id)->second;
			toCentroidHalfEdge->pStartVertex = midpoint;
			// 重心→中心
			toMidpointHalfEdge->pStartVertex = centroidVertex;

			// 自身が所属するFace
			auto pNewFace = tmpNewFaces[i];
			toCentroidHalfEdge->pFace = pNewFace;
			toMidpointHalfEdge->pFace = pNewFace;

			auto he1 = newHalfEdgeDict.find(he->id)->second.first;
			auto he2 = newHalfEdgeDict.find(he->pPrev->id)->second.second;
			he1->pFace = pNewFace;
			he2->pFace = pNewFace;

			// 中点・重心の頂点に関してその頂点を始点とするいずれかのHEを登録
			midpoint->pHalfEdge = toCentroidHalfEdge;
			centroidVertex->pHalfEdge = toMidpointHalfEdge;

			// pNext
			toCentroidHalfEdge->pNext = toMidpointHalfEdge;
			toMidpointHalfEdge->pNext = he2;
			he2->pNext = he1;
			he1->pNext = toCentroidHalfEdge;

			// pPrev
			toCentroidHalfEdge->pPrev = he1;
			toMidpointHalfEdge->pPrev = toCentroidHalfEdge;
			he2->pPrev = toMidpointHalfEdge;
			he1->pPrev = he2;

			// 一時保存配列への追加
			tmpToCentroidHalfEdges.push_back(toCentroidHalfEdge);

			// faceに含まれるいずれかのHalfEdgeの一本を登録
			pNewFace->pHalfEdge = toMidpointHalfEdge;

			he = he->pNext;
			i++;
		} while (he != oldFace->pHalfEdge);

		// pairの設定
		auto olfHE = oldFace->pHalfEdge;
		for (int i = 0; i < k; i++)
		{
			auto toMidpointHE = tmpNewFaces[i]->pHalfEdge;

			int j = (i == 0) ? (k - 1) : (i - 1);
			auto toCentroidHE = tmpToCentroidHalfEdges[j];

			toMidpointHE->pPair = toCentroidHE;
			toCentroidHE->pPair = toMidpointHE;
		}
	}

	std::cerr << __FUNCTION__ << ": check data consistency" << endl;
	newMesh.checkDataConsistency();

	return move(newMesh);
}
