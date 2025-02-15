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
