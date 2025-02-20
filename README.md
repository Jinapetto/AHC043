# AHC043

## 02/14
問題文を読んだ

線路を引く -> 時間を使う

駅を設置する -> お金を使う

序盤、お金が少ないときは線路をひき、5000円たまるごとに駅を作る。
終盤は駅をたくさん生やす

駅の設置場所を固定すると駅を置く順番が大事になり、これは焼けそう

駅を置く順番を決めると最適解がでる？

経路を決めるのが難しそうだけどなんとかなりそう

駅の設置場所も焼けそう

スコア関数
- 駅の数（少ない方がよい）
- 全域木の総和？（全域木にする必要はなくて、それもむずい）<- そんなことなくて木で最適が出せそう
  
すべての家と職場を被覆するようなマスの集合を決める
- フローで解けそう

訪問する順番を焼く

貪欲に線路を作っていく

## 陣地を広げていくゲームだと思う
- 点のpairを両方、陣地に入れると得点

## 02/15

## 貪欲を提出
https://atcoder.jp/contests/ahc043/submissions/62741309

- 一手目は一番遠くできるペアを選ぶ
- 今ある駅からつなげることのできるcostが一番小さいペアを接続する
- 500ターンで操作をやめる

貪欲を改善
https://atcoder.jp/contests/ahc043/submissions/62742163
> 今ある駅からつなげることのできるcostが一番小さいペアを接続する

を増える収入/コストにした

さらに改善
https://atcoder.jp/contests/ahc043/submissions/62744662

700ターンで操作をやめるようにした。

## 大きく改善
https://atcoder.jp/contests/ahc043/submissions/62745494

- 初期解に対してもスコアを適用して貪欲
- not_connect_w を修正（これがカスすぎ！！） <- 改善のメインの理由

Average Score : 2,048,252.83

Average Score (log10) : 5.75598

改善

https://atcoder.jp/contests/ahc043/submissions/62747334

- かかる操作の回数を評価

Average Score          : 2,634,072.44

Average Score (log10)  : 5.89788

改善
- お金がないときもとりあえず線路を引く

https://atcoder.jp/contests/ahc043/submissions/62750386

Average Score          : 2,769,818.47

Average Score (log10)  : 5.94957

改善
- 線路にも駅を生やす
- not_connect の修正
- 2手目以降も13近傍で探索
- 一手で二つ駅を増やすやつを削除 <- 実装上の問題なので、復活させる TODO

https://atcoder.jp/contests/ahc043/submissions/62751940

Average Score          : 3,510,437.30

Average Score (log10)  : 6.07335

改善
- 途中で操作をやめた時の最終スコアを評価
- 手の探索を全ますで行う

https://atcoder.jp/contests/ahc043/submissions/62823586

Average Score          : 3,586,911.53

Average Score (log10)  : 6.09890

## ある程度マシな解ができての考察
- やはり最後は駅の設置が追い付いていない <- 線路の設置が時間がかかりすぎる
- 最初お金がないときに線路を引くべき
  - 線路の配置を事前に決める必要がある。
- スコアの総和があてにならなすぎるので、logを記録する
- 前半、お金がないときは今までの感じでいいけど、後半はお金があるので、長期的にみるべき

## これからの展望

貪欲の改善点
- >前半、お金がないときは今までの感じでいいけど、後半はお金があるので、長期的にみるべき
- 一手目と二手目を同時に考える
- 新しく建設するのをやめる時期をもう少し真面目に考える

貪欲でここまで来てしまったが、そろそろ勝ちに行く方針を思いつきたい
- ビームサーチに乗るぐらい貪欲を軽くする
- 焼きなましを考える

## 今日最後の案
- 焼きなましやビームサーチなどで訪問場所と訪問順を決める
- ある2点間に辺を追加するときは、通過経路となりえる、訪問順が昇順の点を見ていって、貪欲にその点を通るようにがんばる
- とりあえず上のじっそうをしたいので訪問順は今のままがんばる

## 02/16
駅の配置と順番を指定すると、貪欲に線路を貼ってくれるアルゴリズムを書いた
https://atcoder.jp/contests/ahc043/submissions/62834122

Average Score          : 3,549,055.52

Average Score (log10)  : 6.09754

スコアが変わってないのはいいこと

このことから、駅の配置と順番が一番大事であることがいえる？

駅の配置と順番を今の貪欲をもとに作った。今までよりも自由度が少し高いためスコアがよい

https://atcoder.jp/contests/ahc043/submissions/62838587

Average Score          : 3,542,441.64

Average Score (log10)  : 6.12055

## ビームサーチをした

- スコア関数は (今のお金) + (今の収入)*(残っているターン数) + 1000*(通勤は歩きだが、片方は駅になっている頂点数)
- > (通勤は歩きだが、片方は駅になっている頂点数)
  
の部分を入れないと、まともなスコアがでない
- バグが多くてスコアが出てなかったので、気を付けましょう
- 提出したら絶対スコアが下がり、相対スコアが上がった (たくさん稼げるケースに弱い)

https://atcoder.jp/contests/ahc043/submissions/62851454

Average Score          : 3,143,946.17

Average Score (log10)  : 6.18975

改善

今までは通勤は歩きだが、片方は駅になっている頂点数を評価していたが単純に、接続されている家またはオフィスにした

https://atcoder.jp/contests/ahc043/submissions/62852433 (TLE)

Average Score          : 3,247,435.38

Average Score (log10)  : 6.19970

## 修正するべき点
- 変なビームサーチをしているために、終了条件が難しい。とりあえず上位k個がすべて終わりを迎えたらビームサーチを終了するのを実装する。
- スコア関数が粗削りすぎ！
- first_stepに700msぐらい使っている気がしてもったいない

