#include <bits/stdc++.h>
using namespace std;
using ll = long long;
using ull = unsigned long long;

#define rep(i, n) for (int i = 0; i < (int)(n); i++)
#define ALL(obj) (obj).begin(),(obj).end()

int n,m,init_money,t;
vector<pair<int,int>> house;
vector<pair<int,int>> office;
const int station_cost = 5000;
const int rail_cost = 100;
// そのマスにある家のidx
array<array<vector<int>,50>,50> house_on_grid;
// そのマスにあるオフィスのidx
array<array<vector<int>,50>,50> office_on_grid;

// そのマスにある家のidx (13近傍)
array<array<vector<int>,50>,50> house_on_grid13;
// そのマスにあるオフィスのidx(13近傍)
array<array<vector<int>,50>,50> office_on_grid13;

// その人の通勤距離
array<int,1600> commute_dist;

// 13近傍
array<int,13> dx13 = {0,1,0,-1,0,1,-1,-1,1,2,0,-2,0};
array<int,13> dy13 = {0,0,1,0,-1,1,1,-1,-1,0,2,0,-2};

// 4近傍
array<int,4> dx4 = {1,0,-1,0};
array<int,4> dy4 = {0,1,0,-1};

// 25近傍
vector<int> dx25 = {0,1,0,-1,0,1,-1,-1,1,2,0,-2,0,1,2,2,1,-1,-2,-2,-1,3,0,-3,0};
vector<int> dy25 = {0,0,1,0,-1,1,1,-1,-1,0,2,0,-2,-2,-1,1,2,2,1,-1,-2,0,3,0,-3};

// Zobrist_hash
array<array<int,50>,50> z_hash_grid;
array<int,1600> z_hash_house;
array<int,1600> z_hash_office;

// parameter
const int yaki_start_income = 1000;
const int yaki_start_money = 6000; // 両方満たしたときに焼きなます

const array<int,4> shift_rate = {1,2,1,1};

//parameter inputで計算
int connect_cnt_w;

// ビーム幅
int beam_width = 0;

unsigned long xor128(void){
	static unsigned long x = 123456789,y = 362436069,z = 541288629,w = 88675123;
	unsigned long t;
	t = (x^(x<<11));
	x = y;y = z;z = w;
	return(w = (w^(w>>19))^(t^(t>>8)));
}

mt19937 gen(random());

namespace beam_search {

template<class S, S (*op)(S, S), S (*e)()> struct segtree {
    segtree() : segtree(0) {}
    segtree(int n) : segtree(vector<S>(n, e())) {}
    segtree(const vector<S>& v) : n(v.size()) {
        s = bit_ceil(unsigned(n));
        log = countr_zero(unsigned(s));
        d = vector<S>(2 * s, e());
        for(int i = 0; i < n; i++) d[s + i] = v[i];
        for(int i = s - 1; i >= 1; i--) update(i);
    }

    void set(int p, S x) {
        p += s;
        d[p] = x;
        for(int i = 1; i <= log; i++) update(p >> i);
    }

    S get(int p) { return d[p + s]; }

    S prod(int l, int r) const {
        S sml = e(), smr = e();
        l += s, r += s;
        while(l < r) {
            if(l & 1) sml = op(sml, d[l++]);
            if(r & 1) smr = op(d[--r], smr);
            l >>= 1, r >>= 1;
        }
        return op(sml, smr);
    }

    S all_prod() const { return d[1]; }

    template<typename F> int max_right(int l, F f) const {
        if(l == n) return n;
        l += s;
        S sm = e();
        do {
            while(~l & 1) l >>= 1;
            if(!f(op(sm, d[l]))) {
                while(l < s) {
                    l <<= 1;
                    if(f(op(sm, d[l]))) sm = op(sm, d[l++]);
                }
                return l - s;
            }
            sm = op(sm, d[l++]);
        } while((l & -l) != l);
        return n;
    }

    template<typename F> int min_left(int r, F f) const {
        if(!r) return 0;
        r += s;
        S sm = e();
        do {
            r--;
            while(r > 1 and r & 1) r >>= 1;
            if(!f(op(d[r], sm))) {
                while(r < s) {
                    r = (2 * r + 1);
                    if(f(op(d[r], sm))) sm = op(d[r--], sm);
                }
                return r + 1 - s;
            }
            sm = op(d[r], sm);
        } while((r & -r) != r);
        return 0;
    }

private:
    int n, s, log;
    vector<S> d;
    void update(int k) { d[k] = op(d[k * 2], d[k * 2 + 1]); }
};

// メモリの再利用を行いつつ集合を管理するクラス
template<class T>
class ObjectPool {
    public:
        // 配列と同じようにアクセスできる
        T& operator[](int i) {
            return data_[i];
        }

        // 配列の長さを変更せずにメモリを確保する
        void reserve(size_t capacity) {
            data_.reserve(capacity);
        }

        // 要素を追加し、追加されたインデックスを返す
        int push(const T& x) {
            if (garbage_.empty()) {
                data_.push_back(x);
                return data_.size() - 1;
            } else {
                int i = garbage_.top();
                garbage_.pop();
                data_[i] = x;
                return i;
            }
        }

        // 要素を（見かけ上）削除する
        void pop(int i) {
            garbage_.push(i);
        }

        // 使用した最大のインデックス(+1)を得る
        // この値より少し大きい値をreserveすることでメモリの再割り当てがなくなる
        size_t size() {
            return data_.size();
        }

    private:
        vector<T> data_;
        stack<int> garbage_;
};

// 連想配列
// Keyにハッシュ関数を適用しない
// open addressing with linear probing
// unordered_mapよりも速い
// nは格納する要素数よりも4~16倍ほど大きくする
template <class Key, class T>
struct HashMap {
    public:
        explicit HashMap(uint32_t n) {
            n_ = n;
            valid_.resize(n_, false);
            data_.resize(n_);
        }

        // 戻り値
        // - 存在するならtrue、存在しないならfalse
        // - index
        pair<bool,int> get_index(Key key) const {
            Key i = key % n_;
            while (valid_[i]) {
                if (data_[i].first == key) {
                    return {true, i};
                }
                if (++i == n_) {
                    i = 0;
                }
            }
            return {false, i};
        }

        // 指定したindexにkeyとvalueを格納する
        void set(int i, Key key, T value) {
            valid_[i] = true;
            data_[i] = {key, value};
        }

        // 指定したindexのvalueを返す
        T get(int i) const {
            assert(valid_[i]);
            return data_[i].second;
        }

        void clear() {
            fill(valid_.begin(), valid_.end(), false);
        }

    private:
        uint32_t n_;
        vector<bool> valid_;
        vector<pair<Key,T>> data_;
};

using Hash = uint32_t; // TODO

// 状態遷移を行うために必要な情報
// メモリ使用量をできるだけ小さくしてください
struct Action {
    // 全部差分
    // [0,6) -> x, [6,12) -> y, [12,24) -> turn, [24,50) -> income, [50,56) -> tx, [56,62) -> ty, [62,63) -> fin, [63,64) -> is first
    unsigned long long x_y_turn_income_fin;
    int money;
    Action() {
        x_y_turn_income_fin = 0;
        money = 0;
    }
};

using Cost = int; // TODO

// 状態のコストを評価するための構造体
// メモリ使用量をできるだけ小さくしてください
struct Evaluator {

    Evaluator() {
        score = 0;
    }

    int score;
    // 低いほどよい
    Cost evaluate() const {
        return - score;
    }
};

// 展開するノードの候補を表す構造体
struct Candidate {
    Action action;
    Evaluator evaluator;
    Hash hash;
    int parent;
    Cost cost;

    Candidate(Action action, Evaluator evaluator, Hash hash, int parent, Cost cost) :
        action(action),
        evaluator(evaluator),
        hash(hash),
        parent(parent),
        cost(cost) {}
};

// ビームサーチの設定
struct Config {
    int max_turn;
    size_t beam_width;
    size_t nodes_capacity;
    uint32_t hash_map_capacity;
};

// 削除可能な優先度付きキュー
using MaxSegtree = segtree<
    pair<Cost,int>,
    [](pair<Cost,int> a, pair<Cost,int> b){
        if (a.first >= b.first) {
            return a;
        } else {
            return b;
        }
    },
    []() { return make_pair(numeric_limits<Cost>::min(), -1); }
>;

// ノードの候補から実際に追加するものを選ぶクラス
// ビーム幅の個数だけ、評価がよいものを選ぶ
// ハッシュ値が一致したものについては、評価がよいほうのみを残す
class Selector {
    public:
        explicit Selector(const Config& config) :
            hash_to_index_(config.hash_map_capacity)
        {
            beam_width = config.beam_width;
            candidates_.reserve(beam_width);
            full_ = false;
            st_original_.resize(beam_width);
        }

