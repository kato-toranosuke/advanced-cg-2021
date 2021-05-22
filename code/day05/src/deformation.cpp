/*! 
  @file deformation.cpp
	
	@brief 2Dメッシュ変形

  @author Makoto Fujisawa
  @date 2021-03
  */

//-----------------------------------------------------------------------------
// インクルードファイル
//-----------------------------------------------------------------------------
#include "deformation.h"

#include "rx_sampler.h"
#include "rx_delaunay.h"

//-----------------------------------------------------------------------------
// rxMeshDeform2Dクラスの実装
//-----------------------------------------------------------------------------
/*!
 * コンストラクタ
 * @param[in] n グリッド数
 */
rxMeshDeform2D::rxMeshDeform2D()
{
	m_iNv = m_iNt = m_iNfix = 0;

	m_vao_mesh = 0;
	m_vao_fix = 0;

	m_alpha = 1.0;
	m_deform_type = 0;

	Init(0);
}

/*!
 * デストラクタ
 */
rxMeshDeform2D::~rxMeshDeform2D()
{
	if (m_vao_mesh)
		glDeleteVertexArrays(1, &m_vao_mesh);
	if (m_vao_fix)
		glDeleteVertexArrays(1, &m_vao_fix);
}

/*!
 * メッシュ初期化
 */
void rxMeshDeform2D::Init(int random_mesh)
{
	// メッシュ作成
	glm::vec2 c1(-1.0, -1.0);
	glm::vec2 c2(1.0, 1.0);
	if (random_mesh)
		generateRandomMesh(c1, c2, 0.07, 800);
	else
		generateMesh(c1, c2, 32, 32);

	// 頂点配列オブジェクトの作成
	if (m_vao_mesh != 0)
		glDeleteVertexArrays(1, &m_vao_mesh);
	m_vao_mesh = CreateVAO((GLfloat *)&m_vX[0], m_iNv, 2, &m_vTri[0], m_iNt, 0, 0, 0, 0, (GLfloat *)&m_vTC[0], m_iNv);

	// 固定点の設定
	m_vFix.clear();
	m_iNfix = 0;
	updateFixVAO();
}

/*!
 * 近傍頂点探索
 *  - マウスピック用
 *  - 探索半径h内で最も近い点を返す
 * @param[in] pos 探索位置
 * @param[in] h 探索半径
 * @return 近傍頂点インデックス
 */
int rxMeshDeform2D::Search(glm::vec2 pos, double h)
{
	int idx = -1;
	double min_d2 = RX_FEQ_INF;
	double h2 = h * h;
	for (int i = 0; i < m_iNv; ++i)
	{
		double d2 = glm::length2(m_vX[i] - pos);
		if (d2 < h2 && d2 < min_d2)
		{
			min_d2 = d2;
			idx = i;
		}
	}
	return idx;
}
/*!
* 近傍固定頂点探索
*  - マウスピック用
*  - 探索半径h内で最も近い点を返す
* @param[in] pos 探索位置
* @param[in] h 探索半径
* @return 近傍頂点インデックス
*/
int rxMeshDeform2D::SearchFix(glm::vec2 pos, double h)
{
	int idx = -1;
	double min_d2 = RX_FEQ_INF;
	double h2 = h * h;
	for (int k = 0; k < m_iNfix; ++k)
	{
		int i = m_vFix[k];
		double d2 = glm::length2(m_vX[i] - pos);
		if (d2 < h2 && d2 < min_d2)
		{
			min_d2 = d2;
			idx = i;
		}
	}
	return idx;
}

//! 固定点座標VAOの更新
void rxMeshDeform2D::updateFixVAO(void)
{
	if (m_vFix.empty())
	{
		if (m_vao_fix != 0)
		{
			glDeleteVertexArrays(1, &m_vao_fix);
			m_vao_fix = 0;
		}
	}
	else
	{
		vector<glm::vec2> fixpos;
		for (int i : m_vFix)
			fixpos.push_back(m_vX[i]);
		if (m_vao_fix != 0)
			glDeleteVertexArrays(1, &m_vao_fix);
		m_vao_fix = CreateVAO((GLfloat *)&fixpos[0], m_iNfix, 2);
	}
}

//! 固定点設定
void rxMeshDeform2D::SetFix(int idx, glm::vec2 pos, bool move)
{
	if (std::find(m_vFix.begin(), m_vFix.end(), idx) == m_vFix.end())
	{
		m_vFix.push_back(idx);
		m_iNfix++;
		updateFixVAO();
	}
	else if (move)
	{
		m_vX[idx] = pos;
		updateFixVAO();
	}
}
//! 固定点解除
void rxMeshDeform2D::UnsetFix(int idx)
{
	if (std::find(m_vFix.begin(), m_vFix.end(), idx) != m_vFix.end())
	{
		std::remove(m_vFix.begin(), m_vFix.end(), idx);
		m_iNfix--;
		updateFixVAO();
	}
}

