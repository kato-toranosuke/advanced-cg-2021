# Advanced CG 2021
2021年度のアドバンストCGのレポート及びソースコードを格納。

## release btanch
### v1.0
開発環境を作った。ただし、day01の問題05の初期画面において画像が表示されない問題がある。

### v2.0
day01の課題を完成させた。<br>
問題05の初期画面の問題はmac特有の問題だったらしく、修正プログラムが配布されたので、それを統合した。

### v3.0
day02の課題を完成させた。<br>
レポートはlatexで作成。

### v4.0
day03の課題。Lambert BRDF, Blinn-Phong BRDFの重点サンプリングを用いたパストレーシング。

### v5.0
- day04<br>
  細分割曲面の実装を行った。なお、方法は以下の２つ。
  - Loop Subdivision
  - Catmull-Clark Subdivision
- day05<br>
  MLS変形の実装。以下の３種類の変形を行った。
  - Affine Deformations
  - Similarity Deformations
  - Rigid Deformations

### v6.0
day06の課題。弾性変形シミュレーションについて学び、基本的な位置ベース手法（PBD）を実装した。<br>
以下の制約条件についての位置修正量を計算した。
- Stretching Constraint
- Bending Constraint
- Volume Constraint

### v7.0
day07の課題。キャラクターアニメーションにおけるスキニングについて学び、スキニング手法であるLBS及びDQSを実装する。