        // 候補を追加する
        // ターン数最小化型の問題で、candidateによって実行可能解が得られる場合にのみ finished = true とする
        // ビーム幅分の候補をCandidateを追加したときにsegment treeを構築する
        void push(Action action, const Evaluator& evaluator, Hash hash, int parent, bool finished) {
            Cost cost = evaluator.evaluate();
            if (finished) {
                finished_candidates_.emplace_back(Candidate(action, evaluator, hash, parent, cost));
                return;
            }
            if (full_ && cost >= st_.all_prod().first) {
                // 保持しているどの候補よりもコストが小さくないとき
                return;
            }
            auto [valid, i] = hash_to_index_.get_index(hash);

            if (valid) {
                int j = hash_to_index_.get(i);
                if (hash == candidates_[j].hash) {
                    // ハッシュ値が等しいものが存在しているとき
                    if (cost < candidates_[j].cost) {
                        // 更新する場合
                        candidates_[j] = Candidate(action, evaluator, hash, parent, cost);
                        if (full_) {
                            st_.set(j, {cost, j});
                        }
                    }
                    return;
                }
            }
            if (full_) {
                // segment treeが構築されている場合
                int j = st_.all_prod().second;
                hash_to_index_.set(i, hash, j);
                candidates_[j] = Candidate(action, evaluator, hash, parent, cost);
                st_.set(j, {cost, j});
            } else {
                // segment treeが構築されていない場合
                hash_to_index_.set(i, hash, candidates_.size());
                candidates_.emplace_back(Candidate(action, evaluator, hash, parent, cost));

                if (candidates_.size() == beam_width) {
                    // 保持している候補がビーム幅分になったとき
                    construct_segment_tree();
                }
            }
        }

        // 選んだ候補を返す
        const vector<Candidate>& select() const {
            return candidates_;
        }

        // 実行可能解が見つかったか
        bool have_finished() const {
            return !finished_candidates_.empty();
        }

        // 実行可能解に到達する「候補」を返す
        vector<Candidate> get_finished_candidates() const {
            return finished_candidates_;
        }

        void clear() {
            candidates_.clear();
            hash_to_index_.clear();
            full_ = false;
        }

    private:
        size_t beam_width;
        vector<Candidate> candidates_;
        HashMap<Hash,int> hash_to_index_;
        bool full_;
        vector<pair<Cost,int>> st_original_;
        MaxSegtree st_;
        vector<Candidate> finished_candidates_;

        void construct_segment_tree() {
            full_ = true;
            for (size_t i = 0; i < beam_width; ++i) {
                st_original_[i] = {candidates_[i].cost, i};
            }
            st_ = MaxSegtree(st_original_);
        }
};


//駅からの距離
array<array<int,50>,50> calc_dist(vector<pair<int,int>>& station_pos){
    array<array<int,50>,50> dist;
    rep(i,n)rep(j,n) dist[i][j] = (int)1e9;
    queue<pair<int,int>> que;
    for(auto [x,y] : station_pos){
        que.push({x,y});
        dist[x][y] = 0;
    }
    while(!que.empty()){
        auto [x,y] = que.front();
        que.pop();
        rep(i,4){
            int nx = x + dx4[i];
            int ny = y + dy4[i];
            if(0 <= nx && nx < n && 0 <= ny && ny < n && dist[nx][ny] == 1e9){
                dist[nx][ny] = dist[x][y] + 1;
                que.push({nx,ny});
            }
        }
    }
    return dist;
}

// {増える収入、connect_cnt} あとhashを参照でいじる
pair<int,int> calc_inc_income(int x, int y, array<int,1600>& vis_house, array<int,1600>& vis_office, int connect_cnt, Hash& hash){
    int ret = 0;
    for(int idx : house_on_grid13[x][y]){
        if(vis_house[idx] == 0){
            hash ^= z_hash_house[idx];
            connect_cnt++;
            if(vis_office[idx] != 0) ret += commute_dist[idx];
        }
    }
    for(int idx : office_on_grid13[x][y]){
        if(vis_office[idx] == 0){
            hash ^= z_hash_office[idx];
            connect_cnt++;
            if(vis_house[idx] != 0) ret += commute_dist[idx];
        }
    }
    return {ret,connect_cnt};
}

// {inc_income, cnnect_cnt, hash}
tuple<int,int,Hash> calc_first_step(int sx,int sy,int tx,int ty){
    vector<bool> vis_house(m,false);
    vector<bool> vis_office(m,false);

    for(int idx : house_on_grid13[sx][sy]) vis_house[idx] = true;
    for(int idx : house_on_grid13[tx][ty]) vis_house[idx] = true;

    for(int idx : office_on_grid13[sx][sy]) vis_office[idx] = true;
    for(int idx : office_on_grid13[tx][ty]) vis_office[idx] = true;

    int inc_income = 0;
    int connect_cnt = 0;
    Hash hash = 0;

    rep(i,m){
        if(vis_house[i] && vis_office[i]) inc_income += commute_dist[i];
        if(vis_house[i]){
            connect_cnt++;
            hash ^= z_hash_house[i];
        }
        if(vis_office[i]){
            connect_cnt++;
            hash ^= z_hash_office[i];
        }
    }

    return {inc_income,connect_cnt,hash};
}

// 最初の1手 {Action, Hash, connect_cnt}
vector<tuple<Action,Hash,int>> first_step(){
    // すべての人の家とオフィスをつなぐ候補を列挙する
    vector<pair<pair<int,int>,pair<int,int>> > cand;
    rep(i,m){
        int sx = house[i].first,sy = house[i].second;
        int tx = office[i].first,ty = office[i].second;
        rep(j,13){
            int nsx = sx + dx13[j], nsy = sy + dy13[j];
            if(0 <= nsx && nsx < n && 0 <= nsy && nsy < n){
                rep(k,13){
                    int ntx = tx + dx13[k], nty = ty + dy13[k];
                    if(0 <= ntx && ntx < n && 0 <= nty && nty < n) cand.push_back({{nsx,nsy},{ntx,nty}});
                }
            }
        }
    }
    // 重複を取り除く
    rep(i,cand.size()) if(cand[i].first > cand[i].second) swap(cand[i].first, cand[i].second);
    sort(ALL(cand));
    cand.erase(unique(ALL(cand)),cand.end());

    vector<tuple<Action,Hash,int>> ret;
    rep(i,cand.size()){
        int dist = abs(cand[i].first.first - cand[i].second.first) + abs(cand[i].first.second - cand[i].second.second);
        int cost = (dist - 1)*rail_cost + 2*station_cost;
        // お金が足りないとき
        if(cost > init_money) continue;

        int inc_turn = dist + 1;
        auto [inc_income,connect_cnt,hash] = calc_first_step(cand[i].first.first, cand[i].first.second, cand[i].second.first, cand[i].second.second);
        int inc_money = -cost + inc_income;
        Action act;
        act.x_y_turn_income_fin = (cand[i].first.first) | (ull(cand[i].first.second)<<6) | (ull(inc_turn)<<12) | (ull(inc_income)<<24) | (ull(cand[i].second.first)<<50) | (ull(cand[i].second.second)<<56) | (1ULL<<63);
        act.money = inc_money;
        ret.push_back({act,hash,connect_cnt});
    }

    return ret;
}

// 深さ優先探索に沿って更新する情報をまとめたクラス
class State {
    public:
        explicit State(/* const Input& input */) {
            connect_cnt = 0;
            cur_income = 0;
            cur_money = init_money;
            cur_turn = 0;
            fin = 0;
            rep(i,m) vis_house[i] = 0;
            rep(i,m) vis_office[i] = 0;
        }

        // 次の状態候補を全てselectorに追加する
        // 引数
        //   evaluator : 今の評価器
        //   hash      : 今のハッシュ値
        //   parent    : 今のノードID（次のノードにとって親となる）
        void expand(const Evaluator& evaluator, Hash hash, int parent, Selector& selector) {
            if(station_pos.size() == 0){// 1手目
                vector<tuple<Action,Hash,int>> cand = first_step();
                for(auto [act,hash,connect_cnt] : cand){
                    Evaluator n_eva;
                    n_eva.score = (cur_money + act.money) + (cur_income + connect_cnt*connect_cnt_w + ((act.x_y_turn_income_fin>>24) & ((1ULL<<26) - 1)))*(t - (cur_turn + ((act.x_y_turn_income_fin>>12) & ((1ULL<<12) - 1))));
                    selector.push(act,n_eva,hash,parent,false);
                }
                return;
            }
            {// 今を維持
                Action n_act;
                if(fin == 0) n_act.x_y_turn_income_fin |= (1ULL<<62);
                selector.push(n_act,evaluator,hash,parent,false);
            }
            if(fin) return;
            array<array<int,50>,50> dist = calc_dist(station_pos);
            rep(x,n)rep(y,n){
                if(house_on_grid13[x][y].size() == 0 && office_on_grid13[x][y].size() == 0) continue; //追加しても意味がない
                if(dist[x][y] == 0) continue; //駅
                // hash
                Hash n_hash = hash;

                auto [inc_income,n_connect_cnt] = calc_inc_income(x,y,vis_house,vis_office,connect_cnt,n_hash);
                if(n_connect_cnt == connect_cnt) continue; //追加しても意味がない

                int cost = (dist[x][y] - 1)*rail_cost + station_cost;
                
                int inc_turn = max(dist[x][y], (cost - cur_money + cur_income - 1)/cur_income + 1); // max(操作回数、お金がたまるまで)
                int inc_money =  - cost + (inc_turn - 1)*cur_income + (cur_income + inc_income);
                assert(cur_money + inc_money >= 0);

                Action n_act;
                n_act.money = inc_money;
                n_act.x_y_turn_income_fin = ull(x) | (ull(y)<<6) | (ull(inc_turn)<<12) | (ull(inc_income)<<24);
                
                Evaluator n_eva;
                n_eva.score = (cur_money + inc_money) + (cur_income + inc_income + n_connect_cnt*connect_cnt_w)*(t - (cur_turn + inc_turn));
                
                selector.push(n_act,n_eva,n_hash,parent,false);
            }

        }