/*!
 * OpenGLによるメッシュ,頂点,固定頂点描画
 */
void rxMeshDeform2D::DrawMesh(void)
{
	UpdateDataVAO(m_vao_mesh, 0, (GLfloat *)&m_vX[0], 2 * m_iNv);
	glBindVertexArray(m_vao_mesh);
	glDrawElements(GL_TRIANGLES, m_vTri.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}
void rxMeshDeform2D::DrawPoints(void)
{
	UpdateDataVAO(m_vao_mesh, 0, (GLfloat *)&m_vX[0], 2 * m_iNv);
	glBindVertexArray(m_vao_mesh);
	glDrawArrays(GL_POINTS, 0, m_iNv);
	glBindVertexArray(0);
}
void rxMeshDeform2D::DrawFixPoints(void)
{
	glBindVertexArray(m_vao_fix);
	glDrawArrays(GL_POINTS, 0, m_iNfix);
	glBindVertexArray(0);
}
void rxMeshDeform2D::InitVAO(void)
{
	// 頂点配列オブジェクトの作成
	if (m_vao_mesh != 0)
		glDeleteVertexArrays(1, &m_vao_mesh);
	m_vao_mesh = CreateVAO((GLfloat *)&m_vX[0], m_iNv, 2, &m_vTri[0], m_iNt, 0, 0, 0, 0, (GLfloat *)&m_vTC[0], m_iNv);
}

/*!
 * n×nの頂点を持つメッシュ生成(x-z平面)
 * @param[in] c1,c2 2端点座標
 */
void rxMeshDeform2D::generateMesh(glm::vec2 c1, glm::vec2 c2, int nx, int ny)
{
	if (!m_vX.empty())
	{
		m_vX.clear();
		m_vTC.clear();
		m_vP.clear();
		m_vTri.clear();
	}

	// 頂点座標生成
	double dx = (c2[0] - c1[0]) / (nx - 1.0);
	double dz = (c2[1] - c1[1]) / (ny - 1.0);
	m_iNv = nx * ny;
	m_vX.resize(m_iNv);
	m_vTC.resize(m_iNv);
	m_vP.resize(m_iNv);
	for (int j = 0; j < ny; ++j)
	{
		for (int i = 0; i < nx; ++i)
		{
			glm::vec2 pos;
			pos[0] = c1[0] + i * dx;
			pos[1] = c1[1] + j * dz;

			int idx = IDX(i, j, nx);
			m_vX[idx] = m_vP[idx] = pos;
			m_vTC[idx] = glm::vec2((pos[0] + c1[0]) / (c2[0] - c1[0]), (pos[1] + c1[1]) / (c2[1] - c1[1]));
		}
	}

	// メッシュ作成
	for (int j = 0; j < ny - 1; ++j)
	{
		for (int i = 0; i < nx - 1; ++i)
		{
			m_vTri.push_back(IDX(i, j, nx));
			m_vTri.push_back(IDX(i + 1, j + 1, nx));
			m_vTri.push_back(IDX(i + 1, j, nx));

			m_vTri.push_back(IDX(i, j, nx));
			m_vTri.push_back(IDX(i, j + 1, nx));
			m_vTri.push_back(IDX(i + 1, j + 1, nx));
		}
	}

	m_iNt = (int)m_vTri.size() / 3;
}

/*!
 * 線分(を含む直線)と点の距離
 * @param[in] v0,v1 線分の両端点座標
 * @param[in] p 点の座標
 * @return 距離
 */
inline double segment_point_dist(const glm::vec2 &v0, const glm::vec2 &v1, const glm::vec2 &p, glm::vec2 &ip)
{
	glm::vec2 v = glm::normalize(v1 - v0);
	glm::vec2 vp = p - v0;
	glm::vec2 vh = glm::dot(vp, v) * v;
	ip = v0 + vh;
	return glm::length(vp - vh);
}

/*!
 * n×nの頂点を持つメッシュ生成(x-z平面)
 * @param[in] c1,c2 2端点座標
 */
void rxMeshDeform2D::generateRandomMesh(glm::vec2 c1, glm::vec2 c2, double min_dist, int n)
{
	if (!m_vX.empty())
	{
		m_vX.clear();
		m_vTC.clear();
		m_vP.clear();
		m_vTri.clear();
	}

	glm::vec2 minp = c1, maxp = c2;

	vector<glm::vec2> c(4);
	c[0] = glm::vec2(minp[0], minp[1]);
	c[1] = glm::vec2(maxp[0], minp[1]);
	c[2] = glm::vec2(maxp[0], maxp[1]);
	c[3] = glm::vec2(minp[0], maxp[1]);

	// 4隅の点を追加
	vector<glm::vec2> points;
	for (int i = 0; i < 4; ++i)
		points.push_back(c[i]);

	// 境界エッジ上にmin_distを基準に点を追加
	double d = 0.0;
	for (int j = 0; j < 4; ++j)
	{
		glm::vec2 v0 = c[j];
		glm::vec2 v1 = c[(j == 3 ? 0 : j + 1)];
		glm::vec2 edir = v1 - v0;
		double len = glm::length(edir);
		while (d < len)
		{
			double t = min_dist * RX_RAND(1.0, 1.4);
			d += t;
			if (len - d < 0.3 * min_dist)
				break;
			points.push_back(v0 + float(d) * glm::normalize(edir));
		}
		d = 0;
	}

	// ポアソンディスクサンプリングで内部に点を生成
	rxUniformPoissonDiskSampler sampler(minp, maxp, n, 10, min_dist);
	sampler.Generate(points);

	// ドロネー三角形分割で三角形を生成
	vector<vector<int>> tris;
	CreateDelaunayTriangles(points, tris);

	m_iNv = (int)points.size();
	m_iNt = (int)tris.size();

	m_vX = m_vP = points;
	m_vTC.resize(m_iNv);
	for (int i = 0; i < m_iNv; ++i)
	{
		m_vTC[i] = glm::vec2((points[i][0] + c1[0]) / (c2[0] - c1[0]), (points[i][1] + c1[1]) / (c2[1] - c1[1]));
	}
	m_vTri.resize(m_iNt * 3);
	for (int i = 0; i < m_iNt; ++i)
	{
		m_vTri[3 * i + 0] = tris[i][0];
		m_vTri[3 * i + 1] = tris[i][1];
		m_vTri[3 * i + 2] = tris[i][2];
	}
}

/*!
* メッシュ変形 by MLS
*  - Affine Deformation
* @param[in] v 変形する頂点座標
* @param[in] pc,qc 初期形状,変形後の頂点vの重み付き中心(授業スライドでのp*とq*)
* @param[in] alpha 重みwを求める時の係数α
* @return 変形後の座標(f(v))
*/
glm::vec2 rxMeshDeform2D::affineDeformation(const glm::vec2 &v, const glm::vec2 &pc, const glm::vec2 &qc, const double alpha)
{
	// TODO:この部分で 入力頂点座標v と 制御点座標p,q から入力頂点の変形後の座標を計算するコードを書く
	// - 横ベクトル×行列の計算部分はglmのオペレータ*は使わない方がよい(glmは縦ベクトルを前提としている)
	// - Ajを計算してから変形後の座標を計算するのでも，スライドp43の式を直接計算するのでもどちらでもOK
	//   (Ajの前計算は今回は行わないでもよい)
	//
	// - 制御点でループして相対座標を計算するまでのコード例:

	// Ajの逆行列部分の計算
	glm::mat2 WP = glm::mat2(0.f);
	for (size_t k = 0; k < m_iNfix; k++) // 制御点数(m_iNfix)でループを回す
	{
		int i = m_vFix[k]; // 制御点の頂点インデックス

		// 重み
		const glm::vec2 &p = m_vP[i];
		// 固定点と計算点の間の距離に基づく重み
		double dist = glm::length2(p - v);
		float w = (dist > 1.0e-6) ? 1.0f / pow(dist, alpha) : 0.0f;
		// 重心を中心とした制御点の相対座標
		const glm::vec2 p_hat = p - pc;

		WP[0][0] += p_hat.x * w * p_hat.x;
		WP[0][1] += p_hat.y * w * p_hat.x;
		WP[1][0] += p_hat.x * w * p_hat.y;
		WP[1][1] += p_hat.y * w * p_hat.y;
	}

	// 逆行列を計算
	glm::mat2 invWP = glm::mat2(0.f);
	invWP = glm::inverse(WP);
	// float m = 1.f / glm::determinant(WP);
	// float m = 1.f / (WP[0][0] * WP[1][1] - WP[1][0] * WP[0][1]);

	// invWP[0][0] = m * WP[1][1];
	// invWP[0][1] = m * -WP[0][1];
	// invWP[1][0] = m * -WP[1][0];
	// invWP[1][1] = m * WP[0][0];

	// Aj*qj部分の計算
	glm::vec2 fa = glm::vec2(0.f);
	for (int k = 0; k < m_iNfix; ++k)
	{										 // 制御点数(m_iNfix)でループを回す
		int j = m_vFix[k]; // 制御点の頂点インデックス

		// 重心を中心とした制御点の相対座標
		// 各頂点の座標は配列m_vPとm_vXに格納されている(それぞれ初期形状と変形後の形状)
		const glm::vec2 p_hat = m_vP[j] - pc; // 初期形状での座標
		const glm::vec2 q_hat = m_vX[j] - qc; // 変形後の座標

		// 固定点と計算点の間の距離に基づく重み
		const glm::vec2 p = m_vP[j];
		double dist = glm::length2(p - v);
		float w = (dist > 1.0e-6) ? 1.0f / pow(dist, alpha) : 0.0f;

		// Ajを計算する
		// float w = 1.f / pow((m_vP[j] - v).length(), 2.f * alpha);
		glm::vec2 B = v - pc;
		float A = w * ((B.x * invWP[0].x + B.y * invWP[0].y) * p_hat.x + (B.x * invWP[1].x + B.y * invWP[1].y) * p_hat.y);

		fa += A * q_hat;
	}

	// 変形後の頂点vの座標
	fa += qc;

	return fa;
}

/*!
* メッシュ変形 by MLS
*  - Similarity Deformation
* @param[in] v 変形する頂点座標
* @return 変形後の座標(f(v))
*/
glm::vec2 rxMeshDeform2D::similarityDeformation(const glm::vec2 &v, const glm::vec2 &pc, const glm::vec2 &qc, const double alpha)
{
	// TODO:この部分で 入力頂点座標v と 制御点座標p,q から入力頂点の変形後の座標を計算するコードを書く
	// - 横ベクトル×行列の計算部分はglmのオペレータ*は使わない方がよい(glmは縦ベクトルを前提としている)
	// - μsを先に計算してから変形後の頂点位置を計算
	// - 行列A_iの前計算は今回は行わないでもよい

	// 変形後の頂点vの座標
	glm::vec2 fsv = v; // ここも書き換えること

	return fsv;
}

/*!
* メッシュ変形 by MLS
*  - Rigid Deformation
* @param[in] v 変形する頂点座標
* @return 変形後の座標(f(v))
*/
glm::vec2 rxMeshDeform2D::rigidDeformation(const glm::vec2 &v, const glm::vec2 &pc, const glm::vec2 &qc, const double alpha)
{
	// TODO:この部分で 入力頂点座標v と 制御点座標p,q から入力頂点の変形後の座標を計算するコードを書く
	// - 横ベクトル×行列の計算部分はglmのオペレータ*は使わない方がよい(glmは縦ベクトルを前提としている)
	// - μfを先に計算してから変形後の頂点位置を計算
	// - 行列A_iの前計算は今回は行わないでもよい

	// 変形後の頂点vの座標
	glm::vec2 frv = v; // ここも書き換えること

	return frv;
}

/*!
* メッシュ更新
* @param[in] dt 時間ステップ幅(このメッシュ変形法では使わない)
*/
int rxMeshDeform2D::Update(double dt)
{
	if (m_iNfix <= 1)
		return 0;

	// 各頂点を変形
	for (int i = 0; i < m_iNv; ++i)
	{
		if (std::find(m_vFix.begin(), m_vFix.end(), i) != m_vFix.end())
			continue;
		const glm::vec2 &v = m_vP[i];

		// 固定点の移動前，移動後の重み付き中心p*,q*の計算
		glm::vec2 pc(0.0), qc(0.0);
		double wsum = 0.0;
		for (int k = 0; k < m_iNfix; ++k)
		{
			int j = m_vFix[k];
			const glm::vec2 &p = m_vP[j];
			const glm::vec2 &q = m_vX[j];

			// 固定点と計算点の間の距離に基づく重み
			double dist = glm::length2(p - v);
			float w = (dist > 1.0e-6) ? 1.0f / pow(dist, m_alpha) : 0.0f;

			pc += w * p;
			qc += w * q;
			wsum += w;
		}
		pc /= wsum;
		qc /= wsum;

		// MLS Deformations
		switch (m_deform_type)
		{
		case 0:
			m_vX[i] = affineDeformation(v, pc, qc, m_alpha);
			break;
		case 1:
			m_vX[i] = similarityDeformation(v, pc, qc, m_alpha);
			break;
		case 2:
			m_vX[i] = rigidDeformation(v, pc, qc, m_alpha);
			break;
		}
	}

	return 1;
}
