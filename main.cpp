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

// その人の通勤距離
vector<int> commute_dist;

void input(){
    cin >> n >> m >> init_money >> t;
    house.assign(m,pair<int,int>());
    office.assign(m,pair<int,int>());
    house_on_grid.assign(n,vector<vector<int>>(n));
    office_on_grid.assign(n,vector<vector<int>>(n));
    commute_dist.assign(m,0);
    rep(i,m){
        cin >> house[i].first >> house[i].second;
        cin >> office[i].first >> office[i].second;
        house_on_grid[house[i].first][house[i].second].push_back(i);
        office_on_grid[office[i].first][office[i].second].push_back(i);
        commute_dist[i] = abs(office[i].first - house[i].first) + abs(office[i].second - house[i].second);
    }
}

// {sx, sy} -> {tx, ty} にレールを引く　スタートとゴールには触らない 所持金の更新もする レールを設置して収入が増えることがないことを想定
void construct(int sx,int sy,int tx,int ty, vector<vector<int>>& cur_grid, vector<tuple<int,int,int>>& ans, int& cur_money, int cur_income){
    assert(!(sx == tx && sy == ty));
    vector<vector<int>> dist(n,vector<int>(n,1e9));
    dist[sx][sy] = 0;
    queue<pair<int,int>> que;
    que.push({sx,sy});
    vector<int> dx = {1,0,-1,0};
    vector<int> dy = {0,1,0,-1};
    while(!que.empty()){
        auto [x,y] = que.front();
        que.pop();
        int fn = false;
        rep(i,4){
            int nx = x + dx[i];
            int ny = y + dy[i];
            if(0 <= nx && nx < n && 0 <= ny && ny < n && dist[nx][ny] == 1e9 && cur_grid[nx][ny] <= 0){
                dist[nx][ny] = dist[x][y] + 1;
                if(nx == tx && ny == ty) fn = true;
                que.push({nx,ny});
            }
        }
        if(fn) break;
    }
    assert(dist[tx][ty] != 1e9);

    // 矢印の列挙
    vector<int> dir;
    vector<pair<int,int>> pos;
    int x = tx,y = ty;
    while(!(x == sx && y == sy)){
        rep(i,4){
            int nx = x + dx[i];
            int ny = y + dy[i];
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
    // assert((int)ans.size() - pre_ans_sz == (int)dir.size() - 1);
}

//駅からの距離 {距離、最寄り駅の座標}
vector<vector<pair<int,pair<int,int>>>> calc_dist(vector<vector<int>>& cur_grid){
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
void make_station(int x,int y,int& cur_income,int& cur_money,vector<tuple<int,int,int>>& ans,vector<bool>& vis_office, vector<bool>& vis_house, vector<vector<int>>& cur_grid){
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

// 操作をパスする
void pass(int& cur_money, int& cur_income, vector<tuple<int,int,int>>& ans){
    ans.push_back({-1,-1,-1});
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
    int not_connect_w = 0;
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
                }else if(!vis_house[j]){
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
                }else if(!vis_office[j]){
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
                }else if(!vis_house[j]){
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
                }else if(!vis_office[j]){
                    add_office_idx.push_back(j);
                    not_connect_cnt++;
                }
            }
        }
    }

    return ret + not_connect_cnt*not_connect_w;
}