        // actionを実行して次の状態に遷移する
        void move_forward(Action action) {
            fin ^= (action.x_y_turn_income_fin>>62) & 1;
            if(fin) return;
            cur_money += action.money;
            cur_income += (action.x_y_turn_income_fin>>24) & ((1ULL<<26) - 1);
            cur_turn += (action.x_y_turn_income_fin>>12) & ((1ULL<<12) - 1);
            int x = (action.x_y_turn_income_fin) & ((1ULL<<6) - 1);
            int y = (action.x_y_turn_income_fin>>6) & ((1ULL<<6) - 1);
            station_pos.push_back({x, y});
            for(int idx : house_on_grid13[x][y]){
                vis_house[idx]++;
                if(vis_house[idx] == 1) connect_cnt++;
            }
            for(int idx : office_on_grid13[x][y]){
                vis_office[idx]++;
                if(vis_office[idx] == 1) connect_cnt++;
            }
            if(action.x_y_turn_income_fin>>63){// 1手目
                x = (action.x_y_turn_income_fin>>50) & ((1ULL<<6) - 1);
                y = (action.x_y_turn_income_fin>>56) & ((1ULL<<6) - 1);
                station_pos.push_back({x, y});
                for(int idx : house_on_grid13[x][y]){
                    vis_house[idx]++;
                    if(vis_house[idx] == 1) connect_cnt++;
                }
                for(int idx : office_on_grid13[x][y]){
                    vis_office[idx]++;
                    if(vis_office[idx] == 1) connect_cnt++;
                }
            }
        }

        // actionを実行する前の状態に遷移する
        // 今の状態は、親からactionを実行して遷移した状態である
        void move_backward(Action action) {
            fin ^= (action.x_y_turn_income_fin>>62) & 1;
            if(fin ^ ((action.x_y_turn_income_fin>>62) & 1)) return;
            cur_money -= action.money;
            cur_income -= (action.x_y_turn_income_fin>>24) & ((1ULL<<26) - 1);
            cur_turn -= (action.x_y_turn_income_fin>>12) & ((1ULL<<12) - 1);
            int x = (action.x_y_turn_income_fin) & ((1ULL<<6) - 1);
            int y = (action.x_y_turn_income_fin>>6) & ((1ULL<<6) - 1);
            station_pos.pop_back();
            for(int idx : house_on_grid13[x][y]){
                vis_house[idx]--;
                if(vis_house[idx] == 0) connect_cnt--;
            }
            for(int idx : office_on_grid13[x][y]){
                vis_office[idx]--;
                if(vis_office[idx] == 0) connect_cnt--;
            }
            if(action.x_y_turn_income_fin>>63){// 1手目
                x = (action.x_y_turn_income_fin>>50) & ((1ULL<<6) - 1);
                y = (action.x_y_turn_income_fin>>56) & ((1ULL<<6) - 1);
                station_pos.pop_back();
                for(int idx : house_on_grid13[x][y]){
                    vis_house[idx]--;
                    if(vis_house[idx] == 0) connect_cnt--;
                }
                for(int idx : office_on_grid13[x][y]){
                    vis_office[idx]--;
                    if(vis_office[idx] == 0) connect_cnt--;
                }
            }
        }

        int cur_money;
        int cur_income;
        int cur_turn;
        array<int,1600> vis_office;
        array<int,1600> vis_house;
        vector<pair<int,int>> station_pos;
        int fin = 0;
        int connect_cnt = 0;
    private:

};

// 探索木（二重連鎖木）のノード
struct Node {
    Action action;
    Evaluator evaluator;
    Hash hash;
    int parent, child, left, right;

    // 根のコンストラクタ
    Node(Action action, const Evaluator& evaluator, Hash hash) :
        action(action),
        evaluator(evaluator),
        hash(hash),
        parent(-1),
        child(-1),
        left(-1),
        right(-1) {}

    // 通常のコンストラクタ
    Node(const Candidate& candidate, int right) :
        action(candidate.action),
        evaluator(candidate.evaluator),
        hash(candidate.hash),
        parent(candidate.parent),
        child(-1),
        left(-1),
        right(right) {}
};

// 二重連鎖木に対する操作をまとめたクラス
class Tree {
    public:
        explicit Tree(const State& state, size_t nodes_capacity, const Node& root) :
            state_(state)
        {
            nodes_.reserve(nodes_capacity);
            root_ = nodes_.push(root);
        }

        // 状態を更新しながら深さ優先探索を行い、次のノードの候補を全てselectorに追加する
        int dfs(Selector& selector) {
            update_root();
            int fin_cnt = 0;
            int v = root_;
            while (true) {
                v = move_to_leaf(v);
                if(state_.fin) fin_cnt++;
                state_.expand(nodes_[v].evaluator, nodes_[v].hash, v, selector);
                v = move_to_ancestor(v);
                if (v == root_) {
                    break;
                }
                v = move_to_right(v);
            }
            return fin_cnt;
        }

        // 根からノードvまでのパスを取得する
        vector<Action> get_path(int v) {
            // cerr << nodes_.size() << endl;

            vector<Action> path;
            while (nodes_[v].parent != -1) {
                path.push_back(nodes_[v].action);
                v = nodes_[v].parent;
            }
            reverse(path.begin(), path.end());
            return path;
        }

        // 新しいノードを追加する
        int add_leaf(const Candidate& candidate) {
            int parent = candidate.parent;
            int sibling = nodes_[parent].child;
            int v = nodes_.push(Node(candidate, sibling));

            nodes_[parent].child = v;

            if (sibling != -1) {
                nodes_[sibling].left = v;
            }
            return v;
        }

        // ノードvに子がいなかった場合、vと不要な先祖を削除する
        void remove_if_leaf(int v) {
            if (nodes_[v].child == -1) {
                remove_leaf(v);
            }
        }

        // 最も評価がよいノードを返す
        int get_best_leaf(const vector<int>& last_nodes) {
            assert(!last_nodes.empty());
            int ret = last_nodes[0];
            for (int v : last_nodes) {
                if (nodes_[v].evaluator.evaluate() < nodes_[ret].evaluator.evaluate()) {
                    ret = v;
                }
            }
            return ret;
        }

        // 全部のノードを返す
        vector<int> get_all_leaf(const vector<int>& last_nodes) {
            assert(!last_nodes.empty());
            vector<int> ret;
            for (int v : last_nodes) {
                ret.push_back(v);
            }
            //ソートしておく
            sort(ret.begin(),ret.end(),[&](int i,int j){
                return nodes_[i].evaluator.evaluate() < nodes_[j].evaluator.evaluate();
            });
            return ret;
        }

    private:
        State state_;
        ObjectPool<Node> nodes_;
        int root_;

        // 根から一本道の部分は往復しないようにする
        void update_root() {
            int child = nodes_[root_].child;
            while (child != -1 && nodes_[child].right == -1) {
                root_ = child;
                state_.move_forward(nodes_[child].action);
                child = nodes_[child].child;
            }
        }

        // ノードvの子孫で、最も左にある葉に移動する
        int move_to_leaf(int v) {
            int child = nodes_[v].child;
            while (child != -1) {
                v = child;
                state_.move_forward(nodes_[child].action);
                child = nodes_[child].child;
            }
            return v;
        }

        // ノードvの先祖で、右への分岐があるところまで移動する
        int move_to_ancestor(int v) {
            while (v != root_ && nodes_[v].right == -1) {
                state_.move_backward(nodes_[v].action);
                v = nodes_[v].parent;
            }
            return v;
        }

        // ノードvの右のノードに移動する
        int move_to_right(int v) {
            state_.move_backward(nodes_[v].action);
            v = nodes_[v].right;
            state_.move_forward(nodes_[v].action);
            return v;
        }

