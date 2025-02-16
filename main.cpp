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

// 13近傍
vector<int> dx13 = {0,1,0,-1,0,1,-1,-1,1,2,0,-2,0};
vector<int> dy13 = {0,0,1,0,-1,1,1,-1,-1,0,2,0,-2};

// 4近傍
vector<int> dx4 = {1,0,-1,0};
vector<int> dy4 = {0,1,0,-1};

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
    ret = mx_ans_score_ret;
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

int main(){
    input();
    vector<pair<int,int>> station_pos= greedy();
    
    vector<tuple<int,int,int>> ans = greedy_rail(station_pos);
    for(auto [out1, out2, out3] : ans){
        if(out1 != -1) cout << out1 << ' ' << out2 << ' ' << out3 << '\n';
        else cout << -1 << '\n';
    }
}