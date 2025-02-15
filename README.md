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

## 大きく改善

- 初期解に対してもスコアを適用して貪欲
- not_connect_w を修正（これがカスすぎ！！） <- 改善のメインの理由