        // 不要になった葉を再帰的に削除する
        void remove_leaf(int v) {
            while (true) {
                int left = nodes_[v].left;
                int right = nodes_[v].right;
                if (left == -1) {
                    int parent = nodes_[v].parent;

                    if (parent == -1) {
                        cerr << "ERROR: root is removed" << endl;
                        exit(-1);
                    }
                    nodes_.pop(v);
                    nodes_[parent].child = right;
                    if (right != -1) {
                        nodes_[right].left = -1;
                        return;
                    }
                    v = parent;
                } else {
                    nodes_.pop(v);
                    nodes_[left].right = right;
                    if (right != -1) {
                        nodes_[right].left = left;
                    }
                    return;
                }
            }
        }
};

// ビームサーチを行う関数
vector<vector<Action>> beam_search(const Config& config, State state, Node root) {
    Tree tree(state, config.nodes_capacity, root);

    // 探索中のノード集合
    vector<int> curr_nodes;
    curr_nodes.reserve(config.beam_width);
    // 本来は curr_nodes = {state.root_} とすべきだが, 省略しても問題ない

    // 新しいノードの集合
    vector<int> next_nodes;
    next_nodes.reserve(config.beam_width);

    // 新しいノード候補の集合
    Selector selector(config);

    for (int turn = 0; turn < config.max_turn; ++turn) {
        // Euler Tour で selector に候補を追加する
        int fin_cnt = tree.dfs(selector);

        // 終わっているノードがビーム幅と一致したら終了
        if(fin_cnt == config.beam_width){
            break;
        }

        // if (selector.have_finished()) {
        //     // ターン数最小化型の問題で実行可能解が見つかったとき
        //     Candidate candidate = selector.get_finished_candidates()[0];
        //     vector<Action> ret = tree.get_path(candidate.parent);
        //     ret.push_back(candidate.action);
        //     return ret;
        // }
        // 新しいノードを追加する
        for (const Candidate& candidate : selector.select()) {
            next_nodes.push_back(tree.add_leaf(candidate));
        }
        if (next_nodes.empty()) {
            // 新しいノードがないとき
            cerr << "ERROR: Failed to find any valid solution" << endl;
            return {};
        }
        // 不要なノードを再帰的に削除する
        for (int v : curr_nodes) {
            tree.remove_if_leaf(v);
        }
        // ダブルバッファリングで配列を使い回す
        swap(curr_nodes, next_nodes);
        next_nodes.clear();

        selector.clear();
    }
    // ターン数固定型の問題で全ターンが終了したとき
    // int best_leaf = tree.get_best_leaf(curr_nodes);
    // return tree.get_path(best_leaf);
    // 最後に残った状態を全部返す
    vector<int> all_leaf = tree.get_all_leaf(curr_nodes);
    vector<vector<Action>> ret;
    for(int idx : all_leaf) ret.push_back(tree.get_path(idx));
    return ret;
}

} // namespace beam_search

void input(){
    cin >> n >> m >> init_money >> t;
    house.assign(m,pair<int,int>());
    office.assign(m,pair<int,int>());
    rep(i,m){
        cin >> house[i].first >> house[i].second;
        cin >> office[i].first >> office[i].second;
        house_on_grid[house[i].first][house[i].second].push_back(i);
        office_on_grid[office[i].first][office[i].second].push_back(i);
        commute_dist[i] = abs(office[i].first - house[i].first) + abs(office[i].second - house[i].second);
        rep(j,13){
            int nx = house[i].first + dx13[j];
            int ny = house[i].second + dy13[j];
            if(0 <= nx && nx < n && 0 <= ny && ny < n) house_on_grid13[nx][ny].push_back(i);
        }
        rep(j,13){
            int nx = office[i].first + dx13[j];
            int ny = office[i].second + dy13[j];
            if(0 <= nx && nx < n && 0 <= ny && ny < n) office_on_grid13[nx][ny].push_back(i);
        }
    }

    // Zobrist Hash
    rep(i,n)rep(j,n) z_hash_grid[i][j] = xor128()%(int)1e9;

    rep(i,m) z_hash_house[i] = xor128()%(int)1e9;

    rep(i,m) z_hash_office[i] = xor128()%(int)1e9;

    // parameter
    connect_cnt_w = 400/m;
}

// // 操作をパスする
// void pass(int& cur_money, int& cur_income, int& cur_turn){
//     cur_turn++;
//     cur_money += cur_income;
// }

// // dist - 1 の線路を引く
// void construct(int dist,int& cur_money,int cur_income,int& cur_turn){
//     rep(i,dist - 1){
//         while(cur_money < rail_cost){
//             pass(cur_money,cur_income,cur_turn);
//         }
//         cur_turn++;
//         cur_money -= rail_cost;
//         cur_money += cur_income;
//     }
// }

// //駅からの距離 {距離、最寄り駅の座標}
// vector<vector<int>> calc_dist(vector<pair<int,int>>& station_pos){
//     vector<vector<int>> dist(n,vector<int>(n,(int)1e9));
//     queue<pair<int,int>> que;
//     for(auto [x,y] : station_pos){
//         que.push({x,y});
//         dist[x][y] = 0;
//     }
//     vector<int> dx = {1,0,-1,0};
//     vector<int> dy = {0,1,0,-1};
//     while(!que.empty()){
//         auto [x,y] = que.front();
//         que.pop();
//         rep(i,4){
//             int nx = x + dx[i];
//             int ny = y + dy[i];
//             if(0 <= nx && nx < n && 0 <= ny && ny < n && dist[nx][ny] == 1e9){
//                 dist[nx][ny] = dist[x][y] + 1;
//                 que.push({nx,ny});
//             }
//         }
//     }
//     return dist;
// }

// visの情報を更新する
void renew_vis(int x,int y,array<bool,1600>& vis_office, array<bool,1600>& vis_house){
    rep(i,13){
        int nx = x + dx13[i];
        int ny = y + dy13[i];
        if(0 <= nx && nx < n && 0 <= ny && ny < n){
            for(int idx : house_on_grid[nx][ny]) vis_house[idx] = true;
            for(int idx : office_on_grid[nx][ny]) vis_office[idx] = true;
        }
    }
}

// 収入を計算する
int calc_income(array<bool,1600>& vis_office, array<bool,1600>& vis_house){
    int ret = 0;
    rep(i,m) if(vis_house[i] && vis_office[i]) ret += abs(house[i].first - office[i].first) + abs(house[i].second - office[i].second);
    return ret;
}

// // 駅を立てる、収入の更新、所持金の更新もする
// void make_station(int x,int y,int& cur_income,int& cur_money,vector<pair<int,int>>& ret,vector<bool>& vis_office, vector<bool>& vis_house, int& cur_turn){
//     // お金がないときは待つ
//     while(cur_money < station_cost){
//         pass(cur_money,cur_income,cur_turn);
//     }

//     // 所持金を減らす
//     cur_money -= station_cost;
//     assert(cur_money >= 0);

//     cur_turn++;

//     ret.push_back({x,y});

//     // visを更新
//     renew_vis(x,y,vis_office,vis_house);

//     // 収入を更新
//     cur_income = calc_income(vis_office,vis_house);

//     // 増やす
//     cur_money += cur_income;
// }



// // vにxが含まれるか
// bool contains(vector<int>& v,int x){
//     rep(i,v.size()) if(v[i] == x) return true;
//     return false;
// }

// // 貪欲のスコアを計算する
// int calc_score(vector<bool>& vis_house, vector<bool>& vis_office, int sx,int sy,int tx = -1,int ty = -1){
//     // parameter
//     int not_connect_w = 1;
//     // 13近傍
//     vector<int> dx = {0,1,0,-1,0,1,-1,-1,1,2,0,-2,0};
//     vector<int> dy = {0,0,1,0,-1,1,1,-1,-1,0,2,0,-2};

//     // 今回で追加される家のidx
//     vector<int> add_house_idx;

//     // 今回で追加されるオフィスのidx
//     vector<int> add_office_idx;

//     int not_connect_cnt = 0;

//     int ret = 0;
//     rep(i,13){
//         int nx = sx + dx[i];
//         int ny = sy + dy[i];
//         if(0 <= nx && nx < n && 0 <= ny && ny < n){
//             for(int j : house_on_grid[nx][ny]){ 
//                 if(!vis_house[j] && vis_office[j]){ // まだ家が接続されていないかつオフィスが接続されている
//                     ret += commute_dist[j];
//                 }else if(!vis_house[j] && contains(add_office_idx,j)){ // まだ家が接続されていないかつオフィスが接続されている
//                     ret += commute_dist[j];
//                     not_connect_cnt--;
//                 }else if(!vis_house[j] && !contains(add_house_idx,j)){
//                     add_house_idx.push_back(j);
//                     not_connect_cnt++;
//                 }
//             }

//             for(int j : office_on_grid[nx][ny]){ 
//                 if(!vis_office[j] && vis_house[j]){ // まだオフィスが接続されていないかつ家が接続されている
//                     ret += commute_dist[j];
//                 }else if(!vis_office[j] && contains(add_house_idx,j)){ // まだオフィスが接続されていないかつ家が接続されている
//                     ret += commute_dist[j];
//                     not_connect_cnt--;
//                 }else if(!vis_office[j] && !contains(add_office_idx,j)){
//                     add_office_idx.push_back(j);
//                     not_connect_cnt++;
//                 }
//             }
//         }
//     }
//     // txが定義されないときはここでreturn
//     if(tx == -1) return ret + not_connect_cnt*not_connect_w;

