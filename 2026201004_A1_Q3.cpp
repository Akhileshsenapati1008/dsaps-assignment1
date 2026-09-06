#include <stdio.h>
struct Query {
    int type;
    long long l, r, h;
};
struct Node {
    long long max_h;
    long long lazy;
};
const int MAX_Q = 100005;
const int MAX_COORDS = 200005;
Query queries[MAX_Q];
long long raw_coords[MAX_COORDS];
long long leaf_heights[MAX_COORDS];
Node seg_tree[MAX_COORDS * 4];
inline long long max(long long a, long long b) {
    return (a > b) ? a : b;
}
inline void swap(long long& a, long long& b) {
    long long temp = a;
    a = b;
    b = temp;
}
void sift_down(long long* arr, int n, int i) {
    while (true) {
        int max_one = i;
        int le = 2 * i + 1;
        int ri = 2 * i + 2;

        if (le < n && arr[le] > arr[max_one]) max_one = le;
        if (ri < n && arr[ri] > arr[max_one]) max_one = ri;

        if (max_one == i) break;

        swap(arr[i], arr[max_one]);
        i = max_one;
    }
}

void heap_sort(long long* arr, int n) {
    for (int i = n / 2 - 1; i >= 0; i--) {
        sift_down(arr, n, i);
    }
    for (int i = n - 1; i > 0; i--) {
        swap(arr[0], arr[i]);
        sift_down(arr, i, 0);
    }
}
int remove_duplicates(long long* arr, int n) {
    if (n == 0) return 0;
    int j = 0;
    for (int i = 1; i < n; i++) {
        if (arr[i] != arr[j]) {
            j++;
            arr[j] = arr[i];
        }
    }
    return j + 1;
}
int lower_bound(long long* arr, int n, long long val) {
    int lo = 0, hi = n - 1;
    int ans = n;
    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;
        if (arr[mid] >= val) {
            ans = mid;
            hi = mid - 1;
        } else {
            lo = mid + 1;
        }
    }
    return ans;
}
void push(int node) {
    if (seg_tree[node].lazy != 0) {
        long long val = seg_tree[node].lazy;
        seg_tree[2 * node].max_h = max(seg_tree[2 * node].max_h, val);
        seg_tree[2 * node].lazy = max(seg_tree[2 * node].lazy, val);
        seg_tree[2 * node + 1].max_h = max(seg_tree[2 * node + 1].max_h, val);
        seg_tree[2 * node + 1].lazy = max(seg_tree[2 * node + 1].lazy, val);
        seg_tree[node].lazy = 0;
    }
}
void update_range(int node, int start, int end, int ql, int qr, long long val) {
    if (ql > end || qr < start) return;

    if (ql <= start && end <= qr) {
        seg_tree[node].max_h = max(seg_tree[node].max_h, val);
        seg_tree[node].lazy = max(seg_tree[node].lazy, val);
        return;
    }

    push(node);
    int mid = start + (end - start) / 2;
    update_range(2 * node, start, mid, ql, qr, val);
    update_range(2 * node + 1, mid + 1, end, ql, qr, val);

    seg_tree[node].max_h = max(seg_tree[2 * node].max_h, seg_tree[2 * node + 1].max_h);
}
void get_leaves(int node, int start, int end) {
    if (start == end) {
        leaf_heights[start] = seg_tree[node].max_h;
        return;
    }

    push(node);
    int mid = start + (end - start) / 2;
    get_leaves(2 * node, start, mid);
    get_leaves(2 * node + 1, mid + 1, end);
}
int main() {
    int q;
    if (scanf("%d", &q) != 1) return 0;
    int coord_cnt = 0;
    for (int i = 0; i < q; i++) {
        scanf("%d", &queries[i].type);
        if (queries[i].type == 0) {
            scanf("%lld %lld %lld", &queries[i].l, &queries[i].r, &queries[i].h);
            raw_coords[coord_cnt++] = queries[i].l;
            raw_coords[coord_cnt++] = queries[i].r;
        }
    }
    if (coord_cnt == 0) return 0;
    heap_sort(raw_coords,coord_cnt);
    int num_coords = remove_duplicates(raw_coords, coord_cnt);
    int num_intervals = num_coords - 1;
    for (int i = 0; i < q; i++) {
        if (queries[i].type == 0) {
            int l_idx = lower_bound(raw_coords, num_coords, queries[i].l);
            int r_idx = lower_bound(raw_coords, num_coords, queries[i].r);
            if (l_idx <= r_idx - 1) {
                update_range(1, 0, num_intervals - 1, l_idx, r_idx - 1, queries[i].h);
            }
        } 
        else if (queries[i].type == 1) {
            get_leaves(1, 0, num_intervals - 1);
            long long prev_h = 0;
            for (int k = 0; k < num_intervals; k++) {
                long long curr_h = leaf_heights[k];
                if (curr_h != prev_h) {
                    printf("%lld %lld\n", raw_coords[k], curr_h);
                    prev_h = curr_h;
                }
            }
            if (prev_h != 0) {
                printf("%lld 0\n", raw_coords[num_intervals]);
            }
        }
    }
    return 0;
}