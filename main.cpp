#include <bits/stdc++.h>
using namespace std;
using ll = long long;

#define rep(i, n) for (int i = 0; i < (int)(n); i++)
#define ALL(obj) (obj).begin(),(obj).end()

int n,m,init_money,t;
vector<pair<int,int>> house;
vector<pair<int,int>> office;
int station_cost = 5000;
int rail_cost = 100;
// そのマスにある家のidx
vector<vector<vector<int>>> house_on_grid;
// そのマスにあるオフィスのidx
vector<vector<vector<int>>> office_on_grid;

// そのマスにある家のidx (13近傍)
vector<vector<vector<int>>> house_on_grid13;
// そのマスにあるオフィスのidx(13近傍)
vector<vector<vector<int>>> office_on_grid13;

// その人の通勤距離
vector<int> commute_dist;

// 13近傍
vector<int> dx13 = {0,1,0,-1,0,1,-1,-1,1,2,0,-2,0};
vector<int> dy13 = {0,0,1,0,-1,1,1,-1,-1,0,2,0,-2};

// 4近傍
vector<int> dx4 = {1,0,-1,0};
vector<int> dy4 = {0,1,0,-1};

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
    int cur_income;
    int cur_turn;
    int cur_money;
    int station_x;
    int station_y;
    int fin = 0;
    Action() {
        cur_income = 0;
        cur_turn = 0;
        cur_money = 0;
        station_x = -1;
        station_y = -1;
        fin = 0;
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

//駅からの距離 {距離、最寄り駅の座標}
vector<vector<int>> calc_dist(vector<pair<int,int>>& station_pos){
    vector<vector<int>> dist(n,vector<int>(n,(int)1e9));
    queue<pair<int,int>> que;
    for(auto [x,y] : station_pos){
        que.push({x,y});
        dist[x][y] = 0;
    }
    vector<int> dx = {1,0,-1,0};
    vector<int> dy = {0,1,0,-1};
    while(!que.empty()){
        auto [x,y] = que.front();
        que.pop();
        rep(i,4){
            int nx = x + dx[i];
            int ny = y + dy[i];
            if(0 <= nx && nx < n && 0 <= ny && ny < n && dist[nx][ny] == 1e9){
                dist[nx][ny] = dist[x][y] + 1;
                que.push({nx,ny});
            }
        }
    }
    return dist;
}

pair<int,int> calc_inc_income(int x, int y, vector<int>& vis_house, vector<int>& vis_office, int no_connect_cnt){
    int ret = 0;
    for(int idx : house_on_grid13[x][y]){
        if(vis_house[idx] == 0){
            if(vis_office[idx] != 0){
                ret += commute_dist[idx];
                no_connect_cnt--;
            }else no_connect_cnt++;
        }
    }
    for(int idx : office_on_grid13[x][y]){
        if(vis_office[idx] == 0){
            if(vis_house[idx] != 0){
                ret += commute_dist[idx];
                no_connect_cnt--;
            }else no_connect_cnt++;
        }
    }
    return {ret,no_connect_cnt};
}

// 深さ優先探索に沿って更新する情報をまとめたクラス
class State {
    public:
        // explicit State(/* const Input& input */) {
        //     cur_money = 0;
        //     cur_income = 0;
        //     cur_turn = 0;
        //     vis_house.assign(n,false);
        //     vis_office.assign(n,false);
        // }

        // 次の状態候補を全てselectorに追加する
        // 引数
        //   evaluator : 今の評価器
        //   hash      : 今のハッシュ値
        //   parent    : 今のノードID（次のノードにとって親となる）
        void expand(const Evaluator& evaluator, Hash hash, int parent, Selector& selector) {
            {
                Action n_act;
                if(fin == 0) n_act.fin = 1;
                else n_act.fin = 0;
                selector.push(n_act,evaluator,hash,parent,false);
            }
            if(fin) return;
            vector<vector<int>> dist = calc_dist(station_pos);
            rep(x,n)rep(y,n){
                if(dist[x][y] == 0) continue; //駅
                auto [inc_income,n_no_connect_cnt] = calc_inc_income(x,y,vis_house,vis_office,no_connect_cnt);

                int cost = (dist[x][y] - 1)*rail_cost + station_cost;
                
                Action n_act;

                n_act.cur_income = inc_income;
                n_act.cur_turn = max(dist[x][y], (cost - cur_money + cur_income - 1)/cur_income + 1); // max(操作回数、お金がたまるまで)
                n_act.cur_money =  - cost + (n_act.cur_turn - 1)*cur_income + (cur_income + n_act.cur_income);
                n_act.station_x = x;
                n_act.station_y = y;
                assert(cur_money + n_act.cur_money >= 0);
                
                Evaluator n_eva;
                n_eva.score = (cur_money + n_act.cur_money) + max(0, (cur_income + n_act.cur_income)*(t - (cur_turn + n_act.cur_turn))) + n_no_connect_cnt*1000;

                Hash n_hash = random()%(int)1e9;
                
                selector.push(n_act,n_eva,n_hash,parent,false);
            }

        }

        // actionを実行して次の状態に遷移する
        void move_forward(Action action) {
            fin ^= action.fin;
            if(fin) return;
            cur_money += action.cur_money;
            cur_income += action.cur_income;
            cur_turn += action.cur_turn;
            station_pos.push_back({action.station_x, action.station_y});
            for(int idx : house_on_grid13[action.station_x][action.station_y]){
                vis_house[idx]++;
                if(vis_house[idx] == 1){
                   if(vis_office[idx] == 0) no_connect_cnt++;
                   else no_connect_cnt--;
                }
            }
            for(int idx : office_on_grid13[action.station_x][action.station_y]){
                vis_office[idx]++;
                if(vis_office[idx] == 1){
                    if(vis_house[idx] == 0) no_connect_cnt++;
                    else no_connect_cnt--;
                }
            }
        }

        // actionを実行する前の状態に遷移する
        // 今の状態は、親からactionを実行して遷移した状態である
        void move_backward(Action action) {
            fin ^= action.fin;
            if(fin ^ action.fin) return;
            cur_money -= action.cur_money;
            cur_income -= action.cur_income;
            cur_turn -= action.cur_turn;
            station_pos.pop_back();
            for(int idx : house_on_grid13[action.station_x][action.station_y]){
                vis_house[idx]--;
                if(vis_house[idx] == 0){
                   if(vis_office[idx] == 0) no_connect_cnt--;
                   else no_connect_cnt++;
                }
            }
            for(int idx : office_on_grid13[action.station_x][action.station_y]){
                vis_office[idx]--;
                if(vis_office[idx] == 0){
                    if(vis_house[idx] == 0) no_connect_cnt--;
                    else no_connect_cnt++;
                }
            }
        }

        int cur_money;
        int cur_income;
        int cur_turn;
        vector<int> vis_office;
        vector<int> vis_house;
        vector<pair<int,int>> station_pos;
        int fin = 0;
        int no_connect_cnt = 0;
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
        void dfs(Selector& selector) {
            update_root();

            int v = root_;
            while (true) {
                v = move_to_leaf(v);
                state_.expand(nodes_[v].evaluator, nodes_[v].hash, v, selector);
                v = move_to_ancestor(v);
                if (v == root_) {
                    break;
                }
                v = move_to_right(v);
            }
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
vector<Action> beam_search(const Config& config, State state, Node root) {
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
        tree.dfs(selector);

        if (selector.have_finished()) {
            // ターン数最小化型の問題で実行可能解が見つかったとき
            Candidate candidate = selector.get_finished_candidates()[0];
            vector<Action> ret = tree.get_path(candidate.parent);
            ret.push_back(candidate.action);
            return ret;
        }
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
    int best_leaf = tree.get_best_leaf(curr_nodes);
    return tree.get_path(best_leaf);
}

} // namespace beam_search

void input(){
    cin >> n >> m >> init_money >> t;
    house.assign(m,pair<int,int>());
    office.assign(m,pair<int,int>());
    house_on_grid.assign(n,vector<vector<int>>(n));
    office_on_grid.assign(n,vector<vector<int>>(n));
    house_on_grid13.assign(n,vector<vector<int>>(n));
    office_on_grid13.assign(n,vector<vector<int>>(n));
    commute_dist.assign(m,0);
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
}

// 操作をパスする
void pass(int& cur_money, int& cur_income, int& cur_turn){
    cur_turn++;
    cur_money += cur_income;
}

// dist - 1 の線路を引く
void construct(int dist,int& cur_money,int cur_income,int& cur_turn){
    rep(i,dist - 1){
        while(cur_money < rail_cost){
            pass(cur_money,cur_income,cur_turn);
        }
        cur_turn++;
        cur_money -= rail_cost;
        cur_money += cur_income;
    }
}

//駅からの距離 {距離、最寄り駅の座標}
vector<vector<int>> calc_dist(vector<pair<int,int>>& station_pos){
    vector<vector<int>> dist(n,vector<int>(n,(int)1e9));
    queue<pair<int,int>> que;
    for(auto [x,y] : station_pos){
        que.push({x,y});
        dist[x][y] = 0;
    }
    vector<int> dx = {1,0,-1,0};
    vector<int> dy = {0,1,0,-1};
    while(!que.empty()){
        auto [x,y] = que.front();
        que.pop();
        rep(i,4){
            int nx = x + dx[i];
            int ny = y + dy[i];
            if(0 <= nx && nx < n && 0 <= ny && ny < n && dist[nx][ny] == 1e9){
                dist[nx][ny] = dist[x][y] + 1;
                que.push({nx,ny});
            }
        }
    }
    return dist;
}

// visの情報を更新する
void renew_vis(int x,int y,vector<bool>& vis_office, vector<bool>& vis_house){
    // 13近傍
    vector<int> dx = {0,1,0,-1,0,1,-1,-1,1,2,0,-2,0};
    vector<int> dy = {0,0,1,0,-1,1,1,-1,-1,0,2,0,-2};
    rep(i,13){
        int nx = x + dx[i];
        int ny = y + dy[i];
        if(0 <= nx && nx < n && 0 <= ny && ny < n){
            for(int idx : house_on_grid[nx][ny]) vis_house[idx] = true;
            for(int idx : office_on_grid[nx][ny]) vis_office[idx] = true;
        }
    }
}

// 収入を計算する
int calc_income(vector<bool>& vis_office, vector<bool>& vis_house){
    int ret = 0;
    rep(i,m) if(vis_house[i] && vis_office[i]) ret += abs(house[i].first - office[i].first) + abs(house[i].second - office[i].second);
    return ret;
}

// 駅を立てる、収入の更新、所持金の更新もする
void make_station(int x,int y,int& cur_income,int& cur_money,vector<pair<int,int>>& ret,vector<bool>& vis_office, vector<bool>& vis_house, int& cur_turn){
    // お金がないときは待つ
    while(cur_money < station_cost){
        pass(cur_money,cur_income,cur_turn);
    }

    // 所持金を減らす
    cur_money -= station_cost;
    assert(cur_money >= 0);

    cur_turn++;

    ret.push_back({x,y});

    // visを更新
    renew_vis(x,y,vis_office,vis_house);

    // 収入を更新
    cur_income = calc_income(vis_office,vis_house);

    // 増やす
    cur_money += cur_income;
}



// vにxが含まれるか
bool contains(vector<int>& v,int x){
    rep(i,v.size()) if(v[i] == x) return true;
    return false;
}

// 貪欲のスコアを計算する
int calc_score(vector<bool>& vis_house, vector<bool>& vis_office, int sx,int sy,int tx = -1,int ty = -1){
    // parameter
    int not_connect_w = 1;
    // 13近傍
    vector<int> dx = {0,1,0,-1,0,1,-1,-1,1,2,0,-2,0};
    vector<int> dy = {0,0,1,0,-1,1,1,-1,-1,0,2,0,-2};

    // 今回で追加される家のidx
    vector<int> add_house_idx;

    // 今回で追加されるオフィスのidx
    vector<int> add_office_idx;

    int not_connect_cnt = 0;

    int ret = 0;
    rep(i,13){
        int nx = sx + dx[i];
        int ny = sy + dy[i];
        if(0 <= nx && nx < n && 0 <= ny && ny < n){
            for(int j : house_on_grid[nx][ny]){ 
                if(!vis_house[j] && vis_office[j]){ // まだ家が接続されていないかつオフィスが接続されている
                    ret += commute_dist[j];
                }else if(!vis_house[j] && contains(add_office_idx,j)){ // まだ家が接続されていないかつオフィスが接続されている
                    ret += commute_dist[j];
                    not_connect_cnt--;
                }else if(!vis_house[j] && !contains(add_house_idx,j)){
                    add_house_idx.push_back(j);
                    not_connect_cnt++;
                }
            }

            for(int j : office_on_grid[nx][ny]){ 
                if(!vis_office[j] && vis_house[j]){ // まだオフィスが接続されていないかつ家が接続されている
                    ret += commute_dist[j];
                }else if(!vis_office[j] && contains(add_house_idx,j)){ // まだオフィスが接続されていないかつ家が接続されている
                    ret += commute_dist[j];
                    not_connect_cnt--;
                }else if(!vis_office[j] && !contains(add_office_idx,j)){
                    add_office_idx.push_back(j);
                    not_connect_cnt++;
                }
            }
        }
    }
    // txが定義されないときはここでreturn
    if(tx == -1) return ret + not_connect_cnt*not_connect_w;

    rep(i,13){
        int nx = tx + dx[i];
        int ny = ty + dy[i];
        if(0 <= nx && nx < n && 0 <= ny && ny < n){
            for(int j : house_on_grid[nx][ny]){ 
                if(!vis_house[j] && vis_office[j]){ // まだ家が接続されていないかつオフィスが接続されている
                    ret += commute_dist[j];
                }else if(!vis_house[j] && contains(add_office_idx,j)){ // まだ家が接続されていないかつオフィスが接続されている
                    ret += commute_dist[j];
                    not_connect_cnt--;
                }else if(!vis_house[j] && !contains(add_house_idx,j)){
                    add_house_idx.push_back(j);
                    not_connect_cnt++;
                }
            }

            for(int j : office_on_grid[nx][ny]){ 
                if(!vis_office[j] && vis_house[j]){ // まだオフィスが接続されていないかつ家が接続されている
                    ret += commute_dist[j];
                }else if(!vis_office[j] && contains(add_house_idx,j)){ // まだオフィスが接続されていないかつ家が接続されている
                    ret += commute_dist[j];
                    not_connect_cnt--;
                }else if(!vis_office[j] && !contains(add_office_idx,j)){
                    add_office_idx.push_back(j);
                    not_connect_cnt++;
                }
            }
        }
    }

    return ret + not_connect_cnt*not_connect_w;
}

double calc_final_score(int cur_score,int cur_cost,int cur_op_cnt,int cur_money,int cur_income){
    if(cur_income >= 10000) return (double)cur_score/cur_op_cnt;
    return (double)cur_score/cur_cost/sqrt(cur_op_cnt);
}


vector<pair<int,int>> greedy(){
    int cur_money = init_money;
    int cur_income = 0;
    int cur_turn = 0;
    vector<pair<int,int>> ret;
    // すでに家が線路に接続している
    vector<bool> vis_house(m,false);
    // すでにオフィスが線路に接続している
    vector<bool> vis_office(m,false);

    int finish_turn = 1000;

    vector<pair<int,int>> mx_ans_score_ret;
    int mx_ans_score = 0;
    
    {// 最初の一手
        // すべての人の家とオフィスをつなぐ候補を列挙する
        vector<pair<pair<int,int>,pair<int,int>> > cand;
        rep(i,m){
            if(vis_house[i] && vis_office[i]) continue;
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

        // 各スコアを計算する (含まれるpairの数)/(距離)
        // めんどい後回し

        //収入が一番高くなるやつ
        int mx_score = 0, mx_score_pos = -1;
        rep(i,cand.size()){
            auto [sx,sy] = cand[i].first;
            auto [tx,ty] = cand[i].second;
            int dist = abs(tx - sx) + abs(ty - sy);
            if((dist - 1)*rail_cost + 2*station_cost > cur_money) continue;
            int cur_score = calc_score(vis_house,vis_office,sx,sy,tx,ty);
            if(cur_score > mx_score){
                mx_score = cur_score;
                mx_score_pos = i;
            }
        }
        auto [sx,sy] = cand[mx_score_pos].first;
        auto [tx,ty] = cand[mx_score_pos].second;

        construct(abs(sx - tx) + abs(sy - ty),cur_money,cur_income,cur_turn);

        make_station(sx,sy,cur_income,cur_money,ret,vis_office,vis_house,cur_turn);
        make_station(tx,ty,cur_income,cur_money,ret,vis_office,vis_house,cur_turn);
    }

    // 2手目以降は貪欲に領域を伸ばす // TODO 13近傍でやる
    while(cur_turn < finish_turn){
        vector<vector<int>> dist = calc_dist(ret);
        int mx_score_op_cnt = 1e9;
        int mx_score_cost = 1e9;
        int mx_score_x = -1;
        int mx_score_y = -1;
        double mx_score = -1e9;
        rep(x,n)rep(y,n){
            if(dist[x][y] == 0) continue; //駅
            int cur_cost = 0;
            int cur_op_cnt = 0;
            int cur_score = 0;
                               
            cur_score = calc_score(vis_house,vis_office,x,y);
            cur_op_cnt = dist[x][y];
            cur_cost = (dist[x][y] - 1)*rail_cost + station_cost;

            if(mx_score < calc_final_score(cur_score,cur_cost,cur_op_cnt,cur_money,cur_income)){
                mx_score = calc_final_score(cur_score,cur_cost,cur_op_cnt,cur_money,cur_income);
                mx_score_op_cnt = cur_op_cnt;
                mx_score_cost = cur_cost;
                mx_score_x = x;
                mx_score_y = y;
            }
        }

        if(mx_score_x == -1) break; // 候補なし
           
        // 操作がtに収まらない　または　お金がたまらなさそう
        if(cur_turn + mx_score_op_cnt > finish_turn || cur_turn + max(0,(mx_score_cost - cur_money + cur_income - 1)/cur_income) + 1 > finish_turn) break;
        
        // TODO 両方接続してないときは線路を先につないだ方がお金が節約できる
        // TODO 先に接続したほうから伸ばした方がいい場合がある
        // 線路の上に駅を生やすとき以外
        if(mx_score_op_cnt != 1){
            construct(dist[mx_score_x][mx_score_y],cur_money,cur_income,cur_turn);
        }
        make_station(mx_score_x,mx_score_y,cur_income,cur_money,ret,vis_office,vis_house,cur_turn);
     
        assert(cur_money >= 0);

        // この段階で操作をやめるとして、今の答えを評価する
        int cur_ans_score = cur_money + cur_income*(t - cur_turn);

        if(cur_ans_score > mx_ans_score){
            mx_ans_score = cur_ans_score;
            mx_ans_score_ret = ret;
        }
    }

    // 一番いい所で操作をやめたやつを返す
    // ret = mx_ans_score_ret;
    cout << "# cancel = " << cur_turn << endl;
    // cout << "# money = " << cur_money << endl;
    // cout << "# income = " << cur_income << endl;
    return ret;
}

// ２点間を線路で結ぶ、最短距離を維持しながら寄り道できる場合は再帰する
// 返り値は{(sx,sy)につながる線路の位置, (tx,ty)につながる線路の位置}
pair<int,int> construct_yorimichi(int sx,int sy,int tx,int ty, vector<vector<int>>& cur_grid, vector<tuple<int,int,int>>& ans, int& cur_money, int cur_income, vector<pair<int,int>>& station_pos){
    vector<vector<int>> dist(n,vector<int>(n,1e9));
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
    vector<vector<bool>> can_pass(n,vector<bool>(n,false));
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
    // 矢印の列挙
    vector<int> dir;
    vector<pair<int,int>> pos;
    int x = tx,y = ty;
    while(!(x == sx && y == sy)){
        rep(i,4){
            int nx = x + dx4[i];
            int ny = y + dy4[i];
            if(0 <= nx && nx < n && 0 <= ny && ny < n && dist[nx][ny] + 1 == dist[x][y]){
                dir.push_back(i);
                pos.push_back({nx,ny});
                x = nx;
                y = ny;
                break;
            }
            assert(i != 3);
        }
    }

    // 矢印 -> 答え
    // d : v > ^ < 
    int pre_ans_sz = ans.size();
    for(int i = 0;i < dir.size() - 1;i++){
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
    return {(dir.back() + 2)%4, dir[0]};
}

void make_station_rail(int x,int y,int& cur_income,int& cur_money,vector<tuple<int,int,int>>& ans,vector<bool>& vis_office, vector<bool>& vis_house, vector<vector<int>>& cur_grid){
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
vector<vector<pair<int,pair<int,int>>>> calc_dist_rail(vector<vector<int>>& cur_grid){
    vector<vector<pair<int,pair<int,int>>>> dist(n,vector<pair<int,pair<int,int>>>(n,{(int)1e9,{-1,-1}}));
    queue<pair<int,int>> que;
    rep(i,n)rep(j,n)if(cur_grid[i][j] == 0){
        que.push({i,j});
        dist[i][j] = {0,{i,j}};
    }
    vector<int> dx = {1,0,-1,0};
    vector<int> dy = {0,1,0,-1};
    while(!que.empty()){
        auto [x,y] = que.front();
        que.pop();
        rep(i,4){
            int nx = x + dx[i];
            int ny = y + dy[i];
            if(0 <= nx && nx < n && 0 <= ny && ny < n && dist[nx][ny].first == 1e9 && cur_grid[nx][ny] == -1){
                dist[nx][ny] = {dist[x][y].first + 1,dist[x][y].second};
                que.push({nx,ny});
            }
        }
    }
    return dist;
}

// 駅を作る座標とその順番から答えを作成
vector<tuple<int,int,int>> greedy_rail(vector<pair<int,int>> station_pos){
    int cur_money = init_money;
    int cur_income = 0;
    // 今まで最もよい操作
    vector<tuple<int,int,int>> mx_score_ans;
    int mx_score = 0;
    // 答え
    vector<tuple<int,int,int>> ans;
    //現在の盤面
    vector<vector<int>> cur_grid(n,vector<int>(n,-1));
    // すでに家が線路に接続している
    vector<bool> vis_house(m,false);
    // すでにオフィスが線路に接続している
    vector<bool> vis_office(m,false);

    // 一番最初は駅を作る
    make_station_rail(station_pos[0].first,station_pos[0].second,cur_income,cur_money,ans,vis_office,vis_house,cur_grid);

    for(int i = 1;i < station_pos.size();i++){
        auto [x, y] = station_pos[i];
        // 駅の建設予定地に線路がある場合
        if(cur_grid[x][y] >= 1){
            make_station_rail(x,y,cur_income,cur_money,ans,vis_office,vis_house,cur_grid);
            continue;
        }

        vector<vector<pair<int,pair<int,int>>>> dist = calc_dist_rail(cur_grid);
        // 最寄りの駅が存在する
        if(dist[x][y].second.first == -1){
            cout << "# Error don't exsist nearly station" << '\n';
            continue;
        }

        // 線路を引く
        if(construct_yorimichi(dist[x][y].second.first,dist[x][y].second.second,x,y,cur_grid,ans,cur_money,cur_income,station_pos).first == -1) continue;

        // 駅を作る
        make_station_rail(x,y,cur_income,cur_money,ans,vis_office,vis_house,cur_grid);

        // 途中でやめる
        if(ans.size() > t) break;
        int cur_score = cur_money + cur_income*(t - ans.size());
        if(cur_score > mx_score){
            mx_score = cur_score;
            mx_score_ans = ans;
        }
    }
    ans = mx_score_ans;
    assert(ans.size() <= t);
    while(ans.size() != t) ans.push_back({-1,-1,-1});
    return ans;
}

// 最初の1手
pair<beam_search::State,pair<pair<int,int>,pair<int,int>> > first_step(){
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

    // 各スコアを計算する (含まれるpairの数)/(距離)
    // めんどい後回し

    //収入が一番高くなるやつ
    int mx_score = 0, mx_score_pos = -1;
    vector<bool> vis_house(m,false); //めんどくさいから作ったけどいらない
    vector<bool> vis_office(m,false);
    rep(i,cand.size()){
        auto [sx,sy] = cand[i].first;
        auto [tx,ty] = cand[i].second;
        int dist = abs(tx - sx) + abs(ty - sy);
        if((dist - 1)*rail_cost + 2*station_cost > init_money) continue;
        int cur_score = calc_score(vis_house,vis_office,sx,sy,tx,ty);
        if(cur_score > mx_score){
            mx_score = cur_score;
            mx_score_pos = i;
        }
    }
    auto [sx,sy] = cand[mx_score_pos].first;
    auto [tx,ty] = cand[mx_score_pos].second;

    beam_search::State ret;
    ret.cur_turn = abs(sx - tx) + abs(sy - ty);
    ret.cur_money = init_money - (abs(sx - tx) + abs(sy - ty) - 1)*rail_cost - station_cost*2;
    assert(ret.cur_money >= 0);
    ret.station_pos.push_back({sx,sy});
    ret.station_pos.push_back({tx,ty});

    ret.vis_house.assign(m,0);
    ret.vis_office.assign(m,0);
    
    for(int idx : house_on_grid13[sx][sy]) ret.vis_house[idx]++;
    for(int idx : office_on_grid13[sx][sy]) ret.vis_office[idx]++;

    for(int idx : house_on_grid13[tx][ty]) ret.vis_house[idx]++;
    for(int idx : office_on_grid13[tx][ty]) ret.vis_office[idx]++;

    ret.cur_income = 0;
    // 収入を計算
    rep(i,m) if(ret.vis_house[i] && ret.vis_office[i]) ret.cur_income += commute_dist[i];

    // 接続されていない家やオフィス
    ret.no_connect_cnt = 0;
    rep(i,m) if(ret.vis_house[i] != ret.vis_office[i]) ret.no_connect_cnt++;

    return {ret,{{sx,sy},{tx,ty}}};
}

int main(){
    input();
    // vector<pair<int,int>> station_pos= greedy();

    // ビームサーチの設定
    beam_search::Config config;
    config.beam_width = 50;
    config.max_turn = min(m,100);
    config.hash_map_capacity = 50*50*config.beam_width*config.max_turn;
    config.nodes_capacity = 50*50*config.beam_width;

    auto [sta,init_pos] = first_step();
    beam_search::Node node(beam_search::Action(), beam_search::Evaluator(),0);

    vector<beam_search::Action> act = beam_search::beam_search(config,sta,node);

    vector<pair<int,int>> station_pos = {init_pos.first,init_pos.second};
    rep(i,act.size()){
        if(act[i].fin) break;
        station_pos.push_back({act[i].station_x,act[i].station_y});
    }
    
    vector<tuple<int,int,int>> ans = greedy_rail(station_pos);
    for(auto [out1, out2, out3] : ans){
        if(out1 != -1) cout << out1 << ' ' << out2 << ' ' << out3 << '\n';
        else cout << -1 << '\n';
    }
}