//     rep(i,13){
//         int nx = tx + dx[i];
//         int ny = ty + dy[i];
//         if(0 <= nx && nx < n && 0 <= ny && ny < n){
//             for(int j : house_on_grid[nx][ny]){ 
//                 if(!vis_house[j] && vis_office[j]){ // まだ家が接続されていないかつオフィスが接続されている
//                     ret += commute_dist[j];
//                 }else if(!vis_house[j] && contains(add_office_idx,j)){ // まだ家が接続されていないかつオフィスが接続されている
//                     ret += commute_dist[j];
//                     not_connect_cnt--;
//                 }else if(!vis_house[j] && !contains(add_house_idx,j)){
//                     add_house_idx.push_back(j);
//                     not_connect_cnt++;
//                 }
//             }

//             for(int j : office_on_grid[nx][ny]){ 
//                 if(!vis_office[j] && vis_house[j]){ // まだオフィスが接続されていないかつ家が接続されている
//                     ret += commute_dist[j];
//                 }else if(!vis_office[j] && contains(add_house_idx,j)){ // まだオフィスが接続されていないかつ家が接続されている
//                     ret += commute_dist[j];
//                     not_connect_cnt--;
//                 }else if(!vis_office[j] && !contains(add_office_idx,j)){
//                     add_office_idx.push_back(j);
//                     not_connect_cnt++;
//                 }
//             }
//         }
//     }

//     return ret + not_connect_cnt*not_connect_w;
// }

// double calc_final_score(int cur_score,int cur_cost,int cur_op_cnt,int cur_money,int cur_income){
//     if(cur_income >= 10000) return (double)cur_score/cur_op_cnt;
//     return (double)cur_score/cur_cost/sqrt(cur_op_cnt);
// }


// vector<pair<int,int>> greedy(){
//     int cur_money = init_money;
//     int cur_income = 0;
//     int cur_turn = 0;
//     vector<pair<int,int>> ret;
//     // すでに家が線路に接続している
//     vector<bool> vis_house(m,false);
//     // すでにオフィスが線路に接続している
//     vector<bool> vis_office(m,false);

//     int finish_turn = 1000;

//     vector<pair<int,int>> mx_ans_score_ret;
//     int mx_ans_score = 0;
    
//     {// 最初の一手
//         // すべての人の家とオフィスをつなぐ候補を列挙する
//         vector<pair<pair<int,int>,pair<int,int>> > cand;
//         rep(i,m){
//             if(vis_house[i] && vis_office[i]) continue;
//             int sx = house[i].first,sy = house[i].second;
//             int tx = office[i].first,ty = office[i].second;
//             rep(j,13){
//                 int nsx = sx + dx13[j], nsy = sy + dy13[j];
//                 if(0 <= nsx && nsx < n && 0 <= nsy && nsy < n){
//                     rep(k,13){
//                         int ntx = tx + dx13[k], nty = ty + dy13[k];
//                         if(0 <= ntx && ntx < n && 0 <= nty && nty < n) cand.push_back({{nsx,nsy},{ntx,nty}});
//                     }
//                 }
//             }
//         }
//         // 重複を取り除く
//         rep(i,cand.size()) if(cand[i].first > cand[i].second) swap(cand[i].first, cand[i].second);
//         sort(ALL(cand));
//         cand.erase(unique(ALL(cand)),cand.end());

//         // 各スコアを計算する (含まれるpairの数)/(距離)
//         // めんどい後回し

//         //収入が一番高くなるやつ
//         int mx_score = 0, mx_score_pos = -1;
//         rep(i,cand.size()){
//             auto [sx,sy] = cand[i].first;
//             auto [tx,ty] = cand[i].second;
//             int dist = abs(tx - sx) + abs(ty - sy);
//             if((dist - 1)*rail_cost + 2*station_cost > cur_money) continue;
//             int cur_score = calc_score(vis_house,vis_office,sx,sy,tx,ty);
//             if(cur_score > mx_score){
//                 mx_score = cur_score;
//                 mx_score_pos = i;
//             }
//         }
//         auto [sx,sy] = cand[mx_score_pos].first;
//         auto [tx,ty] = cand[mx_score_pos].second;

//         construct(abs(sx - tx) + abs(sy - ty),cur_money,cur_income,cur_turn);

//         make_station(sx,sy,cur_income,cur_money,ret,vis_office,vis_house,cur_turn);
//         make_station(tx,ty,cur_income,cur_money,ret,vis_office,vis_house,cur_turn);
//     }

//     // 2手目以降は貪欲に領域を伸ばす // TODO 13近傍でやる
//     while(cur_turn < finish_turn){
//         vector<vector<int>> dist = calc_dist(ret);
//         int mx_score_op_cnt = 1e9;
//         int mx_score_cost = 1e9;
//         int mx_score_x = -1;
//         int mx_score_y = -1;
//         double mx_score = -1e9;
//         rep(x,n)rep(y,n){
//             if(dist[x][y] == 0) continue; //駅
//             int cur_cost = 0;
//             int cur_op_cnt = 0;
//             int cur_score = 0;
                               
//             cur_score = calc_score(vis_house,vis_office,x,y);
//             cur_op_cnt = dist[x][y];
//             cur_cost = (dist[x][y] - 1)*rail_cost + station_cost;

//             if(mx_score < calc_final_score(cur_score,cur_cost,cur_op_cnt,cur_money,cur_income)){
//                 mx_score = calc_final_score(cur_score,cur_cost,cur_op_cnt,cur_money,cur_income);
//                 mx_score_op_cnt = cur_op_cnt;
//                 mx_score_cost = cur_cost;
//                 mx_score_x = x;
//                 mx_score_y = y;
//             }
//         }

//         if(mx_score_x == -1) break; // 候補なし
           
//         // 操作がtに収まらない　または　お金がたまらなさそう
//         if(cur_turn + mx_score_op_cnt > finish_turn || cur_turn + max(0,(mx_score_cost - cur_money + cur_income - 1)/cur_income) + 1 > finish_turn) break;
        
//         // TODO 両方接続してないときは線路を先につないだ方がお金が節約できる
//         // TODO 先に接続したほうから伸ばした方がいい場合がある
//         // 線路の上に駅を生やすとき以外
//         if(mx_score_op_cnt != 1){
//             construct(dist[mx_score_x][mx_score_y],cur_money,cur_income,cur_turn);
//         }
//         make_station(mx_score_x,mx_score_y,cur_income,cur_money,ret,vis_office,vis_house,cur_turn);
     
//         assert(cur_money >= 0);

//         // この段階で操作をやめるとして、今の答えを評価する
//         int cur_ans_score = cur_money + cur_income*(t - cur_turn);

//         if(cur_ans_score > mx_ans_score){
//             mx_ans_score = cur_ans_score;
//             mx_ans_score_ret = ret;
//         }
//     }

//     // 一番いい所で操作をやめたやつを返す
//     // ret = mx_ans_score_ret;
//     cout << "# cancel = " << cur_turn << endl;
//     // cout << "# money = " << cur_money << endl;
//     // cout << "# income = " << cur_income << endl;
//     return ret;
// }