// 答えを返す 最初の要素が-1なら-1
vector<tuple<int,int,int>> greedy(){
    int cur_money = init_money;
    int cur_income = 0;
    vector<tuple<int,int,int>> ret;
    // 操作を終えるターン
    int finish_turn = 700;
    //現在の盤面
    vector<vector<int>> cur_grid(n,vector<int>(n,-1));
    // すでに家が線路に接続している
    vector<bool> vis_house(m,false);
    // すでにオフィスが線路に接続している
    vector<bool> vis_office(m,false);

    // 駅と接続できる範囲 13近傍
    vector<int> dx = {0,1,0,-1,0,1,-1,-1,1,2,0,-2,0};
    vector<int> dy = {0,0,1,0,-1,1,1,-1,-1,0,2,0,-2};
    {// 最初の一手
        // すべての人の家とオフィスをつなぐ候補を列挙する
        vector<pair<pair<int,int>,pair<int,int>> > cand;
        rep(i,m){
            if(vis_house[i] && vis_office[i]) continue;
            int sx = house[i].first,sy = house[i].second;
            int tx = office[i].first,ty = office[i].second;
            rep(j,13){
                int nsx = sx + dx[j], nsy = sy + dy[j];
                if(0 <= nsx && nsx < n && 0 <= nsy && nsy < n){
                    rep(k,13){
                        int ntx = tx + dx[k], nty = ty + dy[k];
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

        construct(sx,sy,tx,ty,cur_grid,ret,cur_money,cur_income);

        make_station(sx,sy,cur_income,cur_money,ret,vis_office,vis_house,cur_grid);
        
        make_station(tx,ty,cur_income,cur_money,ret,vis_office,vis_house,cur_grid);
    }

    // 2手目以降は貪欲に領域を伸ばす // TODO 13近傍でやる
    while(ret.size() < finish_turn){
        vector<vector<pair<int,pair<int,int>>>> dist = calc_dist(cur_grid);
        int mx_score_op_cnt = 1e9;
        int mx_score_cost = 1e9;
        int mx_score_pos = -1;
        double mx_score = -1e9;
        rep(i,m){
            if(vis_house[i] && vis_office[i]) continue;
            int cur_cost = 0;
            int cur_op_cnt = 0;
            int cur_score = 0;
            if(vis_house[i]){ // 家は接続されている
                if(dist[office[i].first][office[i].second].first == 1e9) continue;
                cur_cost = (dist[office[i].first][office[i].second].first - 1)*rail_cost + station_cost;
                cur_op_cnt = dist[office[i].first][office[i].second].first;
                cur_score = calc_score(vis_house,vis_office,office[i].first,office[i].second);
            }else if(vis_office[i]){ //　職場は接続されている
                if(dist[house[i].first][house[i].second].first == 1e9) continue;
                cur_cost = (dist[house[i].first][house[i].second].first - 1)*rail_cost + station_cost;
                cur_op_cnt = dist[house[i].first][house[i].second].first;
                cur_score = calc_score(vis_house,vis_office,house[i].first,house[i].second);
            }else{
                if(dist[house[i].first][house[i].second].first == 1e9 || dist[office[i].first][office[i].second].first == 1e9) continue;
                cur_cost = (dist[house[i].first][house[i].second].first - 1 + dist[office[i].first][office[i].second].first - 1)*rail_cost + station_cost*2;
                cur_op_cnt = dist[house[i].first][house[i].second].first + dist[office[i].first][office[i].second].first;
                cur_score = calc_score(vis_house,vis_office,house[i].first,house[i].second,office[i].first,office[i].second);
            }
            // TODO 操作回数を評価に入れる
            if(mx_score < (double)cur_score/cur_cost){
                mx_score = (double)cur_score/cur_cost;
                mx_score_op_cnt = cur_op_cnt;
                mx_score_cost = cur_cost;
                mx_score_pos = i;
            }
        }
        if(mx_score_pos == -1) break; // 接続できる所がない

        // お金がたまるまで我慢 めんどくさいので貯金で無理だったら無理なことにする TODO なおす
        while(cur_money < mx_score_cost){
            pass(cur_money, cur_income, ret);
            if(ret.size() == finish_turn) break;
        }

        // 操作がtに収まらない
        if(ret.size() + mx_score_op_cnt > finish_turn) break;
        
        // TODO 両方接続してないときは線路を先につないだ方がお金が節約できる
        // TODO 先に接続したほうから伸ばした方がいい場合がある
        if(!vis_office[mx_score_pos]){ // オフィスは接続されていない
            auto [sx,sy] = office[mx_score_pos];
            auto [tx,ty] = dist[sx][sy].second;
            construct(sx,sy,tx,ty,cur_grid,ret,cur_money,cur_income);

            make_station(sx,sy,cur_income,cur_money,ret,vis_office,vis_house,cur_grid);
        }
        if(!vis_house[mx_score_pos]){ // 家は接続されていない
            auto [sx,sy] = house[mx_score_pos];
            auto [tx,ty] = dist[sx][sy].second;
            construct(sx,sy,tx,ty,cur_grid,ret,cur_money,cur_income);

            make_station(sx,sy,cur_income,cur_money,ret,vis_office,vis_house,cur_grid);
        }
        assert(cur_money >= 0);

    }

    while(ret.size() < t) pass(cur_money, cur_income, ret);
    cout << "# money = " << cur_money << endl;
    cout << "# income = " << cur_income << endl;
    return ret;
}

int main(){
    input();
    vector<tuple<int,int,int>> ans = greedy();
    for(auto [out1, out2, out3] : ans){
        if(out1 != -1) cout << out1 << ' ' << out2 << ' ' << out3 << '\n';
        else cout << -1 << '\n';
    }
}