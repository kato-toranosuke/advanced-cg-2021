#include "LoopSubdivision.h"
#include <unordered_map>

using namespace std;
using namespace glm;

void LoopSubdivision::subdivide(PolygonMesh &mesh, int nSubdiv)
{
	if (mesh.getVertices().empty() || mesh.getFaceIndices().empty())
	{
		std::cerr << __FUNCTION__ << ": mesh not ready" << std::endl;
		return;
	}

	mesh.triangulate();

	HalfEdge::Mesh _mesh;
	_mesh.build(mesh);

	for (int iter = 0; iter < nSubdiv; ++iter)
		_mesh = apply(_mesh);

	_mesh.restore(mesh);
	mesh.calcVertexNormals();
}

HalfEdge::Mesh LoopSubdivision::apply(HalfEdge::Mesh &mesh)
{
	//return mesh;	// TODO: delete this line

	HalfEdge::Mesh newMesh;

	const int nOldVertices = (int)mesh.vertices.size();
	const int nOldFaces = (int)mesh.faces.size();
	const int nOldHalfEdges = (int)mesh.halfEdges.size();

	// Step 0: allocate memory for even (i.e., old) vertices

	newMesh.vertices.reserve(nOldFaces + nOldHalfEdges);

	for (int vi = 0; vi < nOldVertices; ++vi)
		newMesh.addVertex();

	// Step 1: create odd (i.e., new) vertices by splitting half edges

	unordered_map<long, HalfEdge::Vertex *> newEdgeMidpointDict;
	unordered_map<long, pair<HalfEdge::HalfEdge *, HalfEdge::HalfEdge *>> newHalfEdgeDict;

	for (int hi = 0; hi < nOldHalfEdges; ++hi)
	{
		auto oldHE = mesh.halfEdges[hi];

		vec3 startVertexPosition = mesh.vertices[oldHE->pStartVertex->id]->position;
		vec3 endVertexPosition = mesh.vertices[oldHE->getEndVertex()->id]->position;

		HalfEdge::Vertex *edgeMidpoint = nullptr;

		if (oldHE->pPair == nullptr) // on boundary
		{
			edgeMidpoint = newMesh.addVertex();
			// TODO: calculate the position of edge midpoint
			edgeMidpoint->position = (startVertexPosition + endVertexPosition) / 2.f;
		}
		else
		{
			auto edgeMidpointIter = newEdgeMidpointDict.find(oldHE->pPair->id); // check if pair has been already registered

			if (edgeMidpointIter == newEdgeMidpointDict.end()) // not found
			{
				edgeMidpoint = newMesh.addVertex();
				// TODO: calculate the position of edge midpoint
				vec3 topVertexPosition = mesh.vertices[oldHE->pPrev->getStartVertex()->id]->position;
				vec3 bottomVertexPosition = mesh.vertices[oldHE->pPair->pNext->getEndVertex()->id]->position;
				edgeMidpoint->position = 3.f / 8.f * (startVertexPosition + endVertexPosition) + 1.f / 8.f * (topVertexPosition + bottomVertexPosition);
			}
			else // founded
			{
				// edgeMidpointInter->second???Vertex????????????????????????????????????
				edgeMidpoint = edgeMidpointIter->second;
			}
		}

		newEdgeMidpointDict[oldHE->id] = edgeMidpoint; // used in Step 3

		auto formerHE = newMesh.addHalfEdge();
		auto latterHE = newMesh.addHalfEdge();

		auto evenStartVertex = newMesh.vertices[oldHE->pStartVertex->id];
		auto evenEndVertex = newMesh.vertices[oldHE->getEndVertex()->id];

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

	// Step 2: update even (i.e., old) vertices

	for (int vi = 0; vi < nOldVertices; ++vi)
	{
		auto newVertex = newMesh.vertices[vi];
		const auto oldVertex = mesh.vertices[vi];
		const auto oldVertexPosition = oldVertex->position;

		// TODO: calculate the new vertex position
		// c.f., HalfEdge::Vertex::countValence() in HalfEdgeDataStructure.cpp
		int valence = 0; // ??????
		auto he = oldVertex->pHalfEdge;
		bool onBoundary = false; // ????????????????????????????????????

		std::cout << oldVertex->onBoundary() << endl;

		// ???????????????????????????????????????????????????
		vector<vec3> peripheral_vertices_pos; // ??????????????????????????????
		do
		{
			++valence;

			// pair??????????????????????????????
			if (he->pPair == nullptr)
			{
				onBoundary = true;
				break;
			}

			const auto next_he = he->pPair->pNext;
			peripheral_vertices_pos.push_back(next_he->getEndVertex()->position);
			he = next_he;
		} while (he != oldVertex->pHalfEdge);

		// ???????????????????????????????????????
		if (onBoundary)
		{
			he = oldVertex->pHalfEdge->pPrev;

			do
			{
				++valence;

				if (he->pPair == nullptr)
					break;

				const auto prev_he = he = he->pPair->pPrev;
				peripheral_vertices_pos.push_back(prev_he->getStartVertex()->position);
				he = prev_he;
			} while (he != oldVertex->pHalfEdge);
		}

		// ??????????????????????????????
		// regular????????????Loop Subdivision????????????????????????????????????????????????6???regular???
		if (valence == 6)
		{
			// ??????????????????????????????
			if (onBoundary)
			{
				newVertex->position = 3.f / 4.f * oldVertexPosition + 1.f / 8.f * oldVertex->pHalfEdge->getEndVertex()->position + 1.f / 8.f * oldVertex->pHalfEdge->pPrev->pStartVertex->position;
				std::cout << "boundary" << endl;
			}
			// ????????????????????????????????????
			else
			{
				// ?????????
				newVertex->position = 10.f / 16.f * oldVertexPosition;
				// ?????????
				for (auto itr = peripheral_vertices_pos.cbegin(); itr != peripheral_vertices_pos.cend(); ++itr)
				{
					newVertex->position += 1.f / 16.f * (*itr);
				}
			}
		}
		// extraordinary?????????
		else
		{
			const float beta = (valence == 3) ? 3.f / 16.f : 3.f / (8.f * valence);
			// ?????????
			newVertex->position = (1 - valence * beta) * oldVertexPosition;
			// ?????????
			for (auto itr = peripheral_vertices_pos.cbegin(); itr != peripheral_vertices_pos.cend(); ++itr)
			{
				newVertex->position += beta * (*itr);
			}
		}
	}

	// Step 3: create new faces

	for (int fi = 0; fi < nOldFaces; ++fi)
	{
		auto oldFace = mesh.faces[fi];

		// TODO: update the half-edge data structure within each old face
		// HINT: the number of new faces within each old face is always 4 in the case of Loop subdivision,
		//       so you can write down all the steps without using a "for" or "while" loop

		// ???????????????????????????HalfEdge?????????
		vector<HalfEdge::HalfEdge *> pOldHalfEdges(3);
		pOldHalfEdges[0] = oldFace->pHalfEdge;
		pOldHalfEdges[1] = oldFace->pHalfEdge->pNext;
		pOldHalfEdges[2] = oldFace->pHalfEdge->pPrev;

		// ?????????HalfEdge?????????
		vector<HalfEdge::HalfEdge *> pNewHalfEdges(12);
		// ?????????????????????????????????HalfEndge?????????
		for (size_t i = 0; i < 3; i++)
		{
			auto newPair = newHalfEdgeDict.find(pOldHalfEdges[i]->id)->second;
			pNewHalfEdges[2 * i] = newPair.first;
			pNewHalfEdges[2 * i + 1] = newPair.second;
		}

		// ?????????HalfEndge??????????????????
		pNewHalfEdges[6] = newMesh.addHalfEdge();
		pNewHalfEdges[7] = newMesh.addHalfEdge();
		pNewHalfEdges[8] = newMesh.addHalfEdge();
		pNewHalfEdges[9] = newMesh.addHalfEdge();
		pNewHalfEdges[10] = newMesh.addHalfEdge();
		pNewHalfEdges[11] = newMesh.addHalfEdge();

		// ?????????Face??????????????????
		vector<HalfEdge::Face *> pNewFaces(4);
		for (auto itr = pNewFaces.begin(); itr != pNewFaces.end(); ++itr)
		{
			*itr = newMesh.addFace();
		}

		// HalfEdge Class??????????????????????????????
		// ????????????Face???set
		pNewHalfEdges[0]->pFace = pNewHalfEdges[6]->pFace = pNewHalfEdges[5]->pFace = pNewFaces[0];
		pNewHalfEdges[9]->pFace = pNewHalfEdges[10]->pFace = pNewHalfEdges[11]->pFace = pNewFaces[1];
		pNewHalfEdges[1]->pFace = pNewHalfEdges[2]->pFace = pNewHalfEdges[7]->pFace = pNewFaces[2];
		pNewHalfEdges[3]->pFace = pNewHalfEdges[4]->pFace = pNewHalfEdges[8]->pFace = pNewFaces[3];

		// ????????????????????????????????????HalfEdge
		pNewHalfEdges[0]->pPrev = pNewHalfEdges[5];
		pNewHalfEdges[1]->pPrev = pNewHalfEdges[7];
		pNewHalfEdges[2]->pPrev = pNewHalfEdges[1];
		pNewHalfEdges[3]->pPrev = pNewHalfEdges[8];
		pNewHalfEdges[4]->pPrev = pNewHalfEdges[3];
		pNewHalfEdges[5]->pPrev = pNewHalfEdges[6];
		pNewHalfEdges[6]->pPrev = pNewHalfEdges[0];
		pNewHalfEdges[7]->pPrev = pNewHalfEdges[2];
		pNewHalfEdges[8]->pPrev = pNewHalfEdges[4];
		pNewHalfEdges[9]->pPrev = pNewHalfEdges[11];
		pNewHalfEdges[10]->pPrev = pNewHalfEdges[9];
		pNewHalfEdges[11]->pPrev = pNewHalfEdges[10];

		// ???????????????????????????????????????HalfEdge
		pNewHalfEdges[0]->pNext = pNewHalfEdges[6];
		pNewHalfEdges[1]->pNext = pNewHalfEdges[2];
		pNewHalfEdges[2]->pNext = pNewHalfEdges[7];
		pNewHalfEdges[3]->pNext = pNewHalfEdges[4];
		pNewHalfEdges[4]->pNext = pNewHalfEdges[8];
		pNewHalfEdges[5]->pNext = pNewHalfEdges[0];
		pNewHalfEdges[6]->pNext = pNewHalfEdges[5];
		pNewHalfEdges[7]->pNext = pNewHalfEdges[1];
		pNewHalfEdges[8]->pNext = pNewHalfEdges[3];
		pNewHalfEdges[9]->pNext = pNewHalfEdges[10];
		pNewHalfEdges[10]->pNext = pNewHalfEdges[11];
		pNewHalfEdges[11]->pNext = pNewHalfEdges[9];

		// ???????????????Vertex??????newHalfEdge?????????
		pNewHalfEdges[6]->pStartVertex = newEdgeMidpointDict.find(pOldHalfEdges[0]->id)->second;
		pNewHalfEdges[7]->pStartVertex = newEdgeMidpointDict.find(pOldHalfEdges[1]->id)->second;
		pNewHalfEdges[8]->pStartVertex = newEdgeMidpointDict.find(pOldHalfEdges[2]->id)->second;
		pNewHalfEdges[9]->pStartVertex = newEdgeMidpointDict.find(pOldHalfEdges[2]->id)->second;
		pNewHalfEdges[10]->pStartVertex = newEdgeMidpointDict.find(pOldHalfEdges[0]->id)->second;
		pNewHalfEdges[11]->pStartVertex = newEdgeMidpointDict.find(pOldHalfEdges[1]->id)->second;

		// ????????????????????????????????????HalfEdge??????newHalfEdge?????????
		pNewHalfEdges[6]->pPair = pNewHalfEdges[9];
		pNewHalfEdges[7]->pPair = pNewHalfEdges[10];
		pNewHalfEdges[8]->pPair = pNewHalfEdges[11];
		pNewHalfEdges[9]->pPair = pNewHalfEdges[6];
		pNewHalfEdges[10]->pPair = pNewHalfEdges[7];
		pNewHalfEdges[11]->pPair = pNewHalfEdges[8];

		// Face Class???????????????????????????
		pNewFaces[0]->pHalfEdge = pNewHalfEdges[0];
		pNewFaces[1]->pHalfEdge = pNewHalfEdges[9];
		pNewFaces[2]->pHalfEdge = pNewHalfEdges[1];
		pNewFaces[3]->pHalfEdge = pNewHalfEdges[3];
	}

	cerr << __FUNCTION__ << ": check data consistency" << endl;
	newMesh.checkDataConsistency();

	return move(newMesh);
}