// ２点間を線路で結ぶ、最短距離を維持しながら寄り道できる場合は再帰する
// 返り値は{(sx,sy)につながる線路の位置, (tx,ty)につながる線路の位置}
pair<int,int> construct_yorimichi(int sx,int sy,int tx,int ty, array<array<int,50>,50>& cur_grid, vector<tuple<int,int,int>>& ans, int& cur_money, int cur_income, vector<pair<int,int>>& station_pos){
    array<array<int,50>,50> dist;
    rep(i,n)rep(j,n) dist[i][j] = (int)1e9;
    dist[sx][sy] = 0;
    queue<pair<int,int>> que;
    que.push({sx,sy});
    
    while(!que.empty()){
        auto [x,y] = que.front();
        que.pop();
        rep(i,4){
            int nx = x + dx4[i];
            int ny = y + dy4[i];
            if(0 <= nx && nx < n && 0 <= ny && ny < n && dist[nx][ny] == 1e9 && cur_grid[nx][ny] <= 0){
                dist[nx][ny] = dist[x][y] + 1;
                que.push({nx,ny});
            }
        }
    }
    
    if(dist[tx][ty] == 1e9){
        // あんまり良くない
        cout << "# Error can not reachable" << '\n';
        return {-1,-1};
    }

    // 最短経路で寄り道できる
    array<array<bool,50>,50> can_pass;
    rep(i,n)rep(j,n) can_pass[i][j] = false;
    assert(que.empty());
    can_pass[tx][ty] = true;
    que.push({tx,ty});

    while(!que.empty()){
        auto [x,y] = que.front();
        que.pop();
        rep(i,4){
            int nx = x + dx4[i];
            int ny = y + dy4[i];
            if(0 <= nx && nx < n && 0 <= ny && ny < n && !can_pass[nx][ny] && dist[nx][ny] + 1 == dist[x][y]){
                que.push({nx,ny});
                can_pass[nx][ny] = true;
            }
        }
    }

    // staton_pos を昇順にみて一番最初に寄り道できる所で再帰
    for(auto [x,y] : station_pos){
        if(x == sx && y == sy) continue;
        if(x == tx && y == ty) continue;
        if(can_pass[x][y]){
            pair<int,int> ret1 = construct_yorimichi(sx,sy,x,y,cur_grid,ans,cur_money,cur_income,station_pos);
            pair<int,int> ret2 = construct_yorimichi(x,y,tx,ty,cur_grid,ans,cur_money,cur_income,station_pos);

            if(ret1.first == -1 || ret2.first == -1) return {-1,-1};

            // {x,y} に線路が設置されないので補完する
            assert(ret1.second != ret2.first);
            int rail_kind = -1;
            // d : v > ^ < 
            if((ret1.second == 1 && ret2.first == 3) || (ret1.second == 3 && ret2.first == 1)) rail_kind = 1;
            if((ret1.second == 0 && ret2.first == 2) || (ret1.second == 2 && ret2.first == 0)) rail_kind = 2;
            if((ret1.second == 0 && ret2.first == 3) || (ret1.second == 3 && ret2.first == 0)) rail_kind = 3;
            if((ret1.second == 2 && ret2.first == 3) || (ret1.second == 3 && ret2.first == 2)) rail_kind = 4;
            if((ret1.second == 1 && ret2.first == 2) || (ret1.second == 2 && ret2.first == 1)) rail_kind = 5;
            if((ret1.second == 0 && ret2.first == 1) || (ret1.second == 1 && ret2.first == 0)) rail_kind = 6;
            assert(rail_kind != -1);

            // お金がないときは待つ
            while(cur_money < rail_cost){
                cur_money += cur_income;
                ans.push_back({-1,-1,-1});
            }

            // 所持金を減らす 
            cur_money -= rail_cost;
            assert(cur_money >= 0);

            // 所持金を増やす
            cur_money += cur_income;

            ans.push_back({rail_kind,x,y});
            cur_grid[x][y] = rail_kind;

            return {ret1.first,ret2.second};
        }
    }

    // 寄り道できるところがない


    // pathのoffice_on_grid13 + house_on_grid13が最大となるようにdpする

    array<array<int,50>,50> dp;
    rep(i,n)rep(j,n) dp[i][j] = (int)-1e9;
    array<array<pair<int,int>,50>,50> prev;
    rep(i,n)rep(j,n) prev[i][j] = {-1,-1};

    assert(que.empty()); //再利用
    que.push({tx,ty});
    dp[tx][ty] = office_on_grid13[tx][ty].size() + house_on_grid13[tx][ty].size();

    while(!que.empty()){
        auto [x, y] = que.front();
        que.pop();
        rep(i,4){
            int nx = x + dx4[i];
            int ny = y + dy4[i];
            if(0 <= nx && nx < n && 0 <= ny && ny < n && dist[nx][ny] + 1 == dist[x][y]){
                if(dp[nx][ny] == (int)-1e9) que.push({nx,ny});
                if(dp[nx][ny] < dp[x][y] + (int)house_on_grid13[nx][ny].size() + (int)office_on_grid13[nx][ny].size()){
                    dp[nx][ny] = dp[x][y] + (int)house_on_grid13[nx][ny].size() + (int)office_on_grid13[nx][ny].size();
                    prev[nx][ny] = {x,y};
                }
            }
        }
    }

    // DPの復元
    vector<int> dir;
    vector<pair<int,int>> pos;

    int x = sx;
    int y = sy;
    while(!(x == tx && y == ty)){
        rep(i,4){
            int nx = x + dx4[i];
            int ny = y + dy4[i];
            if(prev[x][y] == make_pair(nx,ny)){
                x = nx;
                y = ny;
                dir.push_back(i);
                pos.push_back({nx,ny});
                break;
            }
            assert(i != 3);
        }
    }

    // 矢印 -> 答え
    // d : v > ^ < 
    for(int i = 0;i < (int)dir.size() - 1;i++){
        if(cur_grid[pos[i].first][pos[i].second] == 0) continue;
        assert(cur_grid[pos[i].first][pos[i].second] == -1);

        // お金がないときは待つ
        while(cur_money < rail_cost){
            cur_money += cur_income;
            ans.push_back({-1,-1,-1});
        }

        // 所持金を減らす 
        cur_money -= rail_cost;
        assert(cur_money >= 0);

        // 所持金を増やす
        cur_money += cur_income;

        if((dir[i] == 1 && dir[i + 1] == 1) || (dir[i] == 3 && dir[i + 1] == 3)) ans.push_back({1,pos[i].first,pos[i].second});
        if((dir[i] == 0 && dir[i + 1] == 0) || (dir[i] == 2 && dir[i + 1] == 2)) ans.push_back({2,pos[i].first,pos[i].second});
        if((dir[i] == 1 && dir[i + 1] == 0) || (dir[i] == 2 && dir[i + 1] == 3)) ans.push_back({3,pos[i].first,pos[i].second});
        if((dir[i] == 1 && dir[i + 1] == 2) || (dir[i] == 0 && dir[i + 1] == 3)) ans.push_back({4,pos[i].first,pos[i].second});
        if((dir[i] == 0 && dir[i + 1] == 1) || (dir[i] == 3 && dir[i + 1] == 2)) ans.push_back({5,pos[i].first,pos[i].second});
        if((dir[i] == 2 && dir[i + 1] == 1) || (dir[i] == 3 && dir[i + 1] == 0)) ans.push_back({6,pos[i].first,pos[i].second});
        cur_grid[pos[i].first][pos[i].second] = get<0>(ans.back());
    }
    return {dir[0], (dir.back() + 2)%4};
}

void make_station_rail(int x,int y,int& cur_income,int& cur_money,vector<tuple<int,int,int>>& ans,array<bool,1600>& vis_office, array<bool,1600>& vis_house, array<array<int,50>,50>& cur_grid){
    // お金がないときは待つ
    while(cur_money < station_cost){
        cur_money += cur_income;
        ans.push_back({-1,-1,-1});
    }

    // 所持金を減らす
    cur_money -= station_cost;
    assert(cur_money >= 0);

    ans.push_back({0,x,y});
    cur_grid[x][y] = 0;

    // visを更新
    renew_vis(x,y,vis_office,vis_house);

    // 収入を更新
    cur_income = calc_income(vis_office,vis_house);

    // 増やす
    cur_money += cur_income;
}

//駅からの距離 {距離、最寄り駅の座標}
array<array<pair<int,pair<int,int>>,50>,50> calc_dist_rail(array<array<int,50>,50>& cur_grid){
    array<array<pair<int,pair<int,int>>,50>,50> dist;
    rep(i,n)rep(j,n) dist[i][j] = {(int)1e9,{-1,-1}};
    queue<pair<int,int>> que;
    rep(i,n)rep(j,n)if(cur_grid[i][j] == 0){
        que.push({i,j});
        dist[i][j] = {0,{i,j}};
    }
    while(!que.empty()){
        auto [x,y] = que.front();
        que.pop();
        rep(i,4){
            int nx = x + dx4[i];
            int ny = y + dy4[i];
            if(0 <= nx && nx < n && 0 <= ny && ny < n && dist[nx][ny].first == 1e9 && cur_grid[nx][ny] == -1){
                dist[nx][ny] = {dist[x][y].first + 1,dist[x][y].second};
                que.push({nx,ny});
            }
        }
    }
    return dist;
}

struct need_yaki{
    // 収入がある一定以上になるstaion_posのidx
    int yaki_l = -1;
    int yaki_turn_l = -1;
    array<array<int,50>,50> base_cur_grid;
    int score = 0;
};

