#include<iostream>
#include<vector>
#include <cstdio>
using std::vector;
struct trie_node{
vector<trie_node*> arr;
bool is_end;
bool visited;
trie_node(){
    for(int i=0;i<26;i++){
        arr.push_back(NULL);
    }
    is_end=false;
    visited=false;
}
};
void dfs(trie_node* node,vector<char>&ans,vector<vector<char> >&auto_complete_answer){
if(node->is_end){
    auto_complete_answer.push_back(ans);
}
for(int i=0;i<26;i++){
    char next_char=(char)('a'+i);
    if(node->arr[i]==NULL){
        continue;
    }
    ans.push_back(next_char);
    dfs(node->arr[i],ans,auto_complete_answer);
    ans.pop_back();
}
return;
}
bool comp(vector<char>&a,vector<char>&b){
    if(a.size()!=b.size())
    return false;
    for(int i=0;i<a.size();i++){
        if(a[i]!=b[i]){
            return false;
        }
    }
    return true;
}
int min(int a,int b){
    return a>b?b:a;
}
// void dfs_one(trie_node* node,vector<char>&ans,vector<char>&s,int j,vector<vector<char> >&auto_correct_answer,int rem_val){
// if(rem_val==-1){
//     return;
// }
// if((node->is_end)&&(j==s.size())){
//      bool is_present=false;
// for(int i=0;i<auto_correct_answer.size();i++){
// if(comp(ans,auto_correct_answer[i])==true){
//     is_present=true;
// }
// }
//     if(!is_present){
//          auto_correct_answer.push_back(ans);
//     }
// }

// for(int i=0;i<26;i++){
//      char next_char=(char)('a'+i);
//     if(node->arr[i]!=NULL){
//     ans.push_back(next_char);
//     if((j<s.size())&&(next_char==s[j])){
//     dfs_one(node->arr[i],ans,s,j+1,auto_correct_answer,rem_val);
//     }
//     if(j<s.size()){
//         dfs_one(node->arr[i],ans,s,j+1,auto_correct_answer,rem_val-1);//replacement
//     }
//     dfs_one(node->arr[i],ans,s,j,auto_correct_answer,rem_val-1);//insert
//     ans.pop_back();
//     }
// }
//         if(j<s.size())
//     dfs_one(node,ans,s,j+1,auto_correct_answer,rem_val-1);//delete
// return;
// }
void search_dp(trie_node* node, char ch, const vector<int>& prev_row,  const vector<char>& s, vector<char>& current_word,    vector<vector<char>>& auto_correct_answer,   vector<trie_node*>& visited_nodes) {
    int sz = s.size();
    vector<int> current_row(sz + 1);
    current_row[0] = prev_row[0] + 1;
    int min_dist = current_row[0];
    for (int j = 1; j <= sz; j++) {
        if (ch == s[j - 1]) {
            current_row[j] = prev_row[j - 1]; // Match
        } else {
            current_row[j] = 1 + min(
                prev_row[j],       
                min(current_row[j - 1], 
                prev_row[j - 1] )    
            );
        }
        min_dist = min(min_dist, current_row[j]);
    }

    if (min_dist > 3) return;
    current_word.push_back(ch);
    if (node->is_end && current_row[sz] <= 3 && !node->visited) {
        node->visited = true;
        auto_correct_answer.push_back(current_word);
        visited_nodes.push_back(node); 
    }
    for (int i = 0; i < 26; i++) {
        if (node->arr[i] != NULL) {
            search_dp(node->arr[i], (char)('a' + i), current_row, s, 
                      current_word, auto_correct_answer, visited_nodes);
        }
    }
    current_word.pop_back();
}
class trie{
public:
trie_node* root;
trie(){
    root=new trie_node();
}
void insert_string(vector<char> &s){
    trie_node* temp=root;
    for(int i=0;i<s.size();i++){
        if(temp->arr[s[i]-'a']==NULL){
            temp->arr[s[i]-'a']=new trie_node();
        }
        temp=temp->arr[s[i]-'a'];
    }
    temp->is_end=true;
}
bool spell_check(vector<char>&s){
    trie_node*temp=root;
    for(int i=0;i<s.size();i++){
        if(temp->arr[s[i]-'a']==NULL)
        return false;
        temp=temp->arr[s[i]-'a'];
    }
    return temp->is_end;
}
vector<vector<char> > auto_complete(vector<char>&s){
vector<vector<char> > auto_complete_answer;
trie_node* temp=root;
vector<char> ans;
for(int i=0;i<s.size();i++){
if(temp->arr[s[i]-'a']==NULL){
    return auto_complete_answer;
}
ans.push_back(s[i]);
temp=temp->arr[s[i]-'a'];
}
dfs(temp,ans,auto_complete_answer);
return auto_complete_answer;
}

vector<vector<char>> auto_correct(vector<char>& s) {
    vector<vector<char>> auto_correct_answer;
    vector<trie_node*> visited_nodes;
    vector<char> current_word;
    int m = s.size();

    // Initial root row: [0, 1, 2, ..., m]
    vector<int> root_row(m + 1);
    for (int j = 0; j <= m; j++) {
        root_row[j] = j;
    }

    // Launch DFS from root children
    for (int i = 0; i < 26; i++) {
        if (root->arr[i] != NULL) {
            search_dp(root->arr[i], (char)('a' + i), root_row, s, 
                      current_word, auto_correct_answer, visited_nodes);
        }
    }

    // Reset visited flags for subsequent queries
    for (trie_node* node : visited_nodes) {
        node->visited = false;
    }

    return auto_correct_answer;
}
};
int main(){
int n,q;
scanf("%d%d",&n,&q);
vector<vector<char> > dictionary(n);
trie* t=new trie();

for(int i=0;i<n;i++){
    char buff[101];
    scanf("%s",buff);
    for(int j=0;buff[j]!='\0';j++){
        dictionary[i].push_back(buff[j]);
    }
t->insert_string(dictionary[i]);
}
vector<vector<char> > querry(q);
vector<int> mode(q,0);
for(int i=0;i<q;i++){
scanf("%d",&mode[i]);
char buff[111];
    scanf("%s",buff);
    for(int j=0;buff[j]!='\0';j++){
        querry[i].push_back(buff[j]);
    }
}
int i=0;
while(q--){
if(mode[i]==1){
if(t->spell_check(querry[i])){
printf("1\n");
}else{
    printf("0\n");
}
}else if(mode[i]==2){
vector<vector<char> > temp=t->auto_complete(querry[i]);
int k=temp.size();
printf("%d\n",k);
for(int a=0;a<k;a++){
    for(int j=0;j<temp[a].size();j++){
        printf("%c",temp[a][j]);
    }
        printf("\n");
}
}else if(mode[i]==3){
vector<vector<char> > temp=t->auto_correct(querry[i]);
int k=temp.size();
printf("%d\n",k);
for(int a=0;a<k;a++){
    for(int j=0;j<temp[a].size();j++){
        printf("%c",temp[a][j]);
    }
        printf("\n");
}
}else{
    printf("Invalid mode");
}
i++;
}
}