## 焼ける気がする
- 収入が2000を超えたあたりからお金の管理が楽になるので焼けそう

## 02/17
ビーム幅、hashの導入、ビームサーチの終了条件などをいじった。

hash で結構上がった

https://atcoder.jp/contests/ahc043/submissions/62861278

Average Score          : 3,368,665.39

Average Score (log10)  : 6.22230

絶対スコアが貪欲を越せていない問題
手を評価するのが難しいのが原因

あと、ビームサーチで最終的な解を複数個持ってくる

最初の一手もビームサーチで決める

https://atcoder.jp/contests/ahc043/submissions/62865735

Average Score          : 3,409,277.25

Average Score (log10)  : 6.24180

ビームサーチを多終点にした

https://atcoder.jp/contests/ahc043/submissions/62866594

Average Score          : 3,414,535.82

Average Score (log10)  : 6.24303

## 焼きなましを追加
お金に余裕があるとき以降で焼きなまし、遷移は入れ替えるだけ　ビームサーチの多終点をやめた <- 実装が面倒だっただけ復活させましょう

Average Score          : 3,686,252.08

Average Score (log10)  : 6.25520

ビームサーチ、焼きなましともに高速化で伸びそう

ビームサーチは遷移を見直すなど

焼きなましは遷移を増やす、差分更新を導入する

## 02/18
焼きなましの遷移をいろいろやってみた
- 挿入
- 削除
- 1マスずらす

が、全部だめ　謎

焼きなましの開始収入をいじったら上がった。

https://atcoder.jp/contests/ahc043/submissions/62883558

Average Score          : 3,820,908.26

Average Score (log10)  : 6.26950

ビーム幅をちゃんと考えた

![image](https://github.com/user-attachments/assets/d51ac190-4c5c-4ffd-981e-c990150cee22)


ビームサーチの多終点を再開したが、ほとんど焼きなますため意味なし

https://atcoder.jp/contests/ahc043/submissions/62885763

Average Score          : 3,811,381.71

Average Score (log10)  : 6.27080

つながっている頂点の評価をターン数が少ないほど評価した

https://atcoder.jp/contests/ahc043/submissions/62886935

Average Score          : 3,819,295.94

Average Score (log10)  : 6.27426

線路のつなぐ向き？をランダムにした

https://atcoder.jp/contests/ahc043/submissions/62888002

Average Score          : 3,860,428.91

Average Score (log10)  : 6.27737

## 大幅な改善
焼きなましが効かなかったのは、ターンが超過していた時に、積極的に消してしまっていたためだった。

ターンが超過したときに、そこをマイナスにするのではなく、評価しないことにしたらスコアがかなり伸びた

これでもかなり適当で、ターン超過したときは挿入、削除の遷移をしないなどをしたほうがよさそう

https://atcoder.jp/contests/ahc043/submissions/62889541

Average Score          : 4,165,275.83

Average Score (log10)  : 6.29024

shift, del をターン超過しない範囲に適用、insertをターン超過したときにはしない

https://atcoder.jp/contests/ahc043/submissions/62893174

Average Score          : 4,172,336.26

Average Score (log10)  : 6.29288

shift の遷移を13近傍にした
https://atcoder.jp/contests/ahc043/submissions/62894056

Average Score          : 4,193,622.90

Average Score (log10)  : 6.29664

これ最初の1近傍いらないですね

修正。たぶん上振れ

Average Score          : 4,218,719.54

Average Score (log10)  : 6.29745

焼きなましのスタート地点を変更
https://atcoder.jp/contests/ahc043/submissions/62894971

Average Score          : 4,227,675.60

Average Score (log10)  : 6.29917

焼きなましの差分更新実装案
- station_pos にidxを振る
- それぞれのstation_posに対してターンを求める
- 各家、オフィスについてカバーされるようなstation_posのidxをsetで管理する。setはstation_posのターンでソートしておく
- また各station_pos が最初にカバーするような総和を持つ
- 各遷移において、今まで通りO(|station_pos|) でターンを求める

明日の自分がんばってくれー

## 02/19
差分更新を実装したが効果なし、つらい

焼きなましの距離の計算をマンハッタンの45度回転で解く

頭が悪くてできませんでした。

> 平面座標の点が順番に与えられるので、すべての点について、その点よりも前に追加された点とのマンハッタン距離の最小値を求めてください

これってo(N^2)で解けますか？

多点スタート焼きなましにしているが、一点にしても 0.2%ぐらいスコアが落ちた。なぜ？

焼きなましの評価関数ないで既存の線路の上に駅を設置するとき、距離がINFとなっているバグを修正


Average Score          : 4,403,464.82

Average Score (log10)  : 6.31103

すべてのvectorをarrayにした。これは嘘で一部は置き換えれてない

焼きなましのスタート温度を初期解のスコアから決めるように

https://atcoder.jp/contests/ahc043/submissions/62923463

Average Score          : 4,421,154.68

Average Score (log10)  : 6.31158

insertの確率を上げた

https://atcoder.jp/contests/ahc043/submissions/62925592


Average Score          : 4,413,540.67

Average Score (log10)  : 6.31111

線路を引くときに、たくさんの家やオフィスを通るようにDP

https://atcoder.jp/contests/ahc043/submissions/62927411

Average Score          : 4,431,319.46

Average Score (log10)  : 6.31269

## 02/20

array<int,4> shift_rate = {3,5,2,5};

https://atcoder.jp/contests/ahc043/submissions/62942337

Average Score          : 4,444,719.19

Average Score (log10)  : 6.31302

optuna

https://atcoder.jp/contests/ahc043/submissions/62952314

Average Score          : 4,496,231.71

Average Score (log10)  : 6.31822