// 駅を作る座標とその順番から答えを作成
pair<need_yaki,vector<tuple<int,int,int>>> greedy_rail(vector<pair<int,int>> station_pos){
    int cur_money = init_money;
    int cur_income = 0;
    need_yaki ret_yaki;
    // 今まで最もよい操作
    vector<tuple<int,int,int>> mx_score_ans;
    int mx_score = 0;
    // 答え
    vector<tuple<int,int,int>> ans;
    ans.reserve(t);
    //現在の盤面
    array<array<int,50>,50> cur_grid;
    rep(i,n)rep(j,n) cur_grid[i][j] = -1;
    // すでに家が線路に接続している
    array<bool,1600> vis_house;
    rep(i,m) vis_house[i] = false;
    // すでにオフィスが線路に接続している
    array<bool,1600> vis_office;
    rep(i,m) vis_office[i] = false;

    // 一番最初は駅を作る
    make_station_rail(station_pos[0].first,station_pos[0].second,cur_income,cur_money,ans,vis_office,vis_house,cur_grid);

    for(int i = 1;i < station_pos.size();i++){
        auto [x, y] = station_pos[i];
        // 駅の建設予定地に線路がある場合
        if(cur_grid[x][y] >= 1){
            make_station_rail(x,y,cur_income,cur_money,ans,vis_office,vis_house,cur_grid);
            continue;
        }

        array<array<pair<int,pair<int,int>>,50>,50> dist = calc_dist_rail(cur_grid);
        // 最寄りの駅が存在する
        if(dist[x][y].second.first == -1){
            cout << "# Error don't exsist nearly station" << '\n';
            continue;
        }

        // 線路を引く
        if(construct_yorimichi(dist[x][y].second.first,dist[x][y].second.second,x,y,cur_grid,ans,cur_money,cur_income,station_pos).first == -1) continue;

        // 駅を作る
        make_station_rail(x,y,cur_income,cur_money,ans,vis_office,vis_house,cur_grid);

        if((cur_money >= yaki_start_money && cur_income >= yaki_start_income) && ret_yaki.yaki_l == -1){
            ret_yaki.yaki_l = i + 1;
            ret_yaki.yaki_turn_l = ans.size();
            ret_yaki.base_cur_grid = cur_grid;
        }

        // 途中でやめる
        if(ans.size() > t) break;
        int cur_score = cur_money + cur_income*(t - ans.size());
        if(cur_score > mx_score){
            mx_score = cur_score;
            mx_score_ans = ans;
        }
    }
    ans = mx_score_ans;
    ret_yaki.score = mx_score;
    assert(ans.size() <= t);
    while(ans.size() != t) ans.push_back({-1,-1,-1});
    return {ret_yaki, ans};
}

// // 最初の1手
// pair<beam_search::State,pair<pair<int,int>,pair<int,int>> > first_step(){
//     // すべての人の家とオフィスをつなぐ候補を列挙する
//     vector<pair<pair<int,int>,pair<int,int>> > cand;
//     rep(i,m){
//         int sx = house[i].first,sy = house[i].second;
//         int tx = office[i].first,ty = office[i].second;
//         rep(j,13){
//             int nsx = sx + dx13[j], nsy = sy + dy13[j];
//             if(0 <= nsx && nsx < n && 0 <= nsy && nsy < n){
//                 rep(k,13){
//                     int ntx = tx + dx13[k], nty = ty + dy13[k];
//                     if(0 <= ntx && ntx < n && 0 <= nty && nty < n) cand.push_back({{nsx,nsy},{ntx,nty}});
//                 }
//             }
//         }
//     }
//     // 重複を取り除く
//     rep(i,cand.size()) if(cand[i].first > cand[i].second) swap(cand[i].first, cand[i].second);
//     sort(ALL(cand));
//     cand.erase(unique(ALL(cand)),cand.end());

//     // 各スコアを計算する (含まれるpairの数)/(距離)
//     // めんどい後回し

//     //収入が一番高くなるやつ
//     int mx_score = 0, mx_score_pos = -1;
//     vector<bool> vis_house(m,false); //めんどくさいから作ったけどいらない
//     vector<bool> vis_office(m,false);
//     rep(i,cand.size()){
//         auto [sx,sy] = cand[i].first;
//         auto [tx,ty] = cand[i].second;
//         int dist = abs(tx - sx) + abs(ty - sy);
//         if((dist - 1)*rail_cost + 2*station_cost > init_money) continue;
//         int cur_score = calc_score(vis_house,vis_office,sx,sy,tx,ty);
//         if(cur_score > mx_score){
//             mx_score = cur_score;
//             mx_score_pos = i;
//         }
//     }
//     auto [sx,sy] = cand[mx_score_pos].first;
//     auto [tx,ty] = cand[mx_score_pos].second;

//     beam_search::State ret;
//     ret.cur_turn = abs(sx - tx) + abs(sy - ty);
//     ret.cur_money = init_money - (abs(sx - tx) + abs(sy - ty) - 1)*rail_cost - station_cost*2;
//     assert(ret.cur_money >= 0);
//     ret.station_pos.push_back({sx,sy});
//     ret.station_pos.push_back({tx,ty});

//     ret.vis_house.assign(m,0);
//     ret.vis_office.assign(m,0);
    
//     for(int idx : house_on_grid13[sx][sy]) ret.vis_house[idx]++;
//     for(int idx : office_on_grid13[sx][sy]) ret.vis_office[idx]++;

//     for(int idx : house_on_grid13[tx][ty]) ret.vis_house[idx]++;
//     for(int idx : office_on_grid13[tx][ty]) ret.vis_office[idx]++;

//     ret.cur_income = 0;
//     // 収入を計算
//     rep(i,m) if(ret.vis_house[i] && ret.vis_office[i]) ret.cur_income += commute_dist[i];

//     // 接続されていない家やオフィス
//     ret.connect_cnt = 0;
//     rep(i,m) if(ret.vis_office[i]) ret.connect_cnt++;
//     rep(i,m) if(ret.vis_house[i]) ret.connect_cnt++;

//     return {ret,{{sx,sy},{tx,ty}}};
// }

// 焼きなましを行うためのstruct
struct status{
    // 十分に稼ぎがないときは変更しない
    vector<pair<int,int>> base_station_pos;
    vector<pair<int,int>> station_pos;
    // base_station_pos からの距離
    array<array<int,50>,50> base_dist;
    int yaki_l;
    int yaki_turn_l;
    bitset<1600> base_vis_office;
    bitset<1600> base_vis_house;
    array<array<bool,50>,50> is_station;

    double start_temp = -1;
    double end_temp = 0;

    status(vector<pair<int,int>>& _station_pos, int _yaki_l, int _yaki_turn_l, array<array<int,50>,50>& cur_grid) : yaki_l(_yaki_l), yaki_turn_l(_yaki_turn_l){
        assert(yaki_l != -1);
        rep(i,m) base_vis_house[i] = false;
        rep(i,m) base_vis_office[i] = false;
        rep(i,n)rep(j,n) is_station[i][j] = false;
        rep(i,yaki_l){
            base_station_pos.push_back(_station_pos[i]);
            auto [x,y] = _station_pos[i];
            is_station[x][y] = true;
            for(auto idx : house_on_grid13[x][y]) base_vis_house[idx] = true;
            for(auto idx : office_on_grid13[x][y]) base_vis_office[idx] = true;
        }
        for(int i = yaki_l;i < _station_pos.size();i++){
            is_station[_station_pos[i].first][_station_pos[i].second] = true;
            station_pos.push_back(_station_pos[i]);
        }

        
        rep(i,n)rep(j,n) base_dist[i][j] = (int)1e9;
        queue<pair<int,int>> que;
        for(auto [x,y] : base_station_pos){
            base_dist[x][y] = 0;
            que.push({x,y});
        }

        while(!que.empty()){
            auto [x,y] = que.front();
            que.pop();
            rep(i,4){
                int nx = x + dx4[i];
                int ny = y + dy4[i];
                if(0 <= nx && nx < n && 0 <= ny && ny < n && base_dist[nx][ny] == (int)1e9 && cur_grid[nx][ny] == -1){
                    base_dist[nx][ny] = base_dist[x][y] + 1;
                    que.push({nx,ny});
                }
            }
        }
    }

    // {score, turn超過したidx}
    pair<int,int> calc_score(vector<pair<int,int>>& cur_station_pos){
        bitset<1600> vis_house = base_vis_house;
        bitset<1600> vis_office = base_vis_office;
        int score = 0;
        int cur_turn = yaki_turn_l;
        rep(i,cur_station_pos.size()){
            auto [x,y] = cur_station_pos[i];
            // 増える収入
            int inc_income = 0;
            for(int idx : house_on_grid13[x][y]){
                if(!vis_house[idx]){
                    vis_house[idx] = true;
                    if(vis_office[idx]) inc_income += commute_dist[idx];
                }
            }
            for(int idx : office_on_grid13[x][y]){
                if(!vis_office[idx]){
                    vis_office[idx] = true;
                    if(vis_house[idx]) inc_income += commute_dist[idx];
                }
            }

            int use_turn = base_dist[x][y]; // baseからの距離
            if(use_turn == (int)1e9) use_turn = 1;
            if(use_turn != 1) rep(j,i){ //今までに追加した駅からの距離
                use_turn = min(use_turn, abs(x - cur_station_pos[j].first) + abs(y - cur_station_pos[j].second));
            }

            cur_turn += use_turn;
            if(cur_turn > t) return {score, i}; // ターンが超過した場合、評価をしない
            score += inc_income*(t - cur_turn) - station_cost - (use_turn - 1)*rail_cost;
        }
        return {score, station_pos.size()};
    }

    bool shift(double start_temp,double end_temp,double time_limit,double start_time,double scoredist,double now_time){
		long long INF = 1e18;
		double temp = start_temp + (end_temp - start_temp) * (now_time-start_time) / time_limit;  //線形でstart_tempからend_tempに変化する。
		double prob = exp(((double)scoredist)/temp); //scoredistが正のときは1負のときは1未満
		return (prob > (xor128()%INF)/(double)INF);
	}

    vector<pair<int,int>> yaki(){
        double start_time = (double)clock()/CLOCKS_PER_SEC;
        double cur_time = start_time;
        double end_time = 2.9;
        pair<int,int> score = calc_score(station_pos);
        start_temp = score.first/20000;
        
        for(int yaki_cnt = 0;true;yaki_cnt++){
            if(yaki_cnt%10 == 0) cur_time = (double)clock()/CLOCKS_PER_SEC;
            if(cur_time > end_time){
                cout << "# yaki_cnt = " << yaki_cnt << '\n';
                break;
            }
            array<int,4> choose;
            choose[0] = shift_rate[0];
            choose[1] = shift_rate[0] + shift_rate[1];
            choose[2] = shift_rate[0] + shift_rate[1] + shift_rate[2];
            choose[3] = shift_rate[0] + shift_rate[1] + shift_rate[2] + shift_rate[3];
            
            int op = xor128()%choose[3];
            if(op < choose[0]){ // swap
                if(station_pos.size() <= 1) continue;
                int i = xor128()%station_pos.size();
                int j = xor128()%station_pos.size();
                if(i == j) continue;
                
                swap(station_pos[i],station_pos[j]);

                pair<int,int> nscore = calc_score(station_pos);

                if(shift(start_temp,end_temp,end_time,start_time,nscore.first - score.first,cur_time)){
                    score = nscore;
                }else{
                    swap(station_pos[i],station_pos[j]);
                }
            }
            else if(op < choose[1]){ // insert
                if(score.second != station_pos.size()) continue; // ターン超過しているときは、挿入しない
                int x = xor128()%n;
                int y = xor128()%n;
                int i = xor128()%(station_pos.size() + 1);

                if(is_station[x][y]) continue;
                
                station_pos.insert(station_pos.begin() + i,{x,y});

                pair<int,int> nscore = calc_score(station_pos);

                if(shift(start_temp,end_temp,end_time,start_time,nscore.first - score.first,cur_time)){
                    is_station[x][y] = true;
                    score = nscore;
                }else{
                    station_pos.erase(station_pos.begin() + i);
                }
            }else if(op < choose[2]){ // delate
                if(station_pos.size() == 0) continue;
                if(score.second == 0) continue;
                int i = xor128()%(score.second); //ターン超過していない、評価されている部分を削除する

                pair<int,int> del = station_pos[i];
                
                station_pos.erase(station_pos.begin() + i);

                pair<int,int> nscore = calc_score(station_pos);

                if(shift(start_temp,end_temp,end_time,start_time,nscore.first - score.first,cur_time)){
                    is_station[del.first][del.second] = false;
                    score = nscore;
                }else{
                    station_pos.insert(station_pos.begin() + i,del);
                }
            }else if(op < choose[3]){ // shift
                if(station_pos.size() == 0) continue;
                if(score.second == 0) continue;
                int i = xor128()%(score.second);//ターン超過していない、評価されている部分を動かす
                int dir = xor128()%12;
                dir++; // 最初の{0,0} はいらない

                int nx = station_pos[i].first + dx13[dir];
                int ny = station_pos[i].second + dy13[dir];

                if(nx < 0 || n <= nx || ny < 0 || n <= ny || is_station[nx][ny]) continue;
                
                station_pos[i].first += dx13[dir];
                station_pos[i].second += dy13[dir];

                pair<int,int> nscore = calc_score(station_pos);

                if(shift(start_temp,end_temp,end_time,start_time,nscore.first - score.first,cur_time)){
                    is_station[nx - dx13[dir]][ny - dy13[dir]] = false;
                    is_station[nx][ny] = true;
                    score = nscore;
                }else{
                    station_pos[i].first -= dx13[dir];
                    station_pos[i].second -= dy13[dir];  
                }
            }
        }
        vector<pair<int,int>> ret = base_station_pos;
        rep(i,station_pos.size()) ret.push_back(station_pos[i]);
        return ret;
    }
};

int main(){
    input();
    // vector<pair<int,int>> station_pos= greedy();

    // 予想される手数
    int predict_turn = 0;
    if(m*init_money <= 1e6) predict_turn = 60;
    else if(m*init_money <= 2e6) predict_turn = 80;
    else if(m*init_money <= 3e6) predict_turn = 100;
    else if(m*init_money <= 5e6) predict_turn = 120;
    else if(m*init_money <= 1e7) predict_turn = 140;
    else predict_turn = 160;

    // ビームサーチの設定
    beam_search::Config config;
    config.beam_width = beam_width = 300000/predict_turn/sqrt(m);
    config.max_turn = 1e8; // 途中で止める実装にした
    config.hash_map_capacity = 1e4;
    config.nodes_capacity = 50*50*config.beam_width*10;

    // auto [sta,init_pos] = first_step();

    beam_search::State sta;
    beam_search::Hash hash = 0;
    // rep(i,m){
    //     if(sta.vis_house[i]) hash ^= z_hash_house[i];
    //     if(sta.vis_office[i]) hash ^= z_hash_office[i];
    // }
    beam_search::Node node(beam_search::Action(), beam_search::Evaluator(),hash);

    vector<vector<beam_search::Action>> act_v = beam_search::beam_search(config,sta,node);


    // vector<tuple<int,int,int>> mx_score_ans;
    // int mx_score = 0;

    // for(vector<beam_search::Action>& act : act_v){
    //     if((double)clock()/CLOCKS_PER_SEC > 2.9) break;
        
    //     vector<pair<int,int>> station_pos;
    //     rep(i,act.size()){
    //         if((act[i].x_y_turn_income_fin>>62) & 1) break;
    //         station_pos.push_back({(act[i].x_y_turn_income_fin) & ((1ULL<<6) - 1), (act[i].x_y_turn_income_fin>>6) & ((1ULL<<6) - 1) });
    //         if(act[i].x_y_turn_income_fin>>63){//first_step
    //             station_pos.push_back({(act[i].x_y_turn_income_fin>>50) & ((1ULL<<6) - 1), (act[i].x_y_turn_income_fin>>56) & ((1ULL<<6) - 1) });
    //         }
    //     }
    //     auto [cur_score, cur_ans] = greedy_rail(station_pos);
    //     if(mx_score < cur_score){
    //         mx_score = cur_score;
    //         mx_score_ans = cur_ans;
    //     }
    // }

    vector<pair<int,int>> station_pos;
    vector<beam_search::Action>& act = act_v[0];
    rep(i,act.size()){
        if((act[i].x_y_turn_income_fin>>62) & 1) break;
        station_pos.push_back({(act[i].x_y_turn_income_fin) & ((1ULL<<6) - 1), (act[i].x_y_turn_income_fin>>6) & ((1ULL<<6) - 1) });
        if(act[i].x_y_turn_income_fin>>63){//first_step
            station_pos.push_back({(act[i].x_y_turn_income_fin>>50) & ((1ULL<<6) - 1), (act[i].x_y_turn_income_fin>>56) & ((1ULL<<6) - 1) });
        }
    }

    pair<need_yaki,vector<tuple<int,int,int>> > ans = greedy_rail(station_pos);

    // 焼きなます場合
    if(ans.first.yaki_l != -1 && ans.first.yaki_l != station_pos.size()){
        status st(station_pos, ans.first.yaki_l, ans.first.yaki_turn_l, ans.first.base_cur_grid);
        cout << "# start_size = " << station_pos.size() << '\n';
        station_pos = st.yaki();
        cout << "# end_size = " << station_pos.size() << '\n';
        ans = greedy_rail(station_pos);
    }else{ // 焼かない場合
        // ほとんどそんざいしないらしい
        for(int k = 1;k < act_v.size();k++){
            if((double)clock()/CLOCKS_PER_SEC > 2.9) break;
            vector<beam_search::Action>& act = act_v[k];
            vector<pair<int,int>> station_pos;
            rep(i,act.size()){
                if((act[i].x_y_turn_income_fin>>62) & 1) break;
                station_pos.push_back({(act[i].x_y_turn_income_fin) & ((1ULL<<6) - 1), (act[i].x_y_turn_income_fin>>6) & ((1ULL<<6) - 1) });
                if(act[i].x_y_turn_income_fin>>63){//first_step
                    station_pos.push_back({(act[i].x_y_turn_income_fin>>50) & ((1ULL<<6) - 1), (act[i].x_y_turn_income_fin>>56) & ((1ULL<<6) - 1) });
                }
            }
            pair<need_yaki,vector<tuple<int,int,int>> > ans_cand = greedy_rail(station_pos);
            if(ans.first.score < ans_cand.first.score){
                ans = ans_cand;
            }
        }
    }
    int real_sz = 0;
    for(auto [out1, out2, out3] : ans.second) if(out1 == 0) real_sz++;
    cout << "# real_sz = " << real_sz << endl;
    for(auto [out1, out2, out3] : ans.second){
        if(out1 != -1) cout << out1 << ' ' << out2 << ' ' << out3 << '\n';
        else cout << -1 << '\n';
